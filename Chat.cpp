#include "Chat.h"
#include <QSqlQueryModel>

Chat::Chat(QTcpSocket* socket, int chatId, const QString& login, QWidget* parent)
    : QWidget(parent), chatId(chatId), m_socket(socket), login(login) {
    setupUi();
    connectSignalsAndSlots();
    requestUserId(this->login);
}

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
    connect(m_socket, &QTcpSocket::connected, this, &Chat::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &Chat::onReadyRead);
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
    qDebug() << "Нажатие кнопки\n";
    QString messageText = messageInputWidget->text();

    if (!messageText.isEmpty()) {
        if (m_socket->isOpen()) {
            QTextStream stream(m_socket);
            stream << "send_message:" << chatId << ":" << userId << ":" << messageText << "\n";
            stream.flush();

            // Создаем QTextCursor, связанный с QTextEdit
            QTextCursor cursor(messagesHistoryWidget->textCursor());
            cursor.movePosition(QTextCursor::End); // Перемещаем курсор к концу текста

            QTextBlockFormat blockFormat;
            blockFormat.setAlignment(Qt::AlignRight); // Выравниваем текст справа
            blockFormat.setRightMargin(0);
            blockFormat.setLeftMargin(0);

            // Установка формата текста для исходящих сообщений пользователя
            QTextCharFormat charFormat;
            charFormat.setForeground(QBrush(Qt::blue));

            cursor.insertBlock(blockFormat, charFormat); // Вставляем новый блок с данным форматом
            cursor.insertText(messageText); // Вставляем текст сообщения

            // Очищаем поле ввода сообщения
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
            QString errorMessage = line.section(':', 2, -1); // Получаем сообщение об ошибке
            qDebug() << "Failed to send message:" << errorMessage;
        } else if (line.startsWith("message_item:")) {
            int lastColonIndex = line.lastIndexOf(':');
            if (lastColonIndex != -1) {
                QString messageText = line.mid(13, lastColonIndex - 13); // Извлекаем текст сообщения
                int senderId = line.mid(lastColonIndex + 1).toInt(); // Извлекаем user_id

                // Создаем QTextCursor, связанный с QTextEdit
                QTextCursor cursor(messagesHistoryWidget->textCursor());
                cursor.movePosition(QTextCursor::End); // Перемещаем курсор к концу текста

                QTextBlockFormat blockFormat;
                blockFormat.setRightMargin(0); // Устанавливаем отступы
                blockFormat.setLeftMargin(0);

                // Выбираем выравнивание в зависимости от отправителя
                if (senderId == userId) {
                    blockFormat.setAlignment(Qt::AlignRight);
                    // Установка формата текста для сообщений пользователя
                    QTextCharFormat charFormat;
                    charFormat.setForeground(QBrush(Qt::blue));
                    cursor.setBlockFormat(blockFormat); // Применяем формат блока
                    cursor.setBlockCharFormat(charFormat); // Применяем формат символов
                } else {
                    blockFormat.setAlignment(Qt::AlignLeft);
                    // Установка формата текста для сообщений от других пользователей
                    QTextCharFormat charFormat;
                    charFormat.setForeground(QBrush(Qt::black));
                    cursor.setBlockFormat(blockFormat); // Применяем формат блока
                    cursor.setBlockCharFormat(charFormat); // Применяем формат символов
                }

                cursor.insertText(messageText); // Вставляем текст сообщения
                cursor.insertBlock(); // Вставляем новый блок для следующего сообщения
            }
        } else if (line == "end_of_messages") {
            qDebug() << "All messages have been received.";
        } else if (line.startsWith("user_id:")) {
            userId = line.section(':', 1).toInt();
            qDebug() << "User ID for requested login is:" << userId;
            loadMessages();
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
