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
    connect(m_tcpSocket, &TcpSocket::messageReceived, this, &MainWindow::onMessageReceived);
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

void MainWindow::onMessageReceived(const QString &message)
{
    if (message.startsWith("AUTH_SUCCESS:")) {
        m_isAuthenticated = true;
        m_displayName = message.mid(QString("AUTH_SUCCESS:").size());
        ui->lstChat->addItem("Authenticated as: " + m_displayName);
        return;
    }

    if (message.startsWith("AUTH_FAILED:")) {
        m_isAuthenticated = false;
        ui->lstChat->addItem(message);
        return;
    }

    if (message.startsWith("SEARCH_EMPTY")) {
        ui->lstUsers->clear();
        ui->lstChat->addItem("No authenticated users found.");
        return;
    }

    if (message.startsWith("SEARCH_RESULT:")) {
        const QString payload = message.mid(QString("SEARCH_RESULT:").size());
        const QStringList users = payload.split(',', Qt::SkipEmptyParts);

        ui->lstUsers->clear();
        for (const QString &user : users) {
            ui->lstUsers->addItem(user.trimmed());
        }

        if (users.isEmpty()) {
            ui->lstChat->addItem("No authenticated users found.");
        } else {
            ui->lstChat->addItem(QString("Found %1 user(s). Click a name to create a session.").arg(users.size()));
        }
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

    ui->lstChat->addItem(message);
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

    const QString query = ui->lnSearch->text().trimmed();
    const QString packet = QString("SEARCH:%1\n").arg(query);
    m_tcpSocket->sendMessage(packet);
}

void MainWindow::on_lstUsers_itemClicked(QListWidgetItem *item)
{
    if (!item || !m_isAuthenticated) {
        return;
    }

    const QString selectedUser = item->text().trimmed();
    if (selectedUser.isEmpty()) {
        return;
    }

    const QString req = QString("SESSION_CREATE:%1\n").arg(selectedUser);
    m_tcpSocket->sendMessage(req);
}
