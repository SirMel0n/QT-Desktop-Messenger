#include "authentication.h"
#include "ui_authentication.h"

Authentication::Authentication(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Authentication)
    , m_socket(new QTcpSocket(this))
{
    ui->setupUi(this);

    // set the title
    setWindowTitle("Sign In");

    // set the style sheet for the user interface
    this->setStyleSheet(
        "Authentication { "
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
        "QTextEdit { "
        "    background-color: transparent; "
        "    border: none; "
        "}"
        );

    // set style sheet for the QlineText
    ui->signInText->setStyleSheet(
        "background-color: transparent; "
        "color: #5288C1; "
        "font-size: 24px; "
        "font-weight: bold;"
        );

    // Connect socket signals
    connect(m_socket, &QTcpSocket::connected, this, &Authentication::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &Authentication::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &Authentication::onReadyRead);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &Authentication::onSocketError);

    // connect show button to the function
    connect(ui->showButton, &QPushButton::clicked, this, &Authentication::showButtonPressed);

    // connect cancel button to the function
    connect(ui->cancelButton, &QPushButton::clicked, this, &Authentication::clearInput);

    // Connect apply button to authenticate user
    connect(ui->applyButton, &QPushButton::clicked, this, &Authentication::authenticateUser);

    // set placeholder text for input fields
    ui->loginEdit->setPlaceholderText("Login");
    ui->passwordEdit->setPlaceholderText("Password");

    // set password field to hide text with bullets/asterisks
    ui->passwordEdit->setEchoMode(QLineEdit::Password);

    // set the length of input fields
    ui->loginEdit->setMaxLength(20);
    ui->passwordEdit->setMaxLength(20);
}

Authentication::~Authentication()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
    delete ui;
}

void Authentication::connectToServer(const QString &host, quint16 port)
{
    qDebug() << "Connecting to authentication server:" << host << ":" << port;
    m_socket->connectToHost(host, port);
}

void Authentication::showButtonPressed()
{
    if (ui->passwordEdit->echoMode() == QLineEdit::Password) {
        ui->passwordEdit->setEchoMode(QLineEdit::Normal);
        ui->showButton->setText("Hide");
    } else if (ui->passwordEdit->echoMode() == QLineEdit::Normal) {
        ui->passwordEdit->setEchoMode(QLineEdit::Password);
        ui->showButton->setText("Show");
    }
}

void Authentication::clearInput()
{
    ui->loginEdit->clear();
    ui->passwordEdit->clear();
}

void Authentication::authenticateUser()
{
    // Get input values
    m_pendingLogin = ui->loginEdit->text().trimmed();
    m_pendingPassword = ui->passwordEdit->text().trimmed();

    // Validate input
    if (m_pendingLogin.isEmpty() || m_pendingPassword.isEmpty()) {
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
    QString authRequest = QString("AUTH:%1:%2\n").arg(m_pendingLogin, m_pendingPassword);
    m_socket->write(authRequest.toUtf8());
    m_socket->flush();

    qDebug() << "Sent authentication request for user:" << m_pendingLogin;
}

void Authentication::onConnected()
{
    qDebug() << "Connected to authentication server";
}

void Authentication::onDisconnected()
{
    qDebug() << "Disconnected from authentication server";
}

void Authentication::onReadyRead()
{
    while (m_socket->canReadLine()) {
        QString response = QString::fromUtf8(m_socket->readLine()).trimmed();
        qDebug() << "Received from server:" << response;

        // Parse server response
        if (response.startsWith("AUTH_SUCCESS:")) {
            QString username = response.mid(13); // Extract username after "AUTH_SUCCESS:"

            QMessageBox::information(this, "Success",
                                     "Login successful!\n"
                                     "Welcome, " + username + "!");

            qDebug() << "User authenticated:" << m_pendingLogin << "Username:" << username;

            // Clear the input fields after successful login
            clearInput();

            // Emit success signal with username and password
            emit loginSuccessful(username, m_pendingPassword);

            accept();
        }
        else if (response == "AUTH_FAILED") {
            QMessageBox::warning(this, "Authentication Failed",
                                 "Invalid login or password.\n"
                                 "Please try again.");
            qDebug() << "Authentication failed for login:" << m_pendingLogin;
        }
        else if (response.startsWith("ERROR:")) {
            QString errorMsg = response.mid(6); // Extract error message
            QMessageBox::critical(this, "Server Error", errorMsg);
            qDebug() << "Server error:" << errorMsg;
        }
    }
}

void Authentication::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString errorMsg = m_socket->errorString();
    qCritical() << "Socket error:" << errorMsg;

    QMessageBox::critical(this, "Connection Error",
                          "Failed to connect to server:\n" + errorMsg);

    emit serverConnectionFailed(errorMsg);
}
