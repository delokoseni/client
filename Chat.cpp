#include "Chat.h"
#include <QSqlQueryModel>

Chat::Chat(const QString &host, int port, QWidget *parent, int _chatId, const QString login)
    : QWidget(parent), login(login), m_host(host), m_port(port), chatId(_chatId), m_socket(new QTcpSocket(this)) {
    connectToServer();
    setupUi();
    connectSignalsAndSlots();
    qDebug() << "Login: " << login << "\n";
    qDebug() << "Login: " << this->login << "\n";
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

void Chat::loadMessages() {
    if (m_socket->isOpen()) {
        QTextStream stream(m_socket);
        stream << "get_messages:" << chatId << "\n";
        stream.flush();
    } else {
        qDebug() << "Socket is not open. Cannot load messages.";
    }
}


void Chat::sendMessage() {
    // Реализация отправки сообщений в базу данных
    qDebug() << "Нажатие кнопки\n";
    QString messageText = messageInputWidget->text();
    if (!messageText.isEmpty()) {
        if (m_socket->isOpen()) {
            QTextStream stream(m_socket);
            stream << "send_message:" << chatId << ":" << userId << ":" << messageText << "\n";
            stream.flush();
            QString messageHtml = QString("<div style='text-align: right;'>%1</div>").arg(messageText);
            messagesHistoryWidget->append(messageHtml); // Используйте messageHtml здесь
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

        if (line.startsWith("send_message:success")) {
            qDebug() << "Message sent successfully.";
        } else if (line.startsWith("send_message:fail")) {
            QString errorMessage = line.section(':', 2); // Получаем сообщение об ошибке
            qDebug() << "Failed to send message:" << errorMessage;
        } else if (line.startsWith("message_item:")) {
            QString message = line.section(':', 1); // Получаем текст сообщения
            messagesHistoryWidget->append(message); // Показываем сообщение в истории сообщений
        } else if (line == "end_of_messages") {
            qDebug() << "All messages have been received.";
            requestUserId(this->login);
        } else if (line.startsWith("user_id:")) {
            qDebug() << "Login: " << login << "\n";
            userId = line.section(':', 1).toInt();
            qDebug() << "User ID for requested login is:" << userId;
        }
        // Обработка других команд...
    }
}

void Chat::requestUserId(const QString &login) {
    qDebug() << "Login: " << login << "\n";
    if (m_socket->isOpen()) {
        QTextStream stream(m_socket);
        stream << "get_user_id:" << login << "\n"; // Перенос строки добавлен сразу после запроса
        stream.flush();
    } else {
        qDebug() << "Socket is not open. Cannot request user ID.";
    }
}
