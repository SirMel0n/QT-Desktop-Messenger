#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QShortcut>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include "ConfigManager.h"

MainWindow::MainWindow(const QString &login, const QString &password, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tcpSocket(new TcpSocket(this))
    , shortcut(new QShortcut(QKeySequence(Qt::Key_Return), this))
    , m_login(login)
    , m_password(password)
    , m_isAuthenticated(false)
    , m_searchOpen(false)
{
    ui->setupUi(this);
    setupSplitLayout();

    connect(m_tcpSocket, &TcpSocket::connected, this, &MainWindow::onConnected);
    connect(m_tcpSocket, &TcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(m_tcpSocket, &TcpSocket::messageReceived, this, &MainWindow::onResponseReceived);
    connect(m_tcpSocket, &TcpSocket::errorOccurred, this, &MainWindow::onSocketError);

    connect(shortcut, &QShortcut::activated, this, &MainWindow::on_btnSend_clicked);

    ui->lblActiveSession->setText("Active session: none");
    showStatus("Connecting...");

    m_tcpSocket->connectToServer(ConfigManager::instance().IpServer(), ConfigManager::instance().serverPort());
}

void MainWindow::setupSplitLayout()
{
    QWidget *central = ui->centralwidget;
    if (!central) {
        return;
    }

    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    auto *splitter = new QSplitter(Qt::Horizontal, central);

    auto *leftPane = new QWidget(splitter);
    auto *leftLayout = new QVBoxLayout(leftPane);
    leftLayout->setContentsMargins(8, 8, 8, 8);
    leftLayout->setSpacing(8);
    leftLayout->addWidget(ui->layoutWidget); // search row
    leftLayout->addWidget(ui->lstUsers);     // search/chat list

    auto *rightPane = new QWidget(splitter);
    auto *rightLayout = new QVBoxLayout(rightPane);
    rightLayout->setContentsMargins(8, 8, 8, 8);
    rightLayout->setSpacing(8);
    rightLayout->addWidget(ui->groupBox);    // chat window area

    splitter->setChildrenCollapsible(false);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    splitter->setSizes({280, 520});

    rootLayout->addWidget(splitter);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showStatus(const QString &text)
{
    statusBar()->showMessage(text, 4000);
}

void MainWindow::closeSearchPanel()
{
    m_searchOpen = false;

    ui->label_2->show();
    ui->lnSearch->show();
    ui->btnSearch->setText("Search");

    // keep list visible to preserve splitter layout
    ui->lstUsers->clear();

    // reload existing chats list
    m_tcpSocket->sendMessage("SEARCH:\n");
}

void MainWindow::appendChatBubble(const QString &user, const QString &body, bool outgoing)
{
    if (user.isEmpty() || body.isEmpty()) {
        return;
    }

    QListWidgetItem *item = new QListWidgetItem(ui->lstChat);
    QWidget *rowWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(rowWidget);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(2);

    QLabel *nameLabel = new QLabel(QString("(%1)").arg(user));
    nameLabel->setStyleSheet(outgoing
                                 ? "color: #4A90E2; font-weight: 600;"
                                 : "color: #FF0000; font-weight: 600;");

    QLabel *msgLabel = new QLabel(body);
    msgLabel->setWordWrap(true);
    msgLabel->setStyleSheet("color: #EAEAEA;");

    layout->addWidget(nameLabel);
    layout->addWidget(msgLabel);

    item->setSizeHint(rowWidget->sizeHint());
    ui->lstChat->setItemWidget(item, rowWidget);
    ui->lstChat->scrollToBottom();
}

void MainWindow::onConnected()
{
    showStatus("Connected to server");
    const QString authRequest = QString("AUTH:%1:%2\n").arg(m_login, m_password);
    m_tcpSocket->sendMessage(authRequest);
}

void MainWindow::onDisconnected()
{
    m_isAuthenticated = false;
    m_activePeer.clear();
    ui->lblActiveSession->setText("Active session: none");
    ui->lstChat->clear();
    closeSearchPanel();
    showStatus("Disconnected from server");
}

void MainWindow::onResponseReceived(const QString &message)
{
    qDebug() << "MainWindow received:" << message;

    if (message.startsWith("AUTH_SUCCESS:")) {
        m_isAuthenticated = true;
        m_displayName = message.mid(QString("AUTH_SUCCESS:").size()).trimmed();
        showStatus("Authenticated as " + m_displayName);
        m_tcpSocket->sendMessage("PULL_PENDING\n");
        m_tcpSocket->sendMessage("SEARCH:\n"); // load existing chats immediately
        return;
    }

    if (message.startsWith("AUTH_FAILED:")) {
        m_isAuthenticated = false;
        showStatus(message);
        return;
    }

    if (message.startsWith("ERROR:")) {
        qWarning() << "Server error response:" << message;
        showStatus(message);
        return;
    }

    if (message.startsWith("SEARCH_EMPTY")) {
        ui->lstUsers->clear();

        QListWidgetItem *item = nullptr;
        if (m_searchOpen) {
            item = new QListWidgetItem("No online users found");
        } else {
            item = new QListWidgetItem("No chats yet");
        }

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

            const bool isOnline = (status == "active" || status == "active_session");
            const bool hasSession = (status == "session" || status == "active_session");

            // Search mode: show both online users and existing chats
            if (m_searchOpen && !(isOnline || hasSession)) {
                continue;
            }

            // Normal mode: show existing chats only
            if (!m_searchOpen && !hasSession) {
                continue;
            }

            QString label = username;
            if (status == "active_session") {
                label += " (online chat)";
            } else if (status == "active") {
                label += " (online)";
            } else {
                label += " (chat)";
            }

            QListWidgetItem *item = new QListWidgetItem(label);
            item->setData(Qt::UserRole, username);
            ui->lstUsers->addItem(item);
        }

        if (ui->lstUsers->count() == 0) {
            QListWidgetItem *item = nullptr;
            if (m_searchOpen) {
                item = new QListWidgetItem("No online users found");
            } else {
                item = new QListWidgetItem("No chats yet");
            }
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
            ui->lstUsers->addItem(item);
        }

        return;
    }

    if (message.startsWith("SESSION_CREATED:")) {
        m_activePeer = message.mid(QString("SESSION_CREATED:").size()).trimmed();
        ui->lblActiveSession->setText("Active session: " + m_activePeer);
        ui->lstChat->clear();
        showStatus("Session active with " + m_activePeer);
        m_tcpSocket->sendMessage("PULL_PENDING\n");
        closeSearchPanel();
        return;
    }

    if (message.startsWith("SESSION_FAILED:")) {
        showStatus(message);
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
        appendChatBubble(user, body, false);
        return;
    }

    if (message.startsWith("PRESENCE:")) {
        if (m_isAuthenticated) {
            const QString packet = QString("SEARCH:%1\n").arg(m_searchOpen ? m_lastSearchQuery : QString());
            m_tcpSocket->sendMessage(packet);
        }
        return;
    }
}

void MainWindow::onSocketError(const QString &errorMsg)
{
    showStatus("Error: " + errorMsg);
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
        QMessageBox::information(this, "No session", "Press Search and pick an online user first.");
        return;
    }

    appendChatBubble(m_displayName, msg, true);

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

    if (!m_searchOpen) {
        m_searchOpen = true;
        m_lastSearchQuery = ui->lnSearch->text().trimmed();
        ui->label_2->hide();
        ui->lnSearch->hide();
        ui->btnSearch->setText("Close");

        ui->lstUsers->clear();
        const QString packet = QString("SEARCH:%1\n").arg(m_lastSearchQuery); // online list mode
        m_tcpSocket->sendMessage(packet);
        return;

    }

    closeSearchPanel(); // restore chat list mode
}

void MainWindow::on_lstUsers_itemClicked(QListWidgetItem *item)
{
    if (!item || !m_isAuthenticated) {
        return;
    }   

    const QString selectedUser = item->data(Qt::UserRole).toString().trimmed();
    if (selectedUser.isEmpty()) {
        return;
    }

    const QString req = QString("SESSION_CREATE:%1\n").arg(selectedUser);
    m_tcpSocket->sendMessage(req);
    closeSearchPanel();
}
