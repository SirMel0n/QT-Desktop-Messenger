#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "TcpSocket.h"
#include <QShortcut>
#include <QStringList>
#include <QSet>

class QListWidgetItem;
class QMenu;
class QPoint;
class QSoundEffect;
class QSystemTrayIcon;

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
    void applyAppearanceSettingsFromSettings();
    void applyAppearanceSettings(const QString &theme, const QString &accent, const QString &fontSize, bool showTimestamps, bool compactChatList);
    void applyThemeStyles();
    QString resolveAccentColor(const QString &accent) const;
    int resolveFontSize(const QString &fontSize) const;

    void applyNotificationSettings(bool notifySound, bool notifyPreview, bool notifyDnd);
    void showMessageNotification(const QString &fromUser, const QString &body);
    void playMessageSound();
    void ensureTrayIcon();
    void playSendSound();

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
    QSet<QString> m_readMessageIds;
    qint64 m_lastIncomingSendSoundMs = 0;

    QString m_themeName = "Dark";
    QString m_accentName = "Blue";
    QString m_fontSizeName = "Medium";
    QString m_accentColor = "#2f6ea5";
    int m_uiFontSize = 13;
    bool m_showTimestamps = true;

    bool m_notifySound = true;
    bool m_notifyPreview = true;
    bool m_notifyDnd = false;

    QSystemTrayIcon *m_trayIcon = nullptr;
    QSoundEffect *m_messageSound = nullptr;
    QSoundEffect *m_sendSound = nullptr;
};

#endif // MAINWINDOW_H
