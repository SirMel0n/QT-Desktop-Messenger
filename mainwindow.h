#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "TcpSocket.h"
#include <QShortcut>
#include <QStringList>

class QListWidgetItem;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &login, const QString &password, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnected();
    void onDisconnected();
    void onResponseReceived(const QString &message);
    void onSocketError(const QString &errorMsg);
    void on_btnSend_clicked();
    void on_btnSearch_clicked();
    void on_lstUsers_itemClicked(QListWidgetItem *item);

private:
    void closeSearchPanel();
    void showStatus(const QString &text);
    void appendChatBubble(const QString &user, const QString &body, bool outgoing);
    void setupSplitLayout();

    Ui::MainWindow *ui;
    TcpSocket *m_tcpSocket;
    QShortcut *shortcut;

    QString m_login;
    QString m_password;
    QString m_displayName;
    QString m_activePeer;
    QString m_lastSearchQuery;
    bool m_isAuthenticated;
    bool m_searchOpen;
};

#endif // MAINWINDOW_H
