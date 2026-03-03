#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "TcpSocket.h"
#include "Authentication/authentication.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnected();
    void onDisconnected();
    void onMessageReceived(const QString &message);
    void onSocketError(const QString &errorMsg);
    void on_btnSend_clicked();
    void onLoginSuccessful(const QString &username);

private:
    Ui::MainWindow *ui;
    TcpSocket *m_tcpSocket;
    Authentication *m_auth;
};

#endif // MAINWINDOW_H
