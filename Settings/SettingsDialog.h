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

private:
    QWidget *createPersonalisationPage();
    QWidget *createPage(const QString &title, const QString &description);

    QListWidget *m_navList = nullptr;
    QStackedWidget *m_stack = nullptr;

    QComboBox *m_themeCombo = nullptr;
    QComboBox *m_accentCombo = nullptr;
    QComboBox *m_fontSizeCombo = nullptr;
    QCheckBox *m_compactListCheck = nullptr;
    QCheckBox *m_showTimestampsCheck = nullptr;
};

#endif // SETTINGSDIALOG_H
