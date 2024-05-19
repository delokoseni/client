#include "Chat.h"
#include <QSqlQueryModel>

Chat::Chat(const QString &host, int port, QWidget *parent, int _chatId)
    : QWidget(parent), m_host(host), m_port(port), chatId(_chatId), m_socket(new QTcpSocket(this)) {
    connectToServer();
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
    layout->addWidget(backButton);
    layout->addWidget(messagesHistoryWidget);
    layout->addWidget(messageInputWidget);
    layout->addWidget(sendMessageButton);

    setLayout(layout);
}

void Chat::connectSignalsAndSlots() {
    connect(sendMessageButton, &QPushButton::clicked, this, &Chat::sendMessage);
    connect(backButton, &QPushButton::clicked, this, &Chat::onBackButtonClicked);
}

void Chat::loadMessages() { //Переписать
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
    qDebug() << "Нажатие кнопки\n";
    QString messageText = messageInputWidget->text();
    if (!messageText.isEmpty()) {
        if (m_socket->isOpen()) {
            QTextStream stream(m_socket);
            userId = 1; // Изменить
            stream << "send_message:" << chatId << ":" << userId << ":" << messageText << "\n";
            stream.flush();
            QString messageHtml = QString("<div style='text-align: right;'>%1</div>").arg(messageText);
            messagesHistoryWidget->append(messageText);
            messageInputWidget->clear(); // Очищаем messageInputWidget
        }
        else {
            // Обработать ошибку выполнения запроса
        }
    }
}

void Chat::onBackButtonClicked() {
    emit backToChatsList();
    this->deleteLater();
}

void Chat::connectToServer()
{
    connect(m_socket, &QTcpSocket::connected, this, &Chat::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &Chat::onReadyRead); // Устанавливаем соединение сигнала с слотом
    m_socket->connectToHost(m_host, m_port);
}

void Chat::onConnected()
{
    qDebug() << "Connected to server.";
}

void Chat::onReadyRead()
{
    QTextStream stream(m_socket);
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed(); // Читаем строку за строкой

        // Проверяем успешность отправки сообщения
        if (line.startsWith("send_message:success")) {
            // Сообщение успешно доставлено и можно, например, очистить поле ввода, если это необходимо
            qDebug() << "Message sent successfully.";
        } else if (line.startsWith("send_message:fail")) {
            QString errorMessage = line.section(':', 2); // Получаем сообщение об ошибке
            qDebug() << "Failed to send message:" << errorMessage;
            // Здесь можно уведомить пользователя об ошибке через интерфейс
        }
        // Обработка других команд
    }
}

