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
    m_stack->addWidget(createNotificationsPage());
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
    m_compactListCheck->setChecked(settings.value("ui/compactChatList", false).toBool());
    m_showTimestampsCheck->setChecked(settings.value("ui/showTimestamps", true).toBool());
    m_notifySoundCheck->setChecked(settings.value("ui/notifySound", true).toBool());
    m_notifyPreviewCheck->setChecked(settings.value("ui/notifyPreview", true).toBool());
    m_notifyDndCheck->setChecked(settings.value("ui/notifyDnd", false).toBool());
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

QWidget *SettingsDialog::createNotificationsPage()
{
    // create a page widget and a layout widget
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    // add title
    auto *titleLabel = new QLabel("Notifications", page);
    titleLabel->setStyleSheet("font-weight: 700; font-size: 16px; color: #e6ebf5;");

    // add desicription label
    auto *descLabel = new QLabel("Sound, preview text, and do-not-disturb.", page);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color: #9fb0c3;");

    // add groupbox that contains tick boxes
    auto *box = new QGroupBox("Options", page);
    auto *boxLayout = new QVBoxLayout(box);

    // add tick boxes
    m_notifySoundCheck = new QCheckBox("Play sound on new message", box);
    m_notifyPreviewCheck = new QCheckBox("Show message preview", box);
    m_notifyDndCheck = new QCheckBox("Do not disturb", box);

    // insert tick boxes in the box layout
    boxLayout->addWidget(m_notifySoundCheck);
    boxLayout->addWidget(m_notifyPreviewCheck);
    boxLayout->addWidget(m_notifyDndCheck);

    // display widgets on the page
    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);
    layout->addSpacing(6);
    layout->addWidget(box);
    layout->addStretch();

    return page;
}

QWidget *SettingsDialog::createPage(const QString &title, const QString &description)
{
    // create a page widget and a layout widget
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    // create title label
    auto *titleLabel = new QLabel(title, page);
    titleLabel->setStyleSheet("font-weight: 700; font-size: 16px; color: #e6ebf5;");

    // add description label
    auto *descLabel = new QLabel(description, page);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color: #9fb0c3;");

    // add tick boxes
    auto *box = new QGroupBox("Options", page);
    auto *boxLayout = new QVBoxLayout(box);
    boxLayout->addWidget(new QLabel("Add controls here...", box));

    // laod widgets
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
bool SettingsDialog::notifySound() const { return m_notifySoundCheck->isChecked(); }
bool SettingsDialog::notifyPreview() const { return m_notifyPreviewCheck->isChecked(); }
bool SettingsDialog::notifyDnd() const { return m_notifyDndCheck->isChecked(); }
