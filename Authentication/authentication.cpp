#include "authentication.h"
#include "ui_authentication.h"
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSqlDatabase>
#include <QSqlError>

Authentication::Authentication(QWidget *parent)
    : Database(parent)
    , ui(new Ui::Authentication)
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

    // connect show button to the function
    connect(ui->showButton, &QPushButton::clicked, this, &Authentication::showButtonPressed);

    // connect cancel button to the function
    connect(ui->cancelButton, &QPushButton::clicked, this, &Authentication::clearInput);

    // Connect apply button, so it will call the inherited connectToDatabase and authenticateUser
    connect(ui->applyButton, &QPushButton::clicked, this, [this]() {
        // Get input values
        loginVal = ui->loginEdit->text().trimmed();
        passVal = ui->passwordEdit->text().trimmed();

        // Validate input
        if (loginVal.isEmpty() || passVal.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
            return;
        }

        // Connect to database using inherited method
        if (connectToDatabase()) {
            authenticateUser();
        }
    });

    // set placeholder text for input fields
    ui->loginEdit->setPlaceholderText("Login");
    ui->passwordEdit->setPlaceholderText("Password");

    // set password field to hide text with bullets/asterisks
    ui->passwordEdit->setEchoMode(QLineEdit::Password);

    // set the length of input fields
    ui->loginEdit->setMaxLength(12);
    ui->passwordEdit->setMaxLength(12);
}

Authentication::~Authentication()
{
    delete ui;
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

bool Authentication::authenticateUser()
{
    // Verify connection
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Connection Error", "Database connection is not open.");
        return false;
    }

    // Escape single quotes to prevent SQL injection
    QString escapedLogin = loginVal;
    QString escapedPass = passVal;
    
    escapedLogin.replace("'", "''");
    escapedPass.replace("'", "''");

    // Build query directly (workaround for ODBC issues)
    QString queryStr = QString("SELECT username FROM users WHERE login = '%1' AND password = '%2'")
                           .arg(escapedLogin, escapedPass);

    qDebug() << "Executing authentication query for user:" << loginVal;

    QSqlQuery query(db);

    // Execute the query directly
    if (!query.exec(queryStr)) {
        QMessageBox::critical(this, "Authentication Error",
                              "Failed to authenticate:\n" +
                                  query.lastError().text());
        qDebug() << "SQL Error:" << query.lastError().text();
        qDebug() << "Driver Error:" << query.lastError().driverText();
        qDebug() << "Database Error:" << query.lastError().databaseText();
        return false;
    }


    // check the length of the login is at least 5 characters
    if(loginVal.size() <= 5) {
        QMessageBox::critical(this, "Input error",
                              "Login must contain minimum 5 characters");
        return false;
    }

    // Check if user found
    if (query.next()) {
        QString username = query.value(0).toString();

        QMessageBox::information(this, "Success",
                                 "Login successful!\n"
                                 "Welcome, " + username + "!");

        qDebug() << "User authenticated:" << loginVal;

        // Clear the input fields after successful login
        clearInput();

        // TODO: Navigate to main application window
        emit loginSuccessful(username);

        return true;
    } else {
        QMessageBox::warning(this, "Authentication Failed",
                             "Invalid login or password.\n"
                             "Please try again.");
        qDebug() << "No matching user found for login:" << loginVal;
        return false;
    }
}
