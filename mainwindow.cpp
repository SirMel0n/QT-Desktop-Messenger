#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QShortcut>  // Add this include

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tcpSocket(new TcpSocket(this))
    , m_auth(new Authentication(this))
    , shortcut(new QShortcut(QKeySequence(Qt::Key_Return), this))  // Fixed: use 'this' instead of 'parent'
{
    ui->setupUi(this);

    // Wire up socket signals to MainWindow slots
    connect(m_tcpSocket, &TcpSocket::connected,        this, &MainWindow::onConnected);
    connect(m_tcpSocket, &TcpSocket::disconnected,     this, &MainWindow::onDisconnected);
    connect(m_tcpSocket, &TcpSocket::messageReceived,  this, &MainWindow::onMessageReceived);
    connect(m_tcpSocket, &TcpSocket::errorOccurred,    this, &MainWindow::onSocketError);

    // Connect Enter key shortcut to send button - Fixed: connect to MainWindow slot
    connect(shortcut, &QShortcut::activated, this, &MainWindow::on_btnSend_clicked);
    
    // Connect Authentication signal to MainWindow slot
    connect(m_auth, &Authentication::loginSuccessful, this, &MainWindow::onLoginSuccessful);

    // Connect to server after showing auth dialog
    m_tcpSocket->connectToServer("26.161.132.244", 12345);
}

MainWindow::~MainWindow()
{
    delete ui;
    // shortcut will be deleted automatically if parent is set to 'this'
}

void MainWindow::onConnected()
{
    ui->lstChat->addItem("You are connected to the server");
}

void MainWindow::onDisconnected()
{
    ui->lstChat->addItem("Disconnected from server");
}

void MainWindow::onMessageReceived(const QString &message)
{
    ui->lstChat->addItem(message);
}

void MainWindow::onSocketError(const QString &errorMsg)
{
    ui->lstChat->addItem("Error: " + errorMsg);
}

void MainWindow::on_btnSend_clicked()
{
    QString msg = ui->lnMessage->text().trimmed();
    if (!msg.isEmpty()) {
        m_tcpSocket->sendMessage(msg);
        ui->lnMessage->clear();
    }
}

void MainWindow::onLoginSuccessful(const QString &username)
{
    ui->lstChat->addItem("Welcome: " + username);
}

