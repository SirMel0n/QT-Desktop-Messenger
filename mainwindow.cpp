#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QShortcut>
#include <QMessageBox>
#include <QListWidgetItem>
#include "ConfigManager.h"

MainWindow::MainWindow(const QString &login, const QString &password, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tcpSocket(new TcpSocket(this))
    , shortcut(new QShortcut(QKeySequence(Qt::Key_Return), this))
    , m_login(login)
    , m_password(password)
    , m_isAuthenticated(false)
{
    ui->setupUi(this);

    connect(m_tcpSocket, &TcpSocket::connected, this, &MainWindow::onConnected);
    connect(m_tcpSocket, &TcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(m_tcpSocket, &TcpSocket::messageReceived, this, &MainWindow::onResponseReceived);
    connect(m_tcpSocket, &TcpSocket::errorOccurred, this, &MainWindow::onSocketError);

    connect(shortcut, &QShortcut::activated, this, &MainWindow::on_btnSend_clicked);

    m_tcpSocket->connectToServer(ConfigManager::instance().IpServer(), ConfigManager::instance().serverPort());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConnected()
{
    ui->lstChat->addItem("Connected to server");

    const QString authRequest = QString("AUTH:%1:%2\n").arg(m_login, m_password);
    m_tcpSocket->sendMessage(authRequest);
}

void MainWindow::onDisconnected()
{
    m_isAuthenticated = false;
    m_activePeer.clear();
    ui->lblActiveSession->setText("Active session: none");
    ui->lstUsers->clear();
    ui->lstChat->addItem("Disconnected from server");
}

void MainWindow::onResponseReceived(const QString &message)
{
    qDebug() << "MainWindow received:" << message;

    if (message.startsWith("AUTH_SUCCESS:")) {
        m_isAuthenticated = true;
        m_displayName = message.mid(QString("AUTH_SUCCESS:").size());
        ui->lstChat->addItem("Authenticated as: " + m_displayName);
        m_tcpSocket->sendMessage("PULL_PENDING\n");
        return;
    }

    if (message.startsWith("AUTH_FAILED:")) {
        m_isAuthenticated = false;
        ui->lstChat->addItem(message);
        return;
    }

    if (message.startsWith("ERROR:")) {
        ui->lstChat->addItem(message);
        qWarning() << "Server error response:" << message;
        return;
    }

    if (message.startsWith("SEARCH_EMPTY")) {
        ui->lstUsers->clear();
        QListWidgetItem *item = new QListWidgetItem("No matching users found");
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        ui->lstUsers->addItem(item);
        return;
    }

    if (message.startsWith("SEARCH_RESULT:")) {
        const QString payload = message.mid(QString("SEARCH_RESULT:").size());
        const QStringList entries = payload.split(';', Qt::SkipEmptyParts);

        ui->lstUsers->clear();

        for (const QString &entry : entries) {
            const QStringList parts = entry.split('|');
            const QString username = parts.value(0).trimmed();
            const QString status = parts.value(1).trimmed();

            if (username.isEmpty()) {
                continue;
            }

            QString label = username;
            if (status == "active_session") {
                label += " (online chat)";
            } else if (status == "active") {
                label += " (online)";
            } else if (status == "session") {
                label += " (chat)";
            }

            QListWidgetItem *item = new QListWidgetItem(label);
            item->setData(Qt::UserRole, username);
            ui->lstUsers->addItem(item);
        }

        ui->lstChat->addItem(QString("Found %1 result(s).").arg(ui->lstUsers->count()));
        return;
    }

    if (message.startsWith("SESSION_CREATED:")) {
        m_activePeer = message.mid(QString("SESSION_CREATED:").size()).trimmed();
        ui->lblActiveSession->setText("Active session: " + m_activePeer);
        ui->lstChat->addItem("Session active with: " + m_activePeer);
        return;
    }

    if (message.startsWith("SESSION_FAILED:")) {
        ui->lstChat->addItem(message);
        return;
    }

    if (message.startsWith("MSG:")) {
        const int firstColon = message.indexOf(':');
        const int secondColon = message.indexOf(':', firstColon + 1);

        if (firstColon == -1 || secondColon == -1) {
            qDebug() << "Invalid MSG format:" << message;
            return;
        }

        const QString user = message.mid(firstColon + 1, secondColon - firstColon - 1).trimmed();
        const QString body = message.mid(secondColon + 1).trimmed();

        if (user.isEmpty() || body.isEmpty()) {
            qDebug() << "Invalid MSG content";
            return;
        }

        QListWidgetItem* item = new QListWidgetItem(ui->lstChat);
        QWidget* rowWidget = new QWidget;
        QVBoxLayout* layout = new QVBoxLayout(rowWidget);
        layout->setContentsMargins(8, 4, 8, 4);
        layout->setSpacing(2);

        QLabel* nameLabel = new QLabel(QString("(%1)").arg(user));
        nameLabel->setStyleSheet("color: #FF0000; font-weight: 600;");

        QLabel* msgLabel = new QLabel(body);
        msgLabel->setWordWrap(true);
        msgLabel->setStyleSheet("color: #EAEAEA;");

        layout->addWidget(nameLabel);
        layout->addWidget(msgLabel);

        item->setSizeHint(rowWidget->sizeHint());
        ui->lstChat->setItemWidget(item, rowWidget);
        qDebug() << "Message received from" << user;
    }

    if (message.startsWith("PRESENCE:")) {
        // PRESENCE:<username>:online|offline
        if (m_isAuthenticated) {
            const QString query = ui->lnSearch->text().trimmed();
            const QString packet = QString("SEARCH:%1\n").arg(query);
            m_tcpSocket->sendMessage(packet);
        }
        return;
    }
}

void MainWindow::onSocketError(const QString &errorMsg)
{
    ui->lstChat->addItem("Error: " + errorMsg);
}

void MainWindow::on_btnSend_clicked()
{
    const QString msg = ui->lnMessage->text().trimmed();
    if (msg.isEmpty()) {
        return;
    }

    if (!m_isAuthenticated) {
        QMessageBox::warning(this, "Not authenticated", "Please authenticate first.");
        return;
    }

    if (m_activePeer.isEmpty()) {
        QMessageBox::information(this, "No session", "Search and click a user first.");
        return;
    }


    // display message in the chat window
    QString safeName = m_displayName;
    QString safeMsg  = msg;

    // Create list item
    QListWidgetItem* item = new QListWidgetItem(ui->lstChat);

    //  Create row widget
    QWidget* rowWidget = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(rowWidget);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(2);

    // Name label (top, colored)
    QLabel* nameLabel = new QLabel(QString("(%1)").arg(safeName));
    nameLabel->setStyleSheet("color: #4A90E2; font-weight: 600;");

    // Message label (bottom)
    QLabel* msgLabel = new QLabel(safeMsg);
    msgLabel->setWordWrap(true);
    msgLabel->setStyleSheet("color: #EAEAEA;"); // optional

    layout->addWidget(nameLabel);
    layout->addWidget(msgLabel);

    // Let QListWidget size row correctly
    item->setSizeHint(rowWidget->sizeHint());
    ui->lstChat->setItemWidget(item, rowWidget);

    const QString packet = QString("MSG:%1:%2\n").arg(m_activePeer, msg);
    m_tcpSocket->sendMessage(packet);
    ui->lnMessage->clear();
}

void MainWindow::on_btnSearch_clicked()
{
    if (!m_isAuthenticated) {
        QMessageBox::warning(this, "Not authenticated", "Please authenticate first.");
        return;
    }

    ui->lstUsers->clear(); // clear stale results before new search

    const QString query = ui->lnSearch->text().trimmed();
    const QString packet = QString("SEARCH:%1\n").arg(query);
    m_tcpSocket->sendMessage(packet);
}

void MainWindow::on_lstUsers_itemClicked(QListWidgetItem *item)
{
    if (!item || !m_isAuthenticated) {
        return;
    }

    // only allow real user rows
    const QString selectedUser = item->data(Qt::UserRole).toString().trimmed();
    if (selectedUser.isEmpty()) {
        return;
    }

    const QString req = QString("SESSION_CREATE:%1\n").arg(selectedUser);
    m_tcpSocket->sendMessage(req);
}
