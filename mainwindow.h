#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "TcpSocket.h"
#include <QShortcut>
#include <QStringList>

class QListWidgetItem;
class QMenu;

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

    void on_btnMenu_clicked();
    void onMenuSettingsTriggered();
    void onMenuCreateGroupTriggered();

private:
    void closeSearchPanel();
    void showStatus(const QString &text);
    void appendChatBubble(const QString &user, const QString &body, bool outgoing, qint64 timestampMs);
    void setupSplitLayout();

    Ui::MainWindow *ui;
    TcpSocket *m_tcpSocket;
    QShortcut *shortcut;
    QMenu *m_mainMenu;

    QString m_login;
    QString m_password;
    QString m_displayName;
    QString m_activePeer;
    QString m_lastSearchQuery;
    bool m_isAuthenticated;
    bool m_searchOpen;
};

#endif // MAINWINDOW_H
