#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QListWidget;
class QStackedWidget;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private:
    QWidget *createPage(const QString &title, const QString &description);

    QListWidget *m_navList = nullptr;
    QStackedWidget *m_stack = nullptr;
};

#endif // SETTINGSDIALOG_H