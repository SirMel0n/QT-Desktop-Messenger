#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include "../Database/database.h"

namespace Ui {
class Authentication;
}

class Authentication : public Database
{
    Q_OBJECT

public:
    explicit Authentication(QWidget *parent = nullptr);
    ~Authentication() override;

private:
    Ui::Authentication *ui;

private slots:
    void showButtonPressed();
    void clearInput();
    bool authenticateUser();

signals:
    void loginSuccessful(const QString& username);
};

#endif // AUTHENTICATION_H
