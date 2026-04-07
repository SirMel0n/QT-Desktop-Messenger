#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QListWidget;
class QStackedWidget;
class QComboBox;
class QCheckBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    QString selectedTheme() const;
    QString selectedAccent() const;
    QString selectedFontSize() const;
    bool showTimestamps() const;
    bool compactChatList() const;

    bool notifySound() const;
    bool notifyPreview() const;
    bool notifyDnd() const;

private:
    QWidget *createPersonalisationPage();
    QWidget *createNotificationsPage();
    QWidget *createPage(const QString &title, const QString &description);

    QListWidget *m_navList = nullptr;
    QStackedWidget *m_stack = nullptr;

    QComboBox *m_themeCombo = nullptr;
    QComboBox *m_accentCombo = nullptr;
    QComboBox *m_fontSizeCombo = nullptr;
    QCheckBox *m_compactListCheck = nullptr;
    QCheckBox *m_showTimestampsCheck = nullptr;

    QCheckBox *m_notifySoundCheck = nullptr;
    QCheckBox *m_notifyPreviewCheck = nullptr;
    QCheckBox *m_notifyDndCheck = nullptr;
};

#endif // SETTINGSDIALOG_H
