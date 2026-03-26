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

    // keep list visible to preserve splitter layout
    ui->lstUsers->clear();

    // reload existing chats list
    m_tcpSocket->sendMessage("SEARCH:\n");
}

void MainWindow::appendChatBubble(const QString &user, const QString &body, bool outgoing, qint64 timestampMs, const QString &messageId, bool isEdited, bool prepend)
{
    if (user.isEmpty() || body.isEmpty()) {
        return;
    }

    QListWidgetItem *item = new QListWidgetItem;

    item->setData(Qt::UserRole, messageId);
    item->setData(Qt::UserRole + 1, body);
    item->setData(Qt::UserRole + 2, user);
    item->setData(Qt::UserRole + 3, timestampMs);
    item->setData(Qt::UserRole + 4, outgoing);
    item->setData(Qt::UserRole + 5, isEdited);

    QWidget *rowWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(rowWidget);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(2);

    QWidget *headerWidget = new QWidget(rowWidget);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(6);

    QLabel *nameLabel = new QLabel(QString("(%1)").arg(user), headerWidget);
    nameLabel->setStyleSheet(outgoing
                                 ? "color: #4A90E2; font-weight: 800;"
                                 : "color: #FF0000; font-weight: 800;");

    QLabel *timeLabel = new QLabel(headerWidget);
    timeLabel->setStyleSheet("color: #A9A9A9; font-size: 11px; font-weight: 500;");

    if (timestampMs > 0) {
        const QDateTime localDt = QDateTime::fromMSecsSinceEpoch(timestampMs, Qt::UTC).toLocalTime();
        if (localDt.isValid()) {
            timeLabel->setText(localDt.toString("yyyy-MM-dd HH:mm"));
        }
    }

    QLabel *editedLabel = new QLabel(headerWidget);
    editedLabel->setObjectName("msgEditedLabel");
    editedLabel->setStyleSheet("color: #A9A9A9; font-size: 11px; font-style: italic;");
    editedLabel->setText("edited");
    editedLabel->setVisible(isEdited);

    headerLayout->addWidget(nameLabel);
    headerLayout->addWidget(timeLabel);
    headerLayout->addWidget(editedLabel);
    headerLayout->addStretch();

    QLabel *msgLabel = new QLabel(body, rowWidget);
    msgLabel->setObjectName("msgBodyLabel");
    msgLabel->setWordWrap(true);
    msgLabel->setStyleSheet("color: #EAEAEA;");

    layout->addWidget(headerWidget);
    layout->addWidget(msgLabel);

    item->setSizeHint(rowWidget->sizeHint());

    if (prepend) {
        ui->lstChat->insertItem(0, item);
    } else {
        ui->lstChat->addItem(item);
    }

    ui->lstChat->setItemWidget(item, rowWidget);

    if (!prepend) {
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
        m_tcpSocket->sendMessage("SEARCH:\n"); // load existing chats immediately
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

            QString label = username;
            if (status == "active_session") {
                label += " (online chat)";
            } else if (status == "active") {
                label += " (online)";
            } else {
                label += " (chat)";
            }

            QListWidgetItem *item = new QListWidgetItem(label);
            item->setData(Qt::UserRole, username);
            ui->lstUsers->addItem(item);
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
        appendChatBubble(user, body, outgoing, ok ? tsMs : -1, messageId, false);
        return;
    }

    if (message.startsWith("PRESENCE:")) {
        if (m_isAuthenticated) {
            const QString packet = QString("SEARCH:%1\n").arg(m_searchOpen ? m_lastSearchQuery : QString());
            m_tcpSocket->sendMessage(packet);
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

        if (firstColon == -1 || secondColon == -1 || thirdColon == -1 || fourthColon == -1) {
            qDebug() << "Invalid HMSG format:" << message;
            return;
        }

        const QString user = message.mid(firstColon + 1, secondColon - firstColon - 1).trimmed();
        const QString tsStr = message.mid(secondColon + 1, thirdColon - secondColon - 1).trimmed();
        const QString messageId = message.mid(thirdColon + 1, fourthColon - thirdColon - 1).trimmed();
        const QString body = message.mid(fourthColon + 1).trimmed();

        bool okTs = false;
        const qint64 tsMs = tsStr.toLongLong(&okTs);

        bool okId = false;
        const qint64 msgId = messageId.toLongLong(&okId);

        if (user.isEmpty() || body.isEmpty()) {
            return;
        }

        const bool outgoing = (user == m_displayName);

        // IMPORTANT: prepend only for older pages
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

        // Keep viewport stable only when we prepended older items
        if (bar && m_prependHistoryBatch && m_historyInsertedCount > 0) {
            const int delta = bar->maximum() - m_historyPrevScrollMax;
            bar->setValue(m_historyPrevScrollValue + delta);
        }

        m_loadingHistory = false;
        m_prependHistoryBatch = false;
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

    const QString req = QString("SESSION_CREATE:%1\n").arg(selectedUser);
    m_tcpSocket->sendMessage(req);
    closeSearchPanel();
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
    QMessageBox::information(this, "Settings", "Settings dialog is not implemented yet.");
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
