#include "registration.h"
#include "./Registration/ui_registration.h"
#include "QLabel"
#include "QPushButton"
#include "QLineEdit"


Registration::Registration(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Registration)
    , passVal(" ")
    , nickVal(" ")
    , loginVal(" ")
{
    ui->setupUi(this);


    // set the title
    setWindowTitle("SignUp");

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

    //connect hide button
    connect(ui->hideButton,&QPushButton::clicked,this, &Registration::hideButtonPressed);

    // Set placeholder text for input fields
    ui->loginEdit->setPlaceholderText("Login");
    ui->nickEdit->setPlaceholderText("Nickname");
    ui->passwordEdit->setPlaceholderText("Password");
    
    // Set password field to hide text with bullets/asterisks
    ui->passwordEdit->setEchoMode(QLineEdit::Password);


}

Registration::~Registration()
{
    delete ui;
}

void Registration::showButtonPressed() {

    ui->passwordEdit -> setEchoMode(QLineEdit::Normal);

}

void Registration::hideButtonPressed() {

    ui->passwordEdit->setEchoMode(QLineEdit::Password);
}
