#include "registration.h"
#include "./Registration/ui_registration.h"
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSqlDatabase>
#include <QSqlError>

Registration::Registration(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Registration)
    , m_socket(new QTcpSocket(this))
{
    // Remove initialization of inherited members:
    // passVal, nickVal, loginVal are initialized in Database constructor
    
    ui->setupUi(this);

    // set the title
    setWindowTitle("Sign Up");

    // Telegram Dark Theme Palette
    this->setStyleSheet(
        "Registration { "
        "    background-color: #212121; "
        "}"
        "QLabel { "
        "    background-color: transparent; "
        "    color: #FFFFFF; "
        "    font-size: 14px; "
        "}"
        "QPushButton { "
        "    background-color: #5288C1; "
        "    color: #FFFFFF; "
        "    border: none; "
        "    border-radius: 8px; "
        "    padding: 10px 20px; "
        "    font-size: 14px; "
        "    font-weight: bold; "
        "}"
        "QPushButton:hover { "
        "    background-color: #6BA6DD; "
        "}"
        "QPushButton:pressed { "
        "    background-color: #4A7BA7; "
        "}"
        "QLineEdit { "
        "    background-color: #2B2B2B; "
        "    color: #FFFFFF; "
        "    border: 1px solid #3D3D3D; "
        "    border-radius: 6px; "
        "    padding: 12px; "
        "    font-size: 14px; "
        "    selection-background-color: #5288C1; "
        "}"
        "QLineEdit:focus { "
        "    border: 1px solid #5288C1; "
        "    background-color: #1E1E1E; "
        "}"
        "QLineEdit::placeholder { "
        "    color: #707579; "
        "}"
    );

    // set style for SIGNUP text with Telegram accent color
    ui->signUpText->setStyleSheet(
        "background-color: transparent; "
        "color: #5288C1; "
        "font-size: 24px; "
        "font-weight: bold;"
    );


    // Connect socket signals
    connect(m_socket, &QTcpSocket::connected, this, &Registration::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &Registration::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &Registration::onReadyRead);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &Registration::onSocketError);

    // connect show button to the function
    connect(ui->showButton, &QPushButton::clicked, this, &Registration::showButtonPressed);

    // connect cancel button
    connect(ui->cancelButton, &QPushButton::clicked, this, &Registration::clearInput);

    // connect apply button - now calls the inherited connectToDatabase and registerUser
    connect(ui->applyButton, &QPushButton::clicked, this, &Registration::registerUser);

    // Set placeholder text for input fields
    ui->loginEdit->setPlaceholderText("Login");
    ui->nickEdit->setPlaceholderText("Nickname");
    ui->passwordEdit->setPlaceholderText("Password");
    
    // Set password field to hide text with bullets/asterisks
    ui->passwordEdit->setEchoMode(QLineEdit::Password);

    // set the length of input fields
    ui->loginEdit->setMaxLength(20);
    ui->nickEdit->setMaxLength(20);
    ui->passwordEdit->setMaxLength(20);
}

Registration::~Registration()
{
    // Database destructor will handle closing the connection
    delete ui;
}

// connect to the server
void Registration::connectToServer(const QString &host, quint16 port)
{
    qDebug() << "Connecting to authentication server:" << host << ":" << port;
    m_socket->connectToHost(host, port);
}

void Registration::showButtonPressed() {
    if(ui->passwordEdit->echoMode() == QLineEdit::Password) {
        ui->passwordEdit->setEchoMode(QLineEdit::Normal);
        ui->showButton->setText("Hide");
    } else if(ui->passwordEdit->echoMode() == QLineEdit::Normal) {
        ui->passwordEdit->setEchoMode(QLineEdit::Password);
        ui->showButton->setText("Show");
    }
}

void Registration::clearInput() {
    ui->loginEdit->clear();
    ui->nickEdit->clear();
    ui->passwordEdit->clear();
}

void Registration::registerUser()
{
    // Get input values
    m_pendingLogin = ui->loginEdit->text().trimmed();
    m_pendingPassword = ui->passwordEdit->text().trimmed();
    m_pendingUsername= ui ->nickEdit ->text().trimmed();

    // Validate input
    if (m_pendingLogin.isEmpty() || m_pendingPassword.isEmpty() || m_pendingUsername.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        return;
    }

    // Check the length of the login is at least 5 characters
    if (m_pendingLogin.size() < 5) {
        QMessageBox::critical(this, "Input error",
                              "Login must contain at least 5 characters");
        return;
    }

    // Check if connected to server
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::critical(this, "Connection Error",
                              "Not connected to server. Please try again.");
        return;
    }

    // Send authentication request to server
    QString regRequest = QString("REG:%1:%2:%3\n").arg(m_pendingLogin, m_pendingPassword, m_pendingUsername);
    m_socket->write(regRequest.toUtf8());
    m_socket->flush();

    qDebug() << "Sent authentication request for user:" << m_pendingLogin;
}

void Registration::onConnected()
{
    qDebug() << "Connected to authentication server";
}

void Registration::onDisconnected()
{
    qDebug() << "Connected to authentication server";
}

void Registration::onReadyRead()
{
    while (m_socket->canReadLine()) {
        QString response = QString::fromUtf8(m_socket->readLine()).trimmed();
        qDebug() << "Received from server:" << response;

        // Parse server response
        if (response.startsWith("REG_SUCCESS:")) {
            QString username = response.mid(12); // Extract username after "AUTH_SUCCESS:"

            QMessageBox::information(this, "Success",
                                     "Login successful!\n"
                                     "Welcome, " + username + "!");

            qDebug() << "User registered:" << m_pendingLogin << "Username:" << username;

            // Clear the input fields after successful login
            clearInput();

            // Emit success signal with username and password
            emit loginSuccessful(username, m_pendingPassword);

            accept();
        }
        else if (response == "REG_FAILED:") {
            QString reason = response.mid(12);
            QMessageBox::warning(this, "Registration Failed",
                                 reason);
            qDebug() << "Authentication failed for login:" << m_pendingLogin;
        }
        else if (response.startsWith("ERROR:")) {
            QString errorMsg = response.mid(6); // Extract error message
            QMessageBox::critical(this, "Server Error", errorMsg);
            qDebug() << "Server error:" << errorMsg;
        }
    }
}

void Registration::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString errorMsg = m_socket->errorString();
    qCritical() << "Socket error:" << errorMsg;

    QMessageBox::critical(this, "Connection Error",
                          "Failed to connect to server:\n" + errorMsg);

    emit serverConnectionFailed(errorMsg);
}


