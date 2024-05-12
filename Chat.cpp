#include "Chat.h"
#include <QSqlQueryModel>

Chat::Chat(QWidget *parent, int chatId)
    : QWidget(parent), chatId(chatId) {
    setupUi();
    connectSignalsAndSlots();
    loadMessages();
}

Chat::~Chat() {}

void Chat::setupUi() {
    messagesHistoryWidget = new QTextEdit(this);
    messagesHistoryWidget->setReadOnly(true);

    messageInputWidget = new QLineEdit(this);

    sendMessageButton = new QPushButton("Отправить", this);
    backButton = new QPushButton("Назад", this);

    layout = new QVBoxLayout(this);
    layout->addWidget(messagesHistoryWidget);
    layout->addWidget(messageInputWidget);
    layout->addWidget(sendMessageButton);
    layout->addWidget(backButton);

    setLayout(layout);
}

void Chat::connectSignalsAndSlots() {
    connect(sendMessageButton, &QPushButton::clicked, this, &Chat::sendMessage);
    connect(backButton, &QPushButton::clicked, this, &Chat::onBackButtonClicked);
}

void Chat::loadMessages() {
    QSqlQuery query;
    query.prepare("SELECT message_text FROM messages WHERE chat_id = :chatId ORDER BY timestamp_sent ASC");
    query.bindValue(":chatId", chatId);
    if (query.exec()) {
        while (query.next()) {
            QString message = query.value(0).toString();
            messagesHistoryWidget->append(message);
        }
    } else {
        // Обработать ошибку выполнения запроса
    }
}

void Chat::sendMessage() {
    // Реализация отправки сообщений в базу данных
    QString messageText = messageInputWidget->text();
    if (!messageText.isEmpty()) {
        QSqlQuery query;
        query.prepare("INSERT INTO messages (chat_id, user_id, message_text) VALUES (:chatId, :userId, :messageText)");
        query.bindValue(":chatId", chatId);
        // query.bindValue(":userId", userId); // userId должен быть доступен где-то в вашем приложении
        query.bindValue(":messageText", messageText);
        if (query.exec()) {
            messagesHistoryWidget->append(messageText);
            messageInputWidget->clear();
        } else {
            // Обработать ошибку выполнения запроса
        }
    }
}

void Chat::onBackButtonClicked() {
    emit backToChatsList();
    this->deleteLater();
}
