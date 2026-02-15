#include "registration.h"
#include "./Registration/ui_registration.h"
#include "QLabel"
#include "QPushButton"
#include "QLineEdit"
#include "QSqlDatabase"
#include "QSqlError"


Registration::Registration(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Registration)
    , passVal(" ")
    , nickVal(" ")
    , loginVal(" ")
{
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

    //connect show button to the fucntion
    connect(ui->showButton, &QPushButton::clicked,this , &Registration::showButtonPressed);

    // connect cancel button
    connect(ui->cancelButton,&QPushButton::clicked,this,&Registration::clearInput);

    //connect apply button
    connect(ui->applyButton, &QPushButton::clicked,this, &Registration::connectToDatabase);

    // Set placeholder text for input fields
    ui->loginEdit->setPlaceholderText("Login");
    ui->nickEdit->setPlaceholderText("Nickname");
    ui->passwordEdit->setPlaceholderText("Password");
    
    // Set password field to hide text with bullets/asterisks
    ui->passwordEdit->setEchoMode(QLineEdit::Password);

    //set the length of input fields
    ui->loginEdit->setMaxLength(12);
    ui->nickEdit->setMaxLength(12);
    ui->passwordEdit->setMaxLength(12);


}

Registration::~Registration()
{
    // Close database connection
    if (db.isOpen()) {
        db.close();
    }
    delete ui;
}

void Registration::showButtonPressed() {

    if(ui->passwordEdit->echoMode()==QLineEdit::Password) {
        ui->passwordEdit -> setEchoMode(QLineEdit::Normal);
        ui ->showButton ->setText("Hide");
    } else if(ui->passwordEdit->echoMode()==QLineEdit::Normal) {
        ui->passwordEdit -> setEchoMode(QLineEdit::Password);
        ui->showButton->setText("Show");
    }

}

void Registration::clearInput() {
    ui->loginEdit->clear();
    ui->nickEdit->clear();
    ui->passwordEdit->clear();
}

// function for connection to the database
bool Registration::connectToDatabase()
{

    // get input values
    loginVal = ui->loginEdit->text().trimmed();
    nickVal = ui->nickEdit->text().trimmed();
    passVal = ui->passwordEdit->text().trimmed();

    // validate input
    if (loginVal.isEmpty() || nickVal.isEmpty() || passVal.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        return false;
    }

    // Check if connection already exists and is open
    if (db.isOpen()) {
        qDebug() << "database is connected";
        return true; // Already connected

    }

    // Check if database object has been initialized but not opened
    if (!db.isValid()) {
        // ads QODBC drivers
        db = QSqlDatabase::addDatabase("QODBC");

        // Set the connection details
        db.setDatabaseName("DRIVER={MySQL ODBC 9.6 Unicode Driver};"
                           "SERVER=localhost;"
                           "PORT=3306;"
                           "USER=qtapp;"
                           "PASSWORD=123456789;"
                           "OPTION=3;");
    }

    // Step 3: Open the connection
    if (!db.open()) {
        // If connection fails, show error message
        QMessageBox::critical(this, "Database Error",
                              "Failed to connect to database:\n" +
                                  db.lastError().text());
        return false;
    }

    return true;
}
