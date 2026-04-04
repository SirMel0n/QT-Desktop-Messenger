#include "SettingsDialog.h"
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QSettings>
#include <QCoreApplication>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    // set the window
    setWindowTitle("Settings");
    setModal(true);
    resize(640, 420);

    // add list widgets
    m_navList = new QListWidget(this);
    m_navList->setFixedWidth(180);
    m_navList->addItem("Personalisation");
    m_navList->addItem("Notifications");
    m_navList->addItem("Privacy");
    m_navList->addItem("Account");
    m_navList->setCurrentRow(0);

    // add widgets
    m_stack = new QStackedWidget(this);
    m_stack->addWidget(createPersonalisationPage());
    m_stack->addWidget(createPage("Notifications", "Enable sound, preview text, do-not-disturb."));
    m_stack->addWidget(createPage("Privacy", "Online status visibility, read receipts."));
    m_stack->addWidget(createPage("Account", "Username and password"));

    // display the stack of widgets
    auto *contentLayout = new QHBoxLayout();
    contentLayout->addWidget(m_navList);
    contentLayout->addWidget(m_stack, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *root = new QVBoxLayout(this);
    root->addLayout(contentLayout, 1);
    root->addWidget(buttons);

    connect(m_navList, &QListWidget::currentRowChanged, m_stack, &QStackedWidget::setCurrentIndex);

    const QString settingsPath = QCoreApplication::applicationDirPath() + "/ui_config.ini";
    QSettings settings(settingsPath, QSettings::IniFormat);
    m_themeCombo->setCurrentText(settings.value("ui/theme", "Dark").toString());
    m_accentCombo->setCurrentText(settings.value("ui/accent", "Blue").toString());
    m_fontSizeCombo->setCurrentText(settings.value("ui/fontSize", "Medium").toString());
    m_showTimestampsCheck->setChecked(settings.value("ui/showTimestamps", true).toBool());
}

QWidget *SettingsDialog::createPersonalisationPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *titleLabel = new QLabel("Personalisation", page);
    titleLabel->setStyleSheet("font-weight: 700; font-size: 16px; color: #e6ebf5;");

    auto *descLabel = new QLabel("Theme, accent color, font size, and layout options.", page);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color: #9fb0c3;");

    auto *box = new QGroupBox("Appearance", page);
    auto *boxLayout = new QVBoxLayout(box);

    m_themeCombo = new QComboBox(box);
    m_themeCombo->addItems({ "Dark", "Light", "System" });

    m_accentCombo = new QComboBox(box);
    m_accentCombo->addItems({ "Blue", "Slate", "Emerald", "Purple" });

    m_fontSizeCombo = new QComboBox(box);
    m_fontSizeCombo->addItems({ "Small", "Medium", "Large" });

    m_compactListCheck = new QCheckBox("Compact chat list", box);
    m_showTimestampsCheck = new QCheckBox("Show message timestamps", box);

    boxLayout->addWidget(new QLabel("Theme", box));
    boxLayout->addWidget(m_themeCombo);
    boxLayout->addWidget(new QLabel("Accent color", box));
    boxLayout->addWidget(m_accentCombo);
    boxLayout->addWidget(new QLabel("Font size", box));
    boxLayout->addWidget(m_fontSizeCombo);
    boxLayout->addSpacing(6);
    boxLayout->addWidget(m_compactListCheck);
    boxLayout->addWidget(m_showTimestampsCheck);

    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);
    layout->addSpacing(6);
    layout->addWidget(box);
    layout->addStretch();

    return page;
}

QWidget *SettingsDialog::createPage(const QString &title, const QString &description)
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *titleLabel = new QLabel(title, page);
    titleLabel->setStyleSheet("font-weight: 700; font-size: 16px; color: #e6ebf5;");

    auto *descLabel = new QLabel(description, page);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color: #9fb0c3;");

    auto *box = new QGroupBox("Options", page);
    auto *boxLayout = new QVBoxLayout(box);
    boxLayout->addWidget(new QLabel("Add controls here...", box));

    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);
    layout->addSpacing(6);
    layout->addWidget(box);
    layout->addStretch();

    return page;
}

QString SettingsDialog::selectedTheme() const { return m_themeCombo->currentText(); }
QString SettingsDialog::selectedAccent() const { return m_accentCombo->currentText(); }
QString SettingsDialog::selectedFontSize() const { return m_fontSizeCombo->currentText(); }
bool SettingsDialog::showTimestamps() const { return m_showTimestampsCheck->isChecked(); }
bool SettingsDialog::compactChatList() const { return m_compactListCheck->isChecked(); }
