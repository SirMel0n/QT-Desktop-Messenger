#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "TcpSocket.h"
#include <QShortcut>
#include <QStringList>

class QListWidgetItem;
class QMenu;
class QPoint;

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

    void on_lstChat_customContextMenuRequested(const QPoint &pos);
    void onChatScrollValueChanged(int value);

private:
    void closeSearchPanel();
    void showStatus(const QString &text);
    void appendChatBubble(const QString &user, const QString &body, bool outgoing, qint64 timestampMs, const QString &messageId = QString(), bool isEdited = false, bool prepend = false);
    void setupSplitLayout();

    void requestHistoryPage(qint64 beforeId);

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

    qint64 m_oldestLoadedMessageId = 0;
    bool m_loadingHistory = false;
    bool m_hasMoreHistory = true;
    int m_historyPrevScrollValue = 0;
    int m_historyPrevScrollMax = 0;
    int m_historyInsertedCount = 0;
    bool m_prependHistoryBatch = false;
};

#endif // MAINWINDOW_H
