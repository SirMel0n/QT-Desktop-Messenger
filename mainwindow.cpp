#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QShortcut>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include "ConfigManager.h"
#include <QDateTime>
#include <QScrollBar>
#include <QFrame>
#include "Settings/SettingsDialog.h"
#include <QSettings>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QColor>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QSoundEffect>
#include <QSystemTrayIcon>
#include <QUrl>

MainWindow::MainWindow(const QString &login, const QString &password, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tcpSocket(new TcpSocket(this))
    , shortcut(new QShortcut(QKeySequence(Qt::Key_Return), this))
    , m_mainMenu(new QMenu(this))
    , m_login(login)
    , m_password(password)
    , m_isAuthenticated(false)
    , m_searchOpen(false)
{
    ui->setupUi(this);
    setupSplitLayout();

    // add message input place holder
    ui->lnMessage->setPlaceholderText("Write a mesasage ... ");

    // Telegram-like style for chat list
    ui->lstUsers->setSpacing(4);
    ui->lstUsers->setStyleSheet(R"(
        QListWidget {
            background-color: #17212b;
            border: 1px solid #0e1621;
            border-radius: 10px;
            outline: none;
            color: #e6ebf5;
            padding: 6px;
        }

        QListWidget::item {
            background: transparent;
            border: none;
            border-radius: 10px;
            padding: 10px 12px;
            margin: 2px 0px;
            min-height: 42px;
        }

        QListWidget::item:hover {
            background-color: #202b36;
        }

        QListWidget::item:selected {
            background-color: #2b5278;
            color: #ffffff;
        }

        QListWidget::item:selected:active {
            background-color: #2f6ea5;
        }

        QScrollBar:vertical {
            background: #17212b;
            width: 10px;
            margin: 6px 2px 6px 2px;
            border: none;
        }

        QScrollBar::handle:vertical {
            background: #3a4a5c;
            min-height: 24px;
            border-radius: 5px;
        }

        QScrollBar::handle:vertical:hover {
            background: #4a5e74;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical,
        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            background: none;
            border: none;
            height: 0px;
        }
    )");

    // Top/search area style to match Telegram-like palette
    ui->btnMenu->setStyleSheet(R"(
        QPushButton {
            background-color: #1f2c3a;
            color: #e6ebf5;
            border: 1px solid #34495e;
            border-radius: 9px;
            padding: 6px 14px;
            font-weight: 600;
        }
        QPushButton:hover { background-color: #27384a; }
        QPushButton:pressed { background-color: #2f6ea5; }
    )");

    ui->btnSearch->setStyleSheet(R"(
        QPushButton {
            background-color: #2f6ea5;
            color: #ffffff;
            border: 1px solid #3f82bf;
            border-radius: 9px;
            padding: 6px 14px;
            font-weight: 600;
        }
        QPushButton:hover { background-color: #3a7fba; }
        QPushButton:pressed { background-color: #2a6292; }
    )");

    ui->lnSearch->setStyleSheet(R"(
        QLineEdit {
            background-color: #1f2c3a;
            color: #e6ebf5;
            border: 1px solid #34495e;
            border-radius: 9px;
            padding: 6px 10px;
            selection-background-color: #2f6ea5;
        }
    )");

    // Username row container (label + edit) styling
    if (ui->layoutWidget) {
        ui->layoutWidget->setStyleSheet(R"(
            QLabel {
                color: #dbe6f3;
                font-weight: 600;
            }
            QLineEdit {
                background-color: #1f2c3a;
                color: #e6ebf5;
                border: 1px solid #34495e;
                border-radius: 9px;
                padding: 6px 10px;
                min-height: 18px;
            }
        )");
    }

    // Chat window frame style
    ui->groupBox->setStyleSheet(R"(
        QGroupBox {
            border: 1px solid #0e1621;
            border-radius: 12px;
            margin-top: 10px;
            background-color: #17212b;
            color: #e6ebf5;
            font-weight: 600;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 6px;
            color: #9fb0c3;
        }
    )");

    // Chat message list background + scrollbar
    ui->lstChat->setStyleSheet(R"(
        QListWidget {
            background-color: #0f1822;
            border: 1px solid #223345;
            border-radius: 10px;
            outline: none;
            padding: 6px;
        }

        QListWidget::item {
            border: none;
            margin: 2px 0px;
        }

        QScrollBar:vertical {
            background: #0f1822;
            width: 10px;
            margin: 6px 2px 6px 2px;
            border: none;
        }

        QScrollBar::handle:vertical {
            background: #3a4a5c;
            min-height: 24px;
            border-radius: 5px;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical,
        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            background: none;
            border: none;
            height: 0px;
        }
    )");

    connect(m_tcpSocket, &TcpSocket::connected, this, &MainWindow::onConnected);
    connect(m_tcpSocket, &TcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(m_tcpSocket, &TcpSocket::messageReceived, this, &MainWindow::onResponseReceived);
    connect(m_tcpSocket, &TcpSocket::errorOccurred, this, &MainWindow::onSocketError);

    connect(shortcut, &QShortcut::activated, this, &MainWindow::on_btnSend_clicked);

    ui->lstChat->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->lstChat, &QListWidget::customContextMenuRequested,
            this, &MainWindow::on_lstChat_customContextMenuRequested);

    QAction *settingsAction = m_mainMenu->addAction("Settings");
    QAction *createGroupAction = m_mainMenu->addAction("Create Group Chat");

    connect(settingsAction, &QAction::triggered, this, &MainWindow::onMenuSettingsTriggered);
    connect(createGroupAction, &QAction::triggered, this, &MainWindow::onMenuCreateGroupTriggered);

    ui->btnMenu->setText("Menu");
    ui->btnMenu->setFixedWidth(72);

    ui->lblActiveSession->setText("Active session: none");
    showStatus("Connecting...");

    m_tcpSocket->connectToServer(ConfigManager::instance().IpServer(), ConfigManager::instance().serverPort());

    connect(ui->lstChat->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &MainWindow::onChatScrollValueChanged);

    applyAppearanceSettingsFromSettings();
}

void MainWindow::setupSplitLayout()
{
    QWidget *central = ui->centralwidget;
    if (!central) {
        return;
    }

    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    auto *splitter = new QSplitter(Qt::Horizontal, central);

    auto *leftPane = new QWidget(splitter);
    auto *leftLayout = new QVBoxLayout(leftPane);
    leftLayout->setContentsMargins(8, 8, 8, 8);
    leftLayout->setSpacing(8);

    auto *topRow = new QHBoxLayout();
    topRow->setContentsMargins(0, 0, 0, 0);
    topRow->setSpacing(8);
    topRow->addWidget(ui->btnMenu, 0);
    topRow->addWidget(ui->layoutWidget, 1);

    leftLayout->addLayout(topRow);
    leftLayout->addWidget(ui->lstUsers, 1);

    auto *rightPane = new QWidget(splitter);
    auto *rightLayout = new QVBoxLayout(rightPane);
    rightLayout->setContentsMargins(8, 8, 8, 8);
    rightLayout->setSpacing(8);
    rightLayout->addWidget(ui->groupBox);

    splitter->setChildrenCollapsible(false);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    splitter->setSizes({320, 520});

    rootLayout->addWidget(splitter);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showStatus(const QString &text)
{
    statusBar()->showMessage(text, 4000);
}

void MainWindow::closeSearchPanel()
{
    m_searchOpen = false;

    ui->label_2->show();
    ui->lnSearch->show();
    ui->btnSearch->setText("Search");

    ui->lstUsers->clear();

    // reload existing chats list
    m_tcpSocket->sendMessage("CHAT_LIST\n");
}

void MainWindow::appendChatBubble(const QString &user, const QString &body, bool outgoing, qint64 timestampMs, const QString &messageId, bool isEdited, bool prepend)
{
    if (user.isEmpty() || body.isEmpty()) {
        return;
    }
    const bool isLight = (m_themeName == "Light");
    const QString textColor = isLight ? "#1c2733" : "#eaf2ff";
    const QString metaColor = isLight ? "#64748b" : "#aab7c4";

    const QColor accentColor(m_accentColor);
    const QString accentLight = accentColor.lighter(isLight ? 170 : 150).name();
    const QString accentDark = accentColor.darker(isLight ? 140 : 130).name();

    const QString outgoingNameColor = isLight ? accentDark : accentLight;
    const QString incomingNameColor = isLight ? "#b03a3a" : "#ff9b9b";

    const QString outgoingBubble = isLight ? accentLight : accentDark;
    const QString outgoingBorder = "transparent";
    const QString incomingBubble = isLight ? "#f0f4f8" : "#182533";
    const QString incomingBorder = isLight ? "#d0d9e3" : "#2c3f54";

    QListWidgetItem *item = new QListWidgetItem;
    item->setData(Qt::UserRole, messageId);
    item->setData(Qt::UserRole + 1, body);
    item->setData(Qt::UserRole + 2, user);
    item->setData(Qt::UserRole + 3, timestampMs);
    item->setData(Qt::UserRole + 4, outgoing);
    item->setData(Qt::UserRole + 5, isEdited);

    QWidget *rowWidget = new QWidget;
    QVBoxLayout *rowLayout = new QVBoxLayout(rowWidget);
    rowLayout->setContentsMargins(8, 4, 8, 4);
    rowLayout->setSpacing(0);

    QFrame *bubbleFrame = new QFrame(rowWidget);
    bubbleFrame->setFrameShape(QFrame::NoFrame);
    bubbleFrame->setObjectName(outgoing ? "outgoingBubble" : "incomingBubble");

    bubbleFrame->setStyleSheet(outgoing
        ? QString("QFrame#outgoingBubble {"
                  "background-color: %1;"
                  "border: none;"
                  "border-radius: 14px;"
                  "}").arg(outgoingBubble)
        : QString("QFrame#incomingBubble {"
                  "background-color: %1;"
                  "border: none;"
                  "border-radius: 14px;"
                  "}").arg(incomingBubble));

    const int maxBubbleWidth = qMax(260, (ui->lstChat->viewport()->width() * 72) / 100);
    bubbleFrame->setMaximumWidth(maxBubbleWidth);

    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubbleFrame);
    // increased bottom margin so letters like g/y/p are not clipped
    bubbleLayout->setContentsMargins(10, 8, 10, 11);
    bubbleLayout->setSpacing(4);

    QWidget *headerWidget = new QWidget(bubbleFrame);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(6);

    QLabel *nameLabel = new QLabel(QString("(%1)").arg(user), headerWidget);
    nameLabel->setObjectName("msgNameLabel");
    nameLabel->setStyleSheet(outgoing
                                 ? QString("color: %1; font-weight: 700;").arg(outgoingNameColor)
                                 : QString("color: %1; font-weight: 700;").arg(incomingNameColor));

    QLabel *timeLabel = new QLabel(headerWidget);
    timeLabel->setObjectName("msgTimeLabel");
    timeLabel->setStyleSheet(QString("color: %1; font-size: 11px;")
                                 .arg(outgoing ? outgoingNameColor : metaColor));

    if (timestampMs > 0) {
        const QDateTime localDt = QDateTime::fromMSecsSinceEpoch(timestampMs, Qt::UTC).toLocalTime();
        if (localDt.isValid()) {
            timeLabel->setText(localDt.toString("yyyy-MM-dd HH:mm"));
        }
    }

    QLabel *editedLabel = new QLabel("edited", headerWidget);
    editedLabel->setObjectName("msgEditedLabel");
    editedLabel->setStyleSheet(QString("color: %1; font-size: 11px; font-style: italic;").arg(metaColor));
    editedLabel->setVisible(isEdited);

    QLabel *statusLabel = new QLabel(bubbleFrame);

    statusLabel->setObjectName("msgStatusLabel");
    statusLabel->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 700;")
                                   .arg(outgoingNameColor));
    statusLabel->setVisible(outgoing);

    if (outgoing && !messageId.isEmpty() && m_readMessageIds.contains(messageId)) {
        statusLabel->setText("✓✓");
    } else {
        statusLabel->setText("✓");
    }

    headerLayout->addWidget(nameLabel);
    headerLayout->addWidget(timeLabel);
    headerLayout->addWidget(editedLabel);
    headerLayout->addStretch();

    // Body row: text + status tick at bottom-right
    QHBoxLayout *bodyLayout = new QHBoxLayout();
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(6);

    QLabel *msgLabel = new QLabel(body, bubbleFrame);
    msgLabel->setObjectName("msgBodyLabel");
    msgLabel->setWordWrap(true);
    msgLabel->setStyleSheet(QString("color: %1; font-size: 13px;").arg(textColor));
    msgLabel->setContentsMargins(0, 0, 0, 2);

    bodyLayout->addWidget(msgLabel, 1);

    if (outgoing) {
        statusLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
        statusLabel->setContentsMargins(0, 0, 0, 1);
        bodyLayout->addWidget(statusLabel, 0, Qt::AlignRight | Qt::AlignBottom);
    }

    bubbleLayout->addWidget(headerWidget);
    bubbleLayout->addLayout(bodyLayout);

    rowLayout->addWidget(bubbleFrame, 0, outgoing ? Qt::AlignRight : Qt::AlignLeft);

    // add tiny extra vertical slack to item
    item->setSizeHint(rowWidget->sizeHint() + QSize(0, 4));

    int insertRow = -1;

    if (prepend) {
        insertRow = 0;
    } else {
        // Keep chronological order for live messages too
        for (int i = 0; i < ui->lstChat->count(); ++i) {
            QListWidgetItem *existing = ui->lstChat->item(i);
            if (!existing) {
                continue;
            }

            const qint64 existingTs = existing->data(Qt::UserRole + 3).toLongLong();
            if (timestampMs > 0 && existingTs > 0) {
                if (timestampMs < existingTs) {
                    insertRow = i;
                    break;
                }

                if (timestampMs == existingTs) {
                    bool newOk = false;
                    bool oldOk = false;
                    const qint64 newId = messageId.toLongLong(&newOk);
                    const qint64 oldId = existing->data(Qt::UserRole).toString().toLongLong(&oldOk);
                    if (newOk && oldOk && newId < oldId) {
                        insertRow = i;
                        break;
                    }
                }
            }
        }

        if (insertRow == -1) {
            insertRow = ui->lstChat->count();
        }
    }

    ui->lstChat->insertItem(insertRow, item);
    ui->lstChat->setItemWidget(item, rowWidget);

    // Auto-scroll only when inserted at the end
    if (insertRow == ui->lstChat->count() - 1) {
        ui->lstChat->scrollToBottom();
    }

}

void MainWindow::onConnected()
{
    showStatus("Connected to server");
    const QString authRequest = QString("AUTH:%1:%2\n").arg(m_login, m_password);
    m_tcpSocket->sendMessage(authRequest);
}

void MainWindow::onDisconnected()
{
    m_isAuthenticated = false;
    m_activePeer.clear();
    ui->lblActiveSession->setText("Active session: none");
    ui->lstChat->clear();
    closeSearchPanel();
    showStatus("Disconnected from server");
}

void MainWindow::onResponseReceived(const QString &message)
{
    qDebug() << "MainWindow received:" << message;

    if (message.startsWith("AUTH_SUCCESS:")) {
        m_isAuthenticated = true;
        m_displayName = message.mid(QString("AUTH_SUCCESS:").size()).trimmed();
        showStatus("Authenticated as " + m_displayName);
        m_tcpSocket->sendMessage("PULL_PENDING\n");
        m_tcpSocket->sendMessage("CHAT_LIST\n");
        return;
    }

    if (message.startsWith("AUTH_FAILED:")) {
        m_isAuthenticated = false;
        showStatus(message);
        return;
    }

    if (message.startsWith("ERROR:")) {
        qWarning() << "Server error response:" << message;
        showStatus(message);
        return;
    }

    // NEW: passive incoming session notification (Telegram-style)
    if (message.startsWith("SESSION_INVITE:")) {
        const QString fromUser = message.mid(QString("SESSION_INVITE:").size()).trimmed();
        if (!fromUser.isEmpty()) {
            showStatus(fromUser + " started a chat with you");
        }

        if (m_isAuthenticated) {
            m_tcpSocket->sendMessage("CHAT_LIST\n");
        }
        return;
    }

    if (message.startsWith("SEARCH_EMPTY")) {
        ui->lstUsers->clear();
        QListWidgetItem *item = nullptr;
        if (m_searchOpen) {
            item = new QListWidgetItem("No online users found");
        } else {
            item = new QListWidgetItem("No chats yet");
        }
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        ui->lstUsers->addItem(item);
        return;
    }

    if (message.startsWith("SEARCH_RESULT:")) {
    const QString payload = message.mid(QString("SEARCH_RESULT:").size());
    const QStringList entries = payload.split(';', Qt::SkipEmptyParts);

    ui->lstUsers->clear();

    for (const QString &entry : entries) {
        const QStringList parts = entry.split('|');
        const QString username = parts.value(0).trimmed();
        const QString status = parts.value(1).trimmed();

        if (username.isEmpty()) {
            continue;
        }

        const bool isOnline = (status == "active" || status == "active_session");
        const bool hasSession = (status == "session" || status == "active_session");

        // Search mode: show both online users and existing chats
        if (m_searchOpen && !(isOnline || hasSession)) {
            continue;
        }

        // Normal mode: show existing chats only
        if (!m_searchOpen && !hasSession) {
            continue;
        }

        QString subtitle;
        if (status == "active_session") {
            subtitle = "online • chat";
        } else if (status == "active") {
            subtitle = "online";
        } else {
            subtitle = "chat";
        }

        QListWidgetItem *item = new QListWidgetItem(ui->lstUsers);
        item->setData(Qt::UserRole, username);
        item->setSizeHint(QSize(0, 64));

        QWidget *row = new QWidget(ui->lstUsers);
        row->setAttribute(Qt::WA_TransparentForMouseEvents, true);

        QVBoxLayout *root = new QVBoxLayout(row);
        root->setContentsMargins(10, 6, 10, 6);
        root->setSpacing(3);

        QHBoxLayout *top = new QHBoxLayout();
        top->setContentsMargins(0, 0, 0, 0);
        top->setSpacing(6);

        const bool isLight = (m_themeName == "Light");
        const QString listNameColor = isLight ? "#111827" : "#e6ebf5";
        const QString listMetaColor = isLight ? "#4b5563" : "#9fb0c3";
        const QString listTimeColor = isLight ? "#4b5563" : "#7f91a4";

        QLabel *nameLabel = new QLabel(username, row);
        nameLabel->setObjectName("userNameLabel");
        nameLabel->setStyleSheet(QString("color: %1; font-weight: 700;").arg(listNameColor));

        QLabel *onlineLabel = new QLabel(isOnline ? "●" : "", row);
        onlineLabel->setObjectName("onlineDotLabel");
        onlineLabel->setStyleSheet(QString("color: %1; font-size: 10px;").arg(m_accentColor));

        top->addWidget(nameLabel);
        top->addWidget(onlineLabel);
        top->addStretch();

        root->addLayout(top);

        ui->lstUsers->addItem(item);
        ui->lstUsers->setItemWidget(item, row);
    }

    if (ui->lstUsers->count() == 0) {
        QListWidgetItem *item = nullptr;
        if (m_searchOpen) {
            item = new QListWidgetItem("No online users found");
        } else {
            item = new QListWidgetItem("No chats yet");
        }
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        ui->lstUsers->addItem(item);
    }

    return;
}

    if (message.startsWith("SESSION_CREATED:")) {
        m_activePeer = message.mid(QString("SESSION_CREATED:").size()).trimmed();
        ui->lblActiveSession->setText("Active session: " + m_activePeer);
        ui->lstChat->clear();

        m_oldestLoadedMessageId = 0;
        m_loadingHistory = false;
        m_hasMoreHistory = true;
        m_historyInsertedCount = 0;

        showStatus("Session active with " + m_activePeer);
        requestHistoryPage(0);
        closeSearchPanel();
        return;
    }

    if (message.startsWith("SESSION_FAILED:")) {
        showStatus(message);
        return;
    }

    if (message.startsWith("MSG:")) {
        // New format: MSG:<sender>:<timestampMs>:<messageId>:<body>
        // Backward-compatible fallback: MSG:<sender>:<timestampMs>:<body>
        const int firstColon = message.indexOf(':');
        const int secondColon = message.indexOf(':', firstColon + 1);
        const int thirdColon = message.indexOf(':', secondColon + 1);
        const int fourthColon = message.indexOf(':', thirdColon + 1);

        if (firstColon == -1 || secondColon == -1 || thirdColon == -1) {
            qDebug() << "Invalid MSG format:" << message;
            return;
        }

        const QString user = message.mid(firstColon + 1, secondColon - firstColon - 1).trimmed();
        const QString tsStr = message.mid(secondColon + 1, thirdColon - secondColon - 1).trimmed();

        QString messageId;
        QString body;

        if (fourthColon == -1) {
            // old: MSG:<sender>:<timestampMs>:<body>
            body = message.mid(thirdColon + 1).trimmed();
        } else {
            // new: MSG:<sender>:<timestampMs>:<messageId>:<body>
            messageId = message.mid(thirdColon + 1, fourthColon - thirdColon - 1).trimmed();
            body = message.mid(fourthColon + 1).trimmed();
        }

        bool ok = false;
        const qint64 tsMs = tsStr.toLongLong(&ok);

        if (user.isEmpty() || body.isEmpty()) {
            qDebug() << "Invalid MSG content";
            return;
        }

        const bool outgoing = (user == m_displayName);

        // Do not render incoming messages from another chat in the current chatPane
        if (!outgoing && (m_activePeer.isEmpty() || user != m_activePeer)) {
            showStatus(QString("New message from %1").arg(user));
            showMessageNotification(user, body);
            playMessageSound();
            m_tcpSocket->sendMessage("CHAT_LIST\n");
            return;
        }

        appendChatBubble(user, body, outgoing, ok ? tsMs : -1, messageId, false);

        if (!outgoing && user == m_activePeer) {
            playMessageSound();
            m_tcpSocket->sendMessage(QString("MARK_READ:%1\n").arg(m_activePeer));
        }

        m_tcpSocket->sendMessage("CHAT_LIST\n");
        return;
    }

    if (message.startsWith("PRESENCE:")) {
        if (m_isAuthenticated) {
            if (m_searchOpen) {
                const QString packet = QString("SEARCH:%1\n").arg(m_lastSearchQuery);
                m_tcpSocket->sendMessage(packet);
            } else {
                m_tcpSocket->sendMessage("CHAT_LIST\n");
            }
        }
        return;
    }

    if (message.startsWith("MSG_READ:")) {
    const QString messageId = message.mid(QString("MSG_READ:").size()).trimmed();
    if (messageId.isEmpty()) {
        return;
    }

    // persist read state for future/history rendering
    m_readMessageIds.insert(messageId);

    for (int i = 0; i < ui->lstChat->count(); ++i) {
        QListWidgetItem *it = ui->lstChat->item(i);
        if (!it) {
            continue;
        }

        if (!it->data(Qt::UserRole + 4).toBool()) {
            continue;
        }

        if (it->data(Qt::UserRole).toString() != messageId) {
            continue;
        }

        QWidget *rowWidget = ui->lstChat->itemWidget(it);
        if (rowWidget) {
            QLabel *statusLabel = rowWidget->findChild<QLabel *>("msgStatusLabel");
            if (statusLabel) {
                statusLabel->setVisible(true);
                statusLabel->setText("✓✓");
            }
            it->setSizeHint(rowWidget->sizeHint());
        }
        break;
    }

    return;
}

    if (message.startsWith("MSG_EDIT:")) {
        // MSG_EDIT:<messageId>:<newBody>
        const int firstColon = message.indexOf(':');
        const int secondColon = message.indexOf(':', firstColon + 1);
        // check for empty spaces in the response
        if (firstColon == -1 || secondColon == -1) {
            qDebug() << "Invalid MSG_EDIT format:" << message;
            return;
        }
        // extract variable
        const QString messageId = message.mid(firstColon + 1, secondColon - firstColon - 1).trimmed();
        const QString newBody = message.mid(secondColon + 1).trimmed();

        // checl for empty values
        if (messageId.isEmpty() || newBody.isEmpty()) {
            qDebug() << "Invalid MSG_EDIT content:" << message;
            return;
        }
        // get the QListWidgetItem at index i
        for (int i = 0; i < ui->lstChat->count(); ++i) {
            QListWidgetItem *it = ui->lstChat->item(i);

            // skip null entries
            if (!it) {
                continue;
            }

            // each chat row stores its message id in Qt::UserRole.
            // skip rows that do not match the edited message id.
            if (it->data(Qt::UserRole).toString() != messageId) {
                continue;
            }

            // update cached message body in item metadata
            it->setData(Qt::UserRole + 1, newBody);

            // mark this item as edited
            it->setData(Qt::UserRole + 5, true);

            // get the custom widget used to render this list item
            QWidget *rowWidget = ui->lstChat->itemWidget(it);
            if (rowWidget) {
                // find the label that displays message text and update it
                QLabel *msgLabel = rowWidget->findChild<QLabel *>("msgBodyLabel");
                if (msgLabel) {
                    msgLabel->setText(newBody);
                }

                // find the "edited" label and show it
                QLabel *editedLabel = rowWidget->findChild<QLabel *>("msgEditedLabel");
                if (editedLabel) {
                    editedLabel->setVisible(true);
                }

                // resiee the text length
                it->setSizeHint(rowWidget->sizeHint());
            }

            // stop after updating the first matching message
            break;
        }

        showStatus("Message edited");
        m_tcpSocket->sendMessage("CHAT_LIST\n");

        return;
    }

    if (message.startsWith("MSG_DELETE:")) {
        // MSG_DELETE:<messageId>
        const int firstColon = message.indexOf(':');
        if (firstColon == -1) {
            qDebug() << "Invalid MSG_DELETE format:" << message;
            return;
        }

        const QString messageId = message.mid(firstColon + 1).trimmed();
        if (messageId.isEmpty()) {
            qDebug() << "Invalid MSG_DELETE content:" << message;
            return;
        }

        for (int i = 0; i < ui->lstChat->count(); ++i) {
            QListWidgetItem *it = ui->lstChat->item(i);
            if (!it) {
                continue;
            }

            if (it->data(Qt::UserRole).toString() == messageId) {
                delete ui->lstChat->takeItem(i);
                break;
            }
        }

        showStatus("Message deleted");
        m_tcpSocket->sendMessage("CHAT_LIST\n");
        return;
    }

    if (message.startsWith("HISTORY_BEGIN:")) {
        const QString payload = message.mid(QString("HISTORY_BEGIN:").size());
        const QStringList parts = payload.split(':');

        const QString peer = parts.value(0).trimmed();
        const bool hasMore = (parts.value(1).trimmed() == "1");

        if (peer != m_activePeer) {
            return;
        }

        m_hasMoreHistory = hasMore;
        m_historyInsertedCount = 0;

        // First page after ui->lstChat->clear() => false (append)
        // Next pages while list already has content => true (prepend)
        m_prependHistoryBatch = (ui->lstChat->count() > 0);

        QScrollBar *bar = ui->lstChat->verticalScrollBar();
        if (bar) {
            m_historyPrevScrollValue = bar->value();
            m_historyPrevScrollMax = bar->maximum();
        }
        return;
    }

    if (message.startsWith("HMSG:")) {
    const int firstColon = message.indexOf(':');
    const int secondColon = message.indexOf(':', firstColon + 1);
    const int thirdColon = message.indexOf(':', secondColon + 1);
    const int fourthColon = message.indexOf(':', thirdColon + 1);
    const int fifthColon = message.indexOf(':', fourthColon + 1);

    if (firstColon == -1 || secondColon == -1 || thirdColon == -1 || fourthColon == -1) {
        qDebug() << "Invalid HMSG format:" << message;
        return;
    }

    const QString user = message.mid(firstColon + 1, secondColon - firstColon - 1).trimmed();
    const QString tsStr = message.mid(secondColon + 1, thirdColon - secondColon - 1).trimmed();
    const QString messageId = message.mid(thirdColon + 1, fourthColon - thirdColon - 1).trimmed();

    QString body;
    bool isReadFromHistory = false;

    // backward-compatible:
    // old: HMSG:<sender>:<tsMs>:<id>:<body>
    // new: HMSG:<sender>:<tsMs>:<id>:<isRead>:<body>
    if (fifthColon == -1) {
        body = message.mid(fourthColon + 1).trimmed();
    } else {
        const QString isReadStr = message.mid(fourthColon + 1, fifthColon - fourthColon - 1).trimmed();
        isReadFromHistory = (isReadStr == "1");
        body = message.mid(fifthColon + 1).trimmed();
    }

    bool okTs = false;
    const qint64 tsMs = tsStr.toLongLong(&okTs);

    bool okId = false;
    const qint64 msgId = messageId.toLongLong(&okId);

    if (user.isEmpty() || body.isEmpty()) {
        return;
    }

    const bool outgoing = (user == m_displayName);

    // strict persistence: restore read tick state from DB history
    if (outgoing && isReadFromHistory && !messageId.isEmpty()) {
        m_readMessageIds.insert(messageId);
    }

    appendChatBubble(user, body, outgoing, okTs ? tsMs : -1, messageId, false, m_prependHistoryBatch);

    if (okId && (m_oldestLoadedMessageId == 0 || msgId < m_oldestLoadedMessageId)) {
        m_oldestLoadedMessageId = msgId;
    }

    ++m_historyInsertedCount;
    return;
}
    if (message.startsWith("HISTORY_END:")) {
        const QString peer = message.mid(QString("HISTORY_END:").size()).trimmed();
        if (peer != m_activePeer) {
            return;
        }

        QScrollBar *bar = ui->lstChat->verticalScrollBar();
        if (bar && m_prependHistoryBatch && m_historyInsertedCount > 0) {
            const int delta = bar->maximum() - m_historyPrevScrollMax;
            bar->setValue(m_historyPrevScrollValue + delta);
        }

        // mark currently opened chat as read
        if (m_isAuthenticated && !m_activePeer.isEmpty()) {
            m_tcpSocket->sendMessage(QString("MARK_READ:%1\n").arg(m_activePeer));
        }

        m_loadingHistory = false;
        m_prependHistoryBatch = false;
        return;
    }

    if (message.startsWith("CHAT_LIST_EMPTY")) {

        // check if search field is open
        if (m_searchOpen) {
            return;
        }

        // clear chat list + display message
        ui->lstUsers->clear();
        QListWidgetItem *item = new QListWidgetItem("No chats yet");
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        ui->lstUsers->addItem(item);
        return;
    }

    if (message.startsWith("CHAT_LIST_RESULT:")) {
    if (m_searchOpen) {
        return;
    }

    const QString payload = message.mid(QString("CHAT_LIST_RESULT:").size());
    const QStringList entries = payload.split(';', Qt::SkipEmptyParts);

    ui->lstUsers->clear();

    for (const QString &entry : entries) {
        const QStringList parts = entry.split('|');
        const QString username = parts.value(0).trimmed();
        const bool isOnline = (parts.value(1).trimmed() == "1");
        const int unread = parts.value(2).trimmed().toInt();

        bool okTs = false;
        const qint64 tsMs = parts.value(3).trimmed().toLongLong(&okTs);
        const QString previewRaw = parts.value(4).trimmed();

        if (username.isEmpty()) {
            continue;
        }

        QString timeText;
        if (okTs && tsMs > 0) {
            const QDateTime dt = QDateTime::fromMSecsSinceEpoch(tsMs, Qt::UTC).toLocalTime();
            if (dt.isValid()) {
                if (dt.date() == QDate::currentDate()) {
                    timeText = dt.toString("HH:mm");
                } else if (dt.date().year() == QDate::currentDate().year()) {
                    timeText = dt.toString("dd MMM");
                } else {
                    timeText = dt.toString("dd.MM.yy");
                }
            }
        }

         QString preview = previewRaw.isEmpty() ? "No messages yet" : previewRaw;

        QListWidgetItem *item = new QListWidgetItem(ui->lstUsers);
        item->setData(Qt::UserRole, username);
        item->setSizeHint(QSize(0, 64));

        QWidget *row = new QWidget(ui->lstUsers);
        row->setAttribute(Qt::WA_TransparentForMouseEvents, true);

        QVBoxLayout *root = new QVBoxLayout(row);
        root->setContentsMargins(10, 6, 10, 6);
        root->setSpacing(3);

        QHBoxLayout *top = new QHBoxLayout();
        top->setContentsMargins(0, 0, 0, 0);
        top->setSpacing(6);

        const bool isLight = (m_themeName == "Light");
        const QString listNameColor = isLight ? "#111827" : "#e6ebf5";
        const QString listMetaColor = isLight ? "#4b5563" : "#9fb0c3";
        const QString listTimeColor = isLight ? "#4b5563" : "#7f91a4";

        QLabel *nameLabel = new QLabel(username, row);
        nameLabel->setObjectName("userNameLabel");
        nameLabel->setStyleSheet(QString("color: %1; font-weight: 700;").arg(listNameColor));

        QLabel *onlineLabel = new QLabel(isOnline ? "●" : "", row);
        onlineLabel->setObjectName("onlineDotLabel");
        onlineLabel->setStyleSheet(QString("color: %1; font-size: 10px;").arg(m_accentColor));

        QLabel *timeLabel = new QLabel(timeText, row);
        timeLabel->setObjectName("timeLabel");
        timeLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(listTimeColor));

        QLabel *previewLabel = new QLabel(preview, row);
        previewLabel->setObjectName("previewLabel");
        previewLabel->setStyleSheet(QString("color: %1; font-size: 12px;").arg(listMetaColor));

        top->addWidget(nameLabel);
        top->addWidget(onlineLabel);
        top->addStretch();
        top->addWidget(timeLabel);

        QHBoxLayout *bottom = new QHBoxLayout();
        bottom->setContentsMargins(0, 0, 0, 0);
        bottom->setSpacing(6);

        QLabel *badgeLabel = new QLabel(row);
        badgeLabel->setObjectName("badgeLabel"); 
        if (unread > 0) {
            badgeLabel->setText(QString::number(unread));
            badgeLabel->setAlignment(Qt::AlignCenter);
            badgeLabel->setMinimumWidth(20);
            badgeLabel->setStyleSheet(
                QString("background-color: %1;"
                        "color: white;"
                        "border-radius: 10px;"
                        "padding: 1px 6px;"
                        "font-size: 11px;"
                        "font-weight: 700;").arg(m_accentColor));
            badgeLabel->setVisible(true);
        } else {
            badgeLabel->clear();
            badgeLabel->setMinimumWidth(0);
            badgeLabel->setStyleSheet(QString());
            badgeLabel->setVisible(false);
        }

        bottom->addWidget(previewLabel, 1);
        if (unread > 0) {
            bottom->addWidget(badgeLabel, 0, Qt::AlignRight);
        }

        root->addLayout(top);
        root->addLayout(bottom);

        ui->lstUsers->addItem(item);
        ui->lstUsers->setItemWidget(item, row);
    }

    if (ui->lstUsers->count() == 0) {
        QListWidgetItem *item = new QListWidgetItem("No chats yet");
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        ui->lstUsers->addItem(item);
    }

    return;
}
}

void MainWindow::onSocketError(const QString &errorMsg)
{
    showStatus("Error: " + errorMsg);
}

void MainWindow::on_btnSend_clicked()
{
    const QString msg = ui->lnMessage->text().trimmed();
    if (msg.isEmpty()) {
        return;
    }

    if (!m_isAuthenticated) {
        QMessageBox::warning(this, "Not authenticated", "Please authenticate first.");
        return;
    }

    if (m_activePeer.isEmpty()) {
        QMessageBox::information(this, "No session", "Press Search and pick an online user first.");
        return;
    }

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();

    const QString packet = QString("MSG:%1:%2\n").arg(m_activePeer, msg);
    m_tcpSocket->sendMessage(packet);
    ui->lnMessage->clear();
    playSendSound();
}

void MainWindow::on_btnSearch_clicked()
{
    if (!m_isAuthenticated) {
        QMessageBox::warning(this, "Not authenticated", "Please authenticate first.");
        return;
    }

    if (!m_searchOpen) {
        m_searchOpen = true;
        m_lastSearchQuery = ui->lnSearch->text().trimmed();
        ui->label_2->hide();
        ui->lnSearch->hide();
        ui->btnSearch->setText("Close");

        ui->lstUsers->clear();
        const QString packet = QString("SEARCH:%1\n").arg(m_lastSearchQuery); // online list mode
        m_tcpSocket->sendMessage(packet);
        return;

    }

    closeSearchPanel(); // restore chat list mode
}

void MainWindow::on_lstUsers_itemClicked(QListWidgetItem *item)
{
    if (!item || !m_isAuthenticated) {
        return;
    }

    const QString selectedUser = item->data(Qt::UserRole).toString().trimmed();
    if (selectedUser.isEmpty()) {
        return;
    }

    // Proactively clear the badge on click
    QWidget *rowWidget = ui->lstUsers->itemWidget(item);
    if (rowWidget) {
        QLabel *badgeLabel = rowWidget->findChild<QLabel *>("badgeLabel");
        if (badgeLabel) {
            badgeLabel->clear();
            badgeLabel->setMinimumWidth(0);
            badgeLabel->setStyleSheet(QString());
            badgeLabel->setVisible(false);
        }
    }

    // Search mode: create new session
    if (m_searchOpen) {
        const QString req = QString("SESSION_CREATE:%1\n").arg(selectedUser);
        m_tcpSocket->sendMessage(req);
        closeSearchPanel();
        return;
    }

    // Normal mode: open existing chat locally (no invite/session-create)
    m_activePeer = selectedUser;
    ui->lblActiveSession->setText("Active session: " + m_activePeer);
    ui->lstChat->clear();

    m_oldestLoadedMessageId = 0;
    m_loadingHistory = false;
    m_hasMoreHistory = true;
    m_historyInsertedCount = 0;

    requestHistoryPage(0);
}

void MainWindow::on_btnMenu_clicked()
{
    if (!m_mainMenu || !ui->btnMenu) {
        return;
    }

    const QPoint pos = ui->btnMenu->mapToGlobal(QPoint(0, ui->btnMenu->height()));
    m_mainMenu->exec(pos);
}

void MainWindow::onMenuSettingsTriggered()
{
    SettingsDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString settingsPath = QCoreApplication::applicationDirPath() + "/ui_config.ini";
    QSettings settings(settingsPath, QSettings::IniFormat);

    settings.setValue("ui/theme", dialog.selectedTheme());
    settings.setValue("ui/accent", dialog.selectedAccent());
    settings.setValue("ui/fontSize", dialog.selectedFontSize());
    settings.setValue("ui/showTimestamps", dialog.showTimestamps());
    settings.setValue("ui/compactChatList", dialog.compactChatList());
    settings.setValue("ui/notifySound", dialog.notifySound());
    settings.setValue("ui/notifyPreview", dialog.notifyPreview());
    settings.setValue("ui/notifyDnd", dialog.notifyDnd());
    settings.sync();
    qDebug() << "Settings keys:" << settings.allKeys();
    qDebug() << "Settings file:" << settings.fileName();

    applyAppearanceSettings(dialog.selectedTheme(),
                            dialog.selectedAccent(),
                            dialog.selectedFontSize(),
                            dialog.showTimestamps(),
                            dialog.compactChatList());

    applyNotificationSettings(dialog.notifySound(),
                              dialog.notifyPreview(),
                              dialog.notifyDnd());
}
void MainWindow::onMenuCreateGroupTriggered()
{
    QMessageBox::information(this, "Create Group Chat", "Group chat creation is not implemented yet.");
}

void MainWindow::on_lstChat_customContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem *item = ui->lstChat->itemAt(pos);
    if (!item) {
        return;
    }

    const bool outgoing = item->data(Qt::UserRole + 4).toBool();
    if (!outgoing) {
        return; // only own messages editable/deletable
    }

    QMenu menu(this);
    QAction *editAction = menu.addAction("Edit");
    QAction *deleteAction = menu.addAction("Delete");

    QAction *selected = menu.exec(ui->lstChat->viewport()->mapToGlobal(pos));
    if (!selected) {
        return;
    }

    if (selected == editAction) {
        const QString oldBody = item->data(Qt::UserRole + 1).toString();

        bool ok = false;
        const QString newBody = QInputDialog::getText(
            this,
            "Edit message",
            "Message:",
            QLineEdit::Normal,
            oldBody,
            &ok).trimmed();

        if (!ok || newBody.isEmpty() || newBody == oldBody) {
            return;
        }

        item->setData(Qt::UserRole + 1, newBody);

        QWidget *rowWidget = ui->lstChat->itemWidget(item);
        if (rowWidget) {
            QLabel *msgLabel = rowWidget->findChild<QLabel*>("msgBodyLabel");
            if (msgLabel) {
                msgLabel->setText(newBody);
                item->setSizeHint(rowWidget->sizeHint());
            }
        }

        const QString id = item->data(Qt::UserRole).toString();
        if (!id.isEmpty()) {
             m_tcpSocket->sendMessage(QString("MSG_EDIT:%1:%2\n").arg(id, newBody));
         }

        showStatus("Message edited");
    } else if (selected == deleteAction) {
        const QString id = item->data(Qt::UserRole).toString();
        if (!id.isEmpty()) {
            m_tcpSocket->sendMessage(QString("MSG_DELETE:%1\n").arg(id));
        }

        delete ui->lstChat->takeItem(ui->lstChat->row(item));
        showStatus("Message deleted");
    }
}

void MainWindow::requestHistoryPage(qint64 beforeId)
{
    // Guard conditions:
    // - must be authenticated
    // - must have active peer
    // - don't send while a history request is in-flight
    // - stop if server already said no more pages
    if (!m_isAuthenticated || m_activePeer.isEmpty() || m_loadingHistory || !m_hasMoreHistory) {
        return;
    }

    m_loadingHistory = true;
    const int pageSize = 30;

    // Cursor-style request:
    // beforeId = 0 => latest page
    // beforeId > 0 => older than this id
    m_tcpSocket->sendMessage(QString("HISTORY:%1:%2:%3\n")
                                 .arg(m_activePeer,
                                      QString::number(beforeId),
                                      QString::number(pageSize)));
}

void MainWindow::onChatScrollValueChanged(int value)
{
    QScrollBar *bar = ui->lstChat->verticalScrollBar();
    if (!bar) {
        return;
    }

    // When user reaches top, load older page using current oldest id as cursor.
    if (value == bar->minimum()) {
        requestHistoryPage(m_oldestLoadedMessageId);
    }
}

void MainWindow::applyAppearanceSettingsFromSettings()
{
    const QString settingsPath = QCoreApplication::applicationDirPath() + "/ui_config.ini";
    QSettings settings(settingsPath, QSettings::IniFormat);

    const bool settingsEmpty = (QFileInfo(settingsPath).size() == 0) || settings.allKeys().isEmpty();
    if (settingsEmpty) {
        settings.setValue("ui/theme", "Dark");
        settings.setValue("ui/accent", "Blue");
        settings.setValue("ui/fontSize", "Medium");
        settings.setValue("ui/showTimestamps", true);
        settings.setValue("ui/compactChatList", false);
        settings.setValue("ui/notifySound", true);
        settings.setValue("ui/notifyPreview", true);
        settings.setValue("ui/notifyDnd", false);
        settings.sync();
    }

    applyAppearanceSettings(
        settings.value("ui/theme", "Dark").toString(),
        settings.value("ui/accent", "Blue").toString(),
        settings.value("ui/fontSize", "Medium").toString(),
        settings.value("ui/showTimestamps", true).toBool(),
        settings.value("ui/compactChatList", false).toBool()
    );

    applyNotificationSettings(
        settings.value("ui/notifySound", true).toBool(),
        settings.value("ui/notifyPreview", true).toBool(),
        settings.value("ui/notifyDnd", false).toBool()
    );
}

void MainWindow::applyAppearanceSettings(const QString &theme, const QString &accent, const QString &fontSize, bool showTimestamps, bool compactChatList)
{
    // get parameters values
    m_themeName = theme;
    m_accentName = accent;
    m_fontSizeName = fontSize;
    m_showTimestamps = showTimestamps;

    // resolve fontsize and colours
    m_accentColor = resolveAccentColor(accent);
    m_uiFontSize = resolveFontSize(fontSize);

    // apply theme ui
    applyThemeStyles();

    // set the font
    QFont baseFont = font();
    baseFont.setPointSize(m_uiFontSize);
    ui->lstUsers->setFont(baseFont);
    ui->lstChat->setFont(baseFont);
    ui->lnSearch->setFont(baseFont);
    ui->lnMessage->setFont(baseFont);
    ui->lblActiveSession->setFont(baseFont);
}

QString MainWindow::resolveAccentColor(const QString &accent) const
{
    if (accent == "Slate") return "#607d8b";
    if (accent == "Emerald") return "#2ecc71";
    if (accent == "Purple") return "#7e57c2";
    return "#2f6ea5"; // Blue
}

int MainWindow::resolveFontSize(const QString &fontSize) const
{
    if (fontSize == "Small") return 11;
    if (fontSize == "Large") return 15;
    return 13; // Medium
}

void MainWindow::applyThemeStyles()
{
    const bool isLight = (m_themeName == "Light");
    const QString bg = isLight ? "#eef2f6" : "#17212b";
    const QString panel = isLight ? "#f6f8fb" : "#1f2c3a";
    const QString border = isLight ? "#d2d9e3" : "#34495e";
    const QString text = isLight ? "#1c2733" : "#e6ebf5";
    const QString hover = isLight ? "#e4e9f0" : "#27384a";
    const QString chatBg = isLight ? "#f7f9fc" : "#0f1822";
    const QString chatBorder = isLight ? "#dfe6ee" : "#223345";

    const QString textColor = isLight ? "#1c2733" : "#eaf2ff";
    const QString metaColor = isLight ? "#64748b" : "#aab7c4";
    const QString incomingNameColor = isLight ? "#b03a3a" : "#ff9b9b";

    const QString outgoingBubble = isLight ? "#d7ebff" : "#2b5278";
    const QString outgoingBorder = m_accentColor;
    const QString incomingBubble = isLight ? "#f0f4f8" : "#182533";
    const QString incomingBorder = isLight ? "#d0d9e3" : "#2c3f54";

    const QColor accentColor(m_accentColor);
    const QString accentLight = accentColor.lighter(isLight ? 170 : 150).name();
    const QString accentDark = accentColor.darker(isLight ? 140 : 130).name();

    const QString outgoingNameColor = isLight ? accentDark : accentLight;

    if(isLight) {


        ui->btnMenu->setStyleSheet(QString(R"(
            QPushButton {
                background-color: %1;
                color: #000000;
                border: 1px solid %1;
                border-radius: 9px;
                padding: 6px 14px;
                font-weight: 600;
            }
            QPushButton:hover { background-color: %2; }
            QPushButton:pressed { background-color: %3; }
        )").arg(accentLight,
                accentLight,
                accentLight));
    }else {

        ui->btnMenu->setStyleSheet(QString(R"(
            QPushButton {
                background-color: %1;
                color: #ffffff;
                border: 1px solid %1;
                border-radius: 9px;
                padding: 6px 14px;
                font-weight: 600;
            }
            QPushButton:hover { background-color: %2; }
            QPushButton:pressed { background-color: %3; }
        )").arg(accentDark,accentDark,accentDark));
    }
    if (isLight){
        ui->btnSearch->setStyleSheet(QString(R"(
            QPushButton {
                background-color: %1;
                color: #000000;
                border: 1px solid %2;
                border-radius: 9px;
                padding: 6px 14px;
                font-weight: 600;
            }
            QPushButton:hover { background-color: %3; }
            QPushButton:pressed { background-color: %4; }
        )").arg(accentLight,accentLight,accentLight,accentLight));
    } else {
        ui->btnSearch->setStyleSheet(QString(R"(
            QPushButton {
                background-color: %1;
                color: #ffffff;
                border: 1px solid %2;
                border-radius: 9px;
                padding: 6px 14px;
                font-weight: 600;
            }
            QPushButton:hover { background-color: %3; }
            QPushButton:pressed { background-color: %4; }
        )").arg(accentDark,accentDark,accentDark,accentDark));
    }

    ui->lnSearch->setStyleSheet(QString(R"(
        QLineEdit {
            background-color: %1;
            color: %2;
            border: 1px solid %3;
            border-radius: 9px;
            padding: 6px 10px;
            selection-background-color: %4;
        }
        QLineEdit::placeholder { color: %5; }
    )").arg(panel, text, border, m_accentColor, metaColor));

    ui->lnMessage->setStyleSheet(QString(R"(
        QLineEdit {
            background-color: %1;
            color: %2;
            border: 1px solid %3;
            border-radius: 9px;
            padding: 6px 10px;
            selection-background-color: %4;
        }
        QLineEdit::placeholder { color: %5; }
    )").arg(panel, text, border, m_accentColor, metaColor));

    if(isLight){
        ui->btnSend->setStyleSheet(QString(R"(
            QPushButton {
            background-color: %1;
            color: #000000;
            border: 1px solid %2;
            border-radius: 9px;
            padding: 6px 14px;
            font-weight: 700;
            }
            QPushButton:hover { background-color: %3; }
            QPushButton:pressed { background-color: %4; }
        )").arg(accentLight,accentLight,accentLight,accentLight));
    } else {
        ui->btnSend->setStyleSheet(QString(R"(
            QPushButton {
            background-color: %1;
            color: #ffffff;
            border: 1px solid %2;
            border-radius: 9px;
            padding: 6px 14px;
            font-weight: 700;
            }
            QPushButton:hover { background-color: %3; }
            QPushButton:pressed { background-color: %4; }
        )").arg(accentDark,accentDark,accentDark,accentDark));
    }
    ui->lstUsers->setStyleSheet(QString(R"(
        QListWidget {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 10px;
            outline: none;
            color: %3;
            padding: 6px;
        }
        QListWidget::item:hover { background-color: %4; }
        QListWidget::item:selected { background-color: %5; color: #ffffff; }
        QListWidget::item:selected:active { background-color: %5; }
    )").arg(bg, border, text, hover, (isLight) ? accentLight : accentDark));

    ui->groupBox->setStyleSheet(QString(R"(
        QGroupBox {
            border: 1px solid %1;
            border-radius: 12px;
            margin-top: 10px;
            background-color: %2;
            color: %3;
            font-weight: 600;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 6px;
            color: %4;
        }
    )").arg(border, bg, text, isLight ? "#64748b" : "#9fb0c3"));

    ui->lstChat->setStyleSheet(QString(R"(
        QListWidget {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 10px;
            outline: none;
            padding: 6px;
        }
        QListWidget::item { border: none; margin: 2px 0px; }
    )").arg(chatBg, chatBorder));

    // Message bubbles (used by appendChatBubble)
    const QString bubbleStyle = QString(
        "QFrame#outgoingBubble {"
        "background-color: %1;"
        "border: none;"
        "border-radius: 14px;"
        "} "
        "QFrame#incomingBubble {"
        "background-color: %2;"
        "border: 1px solid %3;"
        "border-radius: 14px;"
        "}"
    ).arg(isLight ? accentLight : accentDark, incomingBubble, incomingBorder);

    ui->lstChat->setStyleSheet(ui->lstChat->styleSheet() + bubbleStyle);

    // Restyle existing chat bubbles + text
    for (int i = 0; i < ui->lstChat->count(); ++i) {
        QListWidgetItem *item = ui->lstChat->item(i);
        if (!item) {
            continue;
        }

        QWidget *rowWidget = ui->lstChat->itemWidget(item);
        if (!rowWidget) {
            continue;
        }

        QFrame *outgoingBubbleFrame = rowWidget->findChild<QFrame *>("outgoingBubble");
        if (outgoingBubbleFrame) {
            outgoingBubbleFrame->setStyleSheet(QString(
                "QFrame#outgoingBubble {"
                "background-color: %1;"
                "border: none;"
                "border-radius: 14px;"
                "}").arg(isLight ? accentLight : accentDark));
        }

        QLabel *timeLabel = rowWidget->findChild<QLabel *>("msgTimeLabel");
        if (timeLabel) {
            const bool outgoing = item->data(Qt::UserRole + 4).toBool();
            timeLabel->setStyleSheet(QString("color: %1; font-size: 11px;")
                                         .arg(outgoing ? outgoingNameColor : metaColor));

            const QString accentUi = isLight ? accentLight : m_accentColor;
            timeLabel->setStyleSheet(QString("color: %1; font-size: 11px;")
                                       .arg(outgoing ? outgoingNameColor : metaColor));
        }

        QLabel *statusLabel = rowWidget->findChild<QLabel *>("msgStatusLabel");
        if (statusLabel) {
            statusLabel->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 700;")
                                   .arg(outgoingNameColor));
        }
    }

    ui->lblActiveSession->setStyleSheet(QString("color: %1; font-weight: 600;").arg(accentDark));
    ui->label_2->setStyleSheet(QString("color: %1; font-weight: 600;").arg(accentLight));

    const QString listNameColor = isLight ? "#111827" : "#e6ebf5";
    const QString listMetaColor = isLight ? "#4b5563" : "#9fb0c3";
    const QString listTimeColor = isLight ? "#4b5563" : "#7f91a4";

    for (int i = 0; i < ui->lstUsers->count(); ++i) {
        QListWidgetItem *item = ui->lstUsers->item(i);
        if (!item) {
            continue;
        }

        QWidget *rowWidget = ui->lstUsers->itemWidget(item);
        if (!rowWidget) {
            continue;
        }

        QLabel *nameLabel = rowWidget->findChild<QLabel *>("userNameLabel");
        if (nameLabel) {
            nameLabel->setStyleSheet(QString("color: %1; font-weight: 700;").arg(listNameColor));
        }

        QLabel *subtitleLabel = rowWidget->findChild<QLabel *>("subtitleLabel");
        if (subtitleLabel) {
            subtitleLabel->setStyleSheet(QString("color: %1; font-size: 12px;").arg(listMetaColor));
        }

        QLabel *previewLabel = rowWidget->findChild<QLabel *>("previewLabel");
        if (previewLabel) {
            previewLabel->setStyleSheet(QString("color: %1; font-size: 12px;").arg(listMetaColor));
        }

        QLabel *timeLabel = rowWidget->findChild<QLabel *>("timeLabel");
        if (timeLabel) {
            timeLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(listTimeColor));
        }

        QLabel *onlineLabel = rowWidget->findChild<QLabel *>("onlineDotLabel");
        if (onlineLabel) {
            onlineLabel->setStyleSheet(QString("color: %1; font-size: 10px;").arg(m_accentColor));
        }
    }
}

void MainWindow::applyNotificationSettings(bool notifySound, bool notifyPreview, bool notifyDnd)
{
    // retrieve parameters
    m_notifySound = notifySound;
    m_notifyPreview = notifyPreview;
    m_notifyDnd = notifyDnd;

    ensureTrayIcon();

    // enables the sound if the status is false
    if (!m_messageSound) {
        m_messageSound = new QSoundEffect(this);
        m_messageSound->setLoopCount(1);
        m_messageSound->setVolume(0.6f);
    }

    // locate the custom notification sound
    const QString soundPath = QCoreApplication::applicationDirPath() + "/notify_fixed.wav";
    if (QFileInfo::exists(soundPath)) {
        m_messageSound->setSource(QUrl::fromLocalFile(soundPath));
        qDebug() << "Notification sound:" << soundPath;
    } else {
        qWarning() << "Notification sound missing:" << soundPath;
        m_messageSound->setSource(QUrl());
    }


    m_sendSound = new QSoundEffect(this);
    m_sendSound->setLoopCount(1);
    m_sendSound->setVolume(0.6f);

    const QString sendPath = QCoreApplication::applicationDirPath() + "/send_fixed.wav";
    if (QFileInfo::exists(sendPath)) {
        m_sendSound->setSource(QUrl::fromLocalFile(sendPath));
        qDebug() << "Send sound:" << sendPath;
    } else {
        qWarning() << "Send sound missing:" << sendPath;
        m_sendSound->setSource(QUrl());
    }

}

void MainWindow::showMessageNotification(const QString &fromUser, const QString &body)
{
    if (m_notifyDnd) {
        return;
    }

    ensureTrayIcon();
    if (!m_trayIcon || !m_trayIcon->isVisible()) {
        return;
    }

    const QString title = QString("New message from %1").arg(fromUser);
    const QString text = m_notifyPreview ? body : "Message preview hidden";
    m_trayIcon->showMessage(title, text, QSystemTrayIcon::Information, 4000);
}

void MainWindow::playMessageSound()
{
    if (m_notifyDnd || !m_notifySound) {
        return;
    }

    if (!m_messageSound || !m_messageSound->source().isValid()) {
        return;
    }

    if (m_messageSound->status() == QSoundEffect::Ready) {
        m_messageSound->play();
    }
}

void MainWindow::ensureTrayIcon()
{
    if (m_trayIcon || !QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }

    m_trayIcon = new QSystemTrayIcon(this);
    QIcon icon = windowIcon();
    if (icon.isNull()) {
        icon = QIcon::fromTheme("dialog-information");
    }
    m_trayIcon->setIcon(icon);
    m_trayIcon->setVisible(true);
}

void MainWindow::playSendSound()
{
    if (m_sendSound->status() == QSoundEffect::Ready) {
        m_sendSound->play();
    }
}
