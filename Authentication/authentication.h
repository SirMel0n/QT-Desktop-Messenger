#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include "../Database/database.h"

namespace Ui {
class Authentication;
}

class Authentication : public QDialog
{
    Q_OBJECT

public:
    explicit Authentication(QWidget *parent = nullptr);
    ~Authentication() override;

private:
    Ui::Authentication *ui;
    Database *m_database;

private slots:
    void showButtonPressed();
    void clearInput();
    bool authenticateUser();

signals:
    void loginSuccessful(const QString& username);

};

#endif // AUTHENTICATION_H
