#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QDebug>
#include <QDialog>
#include <QTcpSocket>
#include <QMessageBox>

namespace Ui {
class Registration;
}

class Registration : public QDialog
{
    Q_OBJECT

public:
    explicit Registration(QWidget *parent = nullptr);
    ~Registration();

    void connectToServer(const QString &host, quint16 port);

private:
    Ui::Registration *ui;
    QTcpSocket *m_socket;
    QString m_pendingLogin;
    QString m_pendingPassword;
    QString m_pendingUsername;

private slots:
    void showButtonPressed();
    void clearInput();
    void registerUser();

    // Socket handlers
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);

signals:

    void loginSuccessful(const QString& username, const QString& password);
    void serverConnectionFailed(const QString& error);
};

#endif // REGISTRATION_H
