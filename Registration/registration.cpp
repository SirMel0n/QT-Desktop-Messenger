#include "registration.h"
#include "./Registration/ui_registration.h"
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSqlDatabase>
#include <QSqlError>

Registration::Registration(QWidget *parent)
    : Database(parent)  // Changed from QWidget to Database
    , ui(new Ui::Registration)
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

    // connect show button to the function
    connect(ui->showButton, &QPushButton::clicked, this, &Registration::showButtonPressed);

    // connect cancel button
    connect(ui->cancelButton, &QPushButton::clicked, this, &Registration::clearInput);

    // connect apply button - now calls the inherited connectToDatabase and registerUser
    connect(ui->applyButton, &QPushButton::clicked, this, [this]() {
        // Get input values
        loginVal = ui->loginEdit->text().trimmed();
        nickVal = ui->nickEdit->text().trimmed();
        passVal = ui->passwordEdit->text().trimmed();

        // Validate input
        if (loginVal.isEmpty() || nickVal.isEmpty() || passVal.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
            return;
        }

        // Connect to database using inherited method
        if (connectToDatabase()) {
            registerUser();
        }
    });

    // Set placeholder text for input fields
    ui->loginEdit->setPlaceholderText("Login");
    ui->nickEdit->setPlaceholderText("Nickname");
    ui->passwordEdit->setPlaceholderText("Password");
    
    // Set password field to hide text with bullets/asterisks
    ui->passwordEdit->setEchoMode(QLineEdit::Password);

    // set the length of input fields
    ui->loginEdit->setMaxLength(12);
    ui->nickEdit->setMaxLength(12);
    ui->passwordEdit->setMaxLength(12);
}

Registration::~Registration()
{
    // Database destructor will handle closing the connection
    delete ui;
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

bool Registration::registerUser()
{
    // verify connection
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Connection Error", "Database connection is not open.");
        return false;
    }

    // check the length of the login is at least 5 characters
    if(loginVal.size() <= 5) {
        QMessageBox::critical(this, "Input error",
                              "Login must contain minimum 5 characters");
        return false;
    }


    // check if username already exists
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    checkQuery.addBindValue(nickVal);

    if (checkQuery.exec() && checkQuery.next()) {
        int count = checkQuery.value(0).toInt();
        if (count > 0) {
            QMessageBox::warning(this, "Username Taken",
                                 "The username '" + nickVal + "' is already taken.\n"
                                 "Please choose a different username.");
            return false;
        }
    }

    // check if login already exists
    QSqlQuery checkLoginQuery(db);
    checkLoginQuery.prepare("SELECT COUNT(*) FROM users WHERE login = ?");
    checkLoginQuery.addBindValue(loginVal);

    if (checkLoginQuery.exec() && checkLoginQuery.next()) {
        int count = checkLoginQuery.value(0).toInt();
        if (count > 0) {
            QMessageBox::warning(this, "Login Taken",
                                 "The login '" + loginVal + "' is already taken.\n"
                                 "Please choose a different login.");
            return false;
        }
    }

    // prepare insert query
    QSqlQuery query(db);
    query.prepare("INSERT INTO users (username, password, login) VALUES (?, ?, ?)");
    query.addBindValue(nickVal);
    query.addBindValue(passVal);
    query.addBindValue(loginVal);

    // execute the query
    if (!query.exec()) {
        // handle specific error codes
        QString errorText = query.lastError().text();

        // check if the input value is already in the database
        if (errorText.contains("Duplicate entry")) {
            QMessageBox::warning(this, "Duplicate Entry",
                                 "This username or login already exists.\n"
                                 "Please try different credentials.");
        } else {
            QMessageBox::critical(this, "Registration Error",
                                  "Failed to register user:\n" + errorText);
            qDebug() << "SQL Error:" << query.lastError().text();
            qDebug() << "Driver Error:" << query.lastError().driverText();
            qDebug() << "Database Error:" << query.lastError().databaseText();
        }
        return false;
    }

    // display if the user is successfully registered
    QMessageBox::information(this, "Success",
                             "User registered successfully!\n"
                             "Username: " + nickVal);

    qDebug() << "User registered:" << loginVal;

    // clear the input fields after successful registration
    clearInput();

    return true;
}
