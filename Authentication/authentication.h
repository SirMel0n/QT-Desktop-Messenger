#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <QDialog>
#include <QTcpSocket>
#include <QMessageBox>
#include <QDialog>
#include <QTcpSocket>
#include <QMessageBox>

namespace Ui {
class Authentication;
}

class Authentication : public QDialog
{
    Q_OBJECT

public:
    explicit Authentication(QWidget *parent = nullptr);
    virtual ~Authentication();

    void connectToServer(const QString &host, quint16 port);

private:
    Ui::Authentication *ui;
    QTcpSocket *m_socket;
    QString m_pendingLogin;
    QString m_pendingPassword;

private slots:
    void showButtonPressed();
    void clearInput();
    void authenticateUser();

    // Socket handlers
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);

signals:
    void loginSuccessful(const QString& username, const QString& password);
    void serverConnectionFailed(const QString& error);
};

#endif // AUTHENTICATION_H
