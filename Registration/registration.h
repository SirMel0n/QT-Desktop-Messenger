#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <QWidget>

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

private slots:
    void showButtonPressed();
    void clearInput();
};

#endif // REGISTRATION_H
