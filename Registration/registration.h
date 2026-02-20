#ifndef REGISTRATION_H
#define REGISTRATION_H

#include "../Database/database.h"
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>

namespace Ui {
class Registration;
}

class Registration : public Database
{
    Q_OBJECT

public:
    explicit Registration(QWidget *parent = nullptr);
    ~Registration();

private:
    Ui::Registration *ui;

private slots:
    void showButtonPressed();
    void clearInput();
    bool registerUser();
};

#endif // REGISTRATION_H
