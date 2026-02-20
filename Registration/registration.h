#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <QWidget>
#include "QLabel"
#include "QPushButton"
#include "QLineEdit"
#include "QSqlDatabase"
#include "QSqlError"
#include "QSqlQuery"
#include "QMessageBox"
#include <QDebug>

namespace Ui {
class Registration;
}

class Registration : public QWidget
{
    Q_OBJECT

public:
    explicit Registration(QWidget *parent = nullptr);
    ~Registration();

private:
    Ui::Registration *ui;
    QString loginVal;
    QString nickVal;
    QString passVal;
    QSqlDatabase db;

private slots:
    void showButtonPressed();
    void clearInput();
    bool connectToDatabase();
    bool registerUser();
};

#endif // REGISTRATION_H
