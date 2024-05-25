#include "Chat.h"

#include <QScrollBar>
#include <QMessageBox>
#include <QShortcut>

Chat::Chat(QTcpSocket* socket, int chatId, const QString& login, QWidget* parent)
    : QWidget(parent), chatId(chatId), m_socket(socket), login(login)
{
    setupUi();
    connectSignalsAndSlots();
    requestUserId(this->login);
}

void Chat::setupUi()
{
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
    new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(sendMessage()));
    new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(sendMessage()));
    new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(onBackButtonClicked()));
}

void Chat::connectSignalsAndSlots()
{
    connect(sendMessageButton, &QPushButton::clicked, this, &Chat::sendMessage);
    connect(backButton, &QPushButton::clicked, this, &Chat::onBackButtonClicked);
    connect(m_socket, &QTcpSocket::readyRead, this, &Chat::onReadyRead);
}

void Chat::loadMessages()
{
    if (m_socket->isOpen())
    {
        QTextStream stream(m_socket);
        stream << "get_messages:" << chatId << ":" << userId <<"\n";
        stream.flush();
    }
    else
    {
        QMessageBox::information(this, "Ошибка!", "Кажется, что-то пошло не так. Попробуйте позже.");
        qDebug() << "Socket is not open. Cannot load messages.";
    }
}

void Chat::sendMessage()
{
    QString messageText = messageInputWidget->text();

    if (!messageText.isEmpty())
    {
        if (m_socket->isOpen())
        {
            QTextStream stream(m_socket);
            stream << "send_message:" << chatId << ":" << userId << ":" << messageText << "\n";
            stream.flush();
            QTextCursor cursor(messagesHistoryWidget->textCursor());
            cursor.movePosition(QTextCursor::End);
            QTextBlockFormat blockFormat;
            blockFormat.setAlignment(Qt::AlignRight);
            blockFormat.setRightMargin(0);
            blockFormat.setLeftMargin(0);
            QTextCharFormat charFormat;
            charFormat.setForeground(QBrush(Qt::blue));
            cursor.insertBlock(blockFormat, charFormat);
            cursor.insertText(messageText);
            messageInputWidget->clear();
            QScrollBar *scrollBar = messagesHistoryWidget->verticalScrollBar();
            scrollBar->setValue(scrollBar->maximum());
        }
        else
        {
            QMessageBox::information(this, "Ошибка!", "Кажется, что-то пошло не так. Попробуйте позже.");
            qDebug() << "Socket is not open. Cannot send message.";
        }
    }
}

void Chat::onBackButtonClicked()
{
    emit backToChatsList();
    this->deleteLater();
}

void Chat::onReadyRead()
{
    QTextStream stream(m_socket);
    while (!stream.atEnd())
    {
        QString line = stream.readLine().trimmed();

        if (line.startsWith("send_message:success"))
        {
            qDebug() << "Message sent successfully.";
        }
        else if (line.startsWith("send_message:fail"))
        {
            QString errorMessage = line.section(':', 2, -1);
            qDebug() << "Failed to send message:" << errorMessage;
            QMessageBox::information(this, "Ошибка!", "Кажется, что-то пошло не так. Попробуйте позже.");
        }
        else if (line.startsWith("message_item:"))
        {
            int lastColonIndex = line.lastIndexOf(':');
            if (lastColonIndex != -1)
            {
                QString messageText = line.mid(13, lastColonIndex - 13);
                int senderId = line.mid(lastColonIndex + 1).toInt();
                QTextCursor cursor(messagesHistoryWidget->textCursor());
                cursor.movePosition(QTextCursor::End);
                QTextBlockFormat blockFormat;
                blockFormat.setRightMargin(0);
                blockFormat.setLeftMargin(0);

                if (senderId == userId) {
                    blockFormat.setAlignment(Qt::AlignRight);
                    QTextCharFormat charFormat;
                    charFormat.setForeground(QBrush(Qt::blue));
                    cursor.setBlockFormat(blockFormat);
                    cursor.setBlockCharFormat(charFormat);
                }
                else
                {
                    blockFormat.setAlignment(Qt::AlignLeft);
                    QTextCharFormat charFormat;
                    charFormat.setForeground(QBrush(Qt::black));
                    cursor.setBlockFormat(blockFormat);
                    cursor.setBlockCharFormat(charFormat);
                }

                cursor.insertText(messageText);
                cursor.insertBlock();
            }
        }
        else if (line == "end_of_messages")
        {
            qDebug() << "All messages have been received.";
            QScrollBar *scrollBar = messagesHistoryWidget->verticalScrollBar();
            scrollBar->setValue(scrollBar->maximum());
        }
        else if (line.startsWith("user_id:"))
        {
            userId = line.section(':', 1).toInt();
            qDebug() << "User ID for requested login is:" << userId;
            loadMessages();
        }
        else if (line.startsWith("create_chat:"))
        {
            QStringList parts = line.split(":");
            chatId = parts.at(2).toInt();
            qDebug() << "New chat id: " << chatId << "\n";
        }
        else if(line.startsWith("new_message_in_chat:"))
        {
            int receivedChatId = line.section(':', 1).toInt();
            if(receivedChatId == this->chatId)
            {
                qDebug() << "New message received in chat ID" << this->chatId << ". Loading new messages...";
                messagesHistoryWidget->clear();
                loadMessages();
            }
        }
    }
}


void Chat::requestUserId(const QString &login)
{
    qDebug() << "Login: " << login << "\n";
    if (m_socket->isOpen())
    {
        QTextStream stream(m_socket);
        stream << "get_user_id:" << login << "\n";
        stream.flush();
    }
    else
    {
        QMessageBox::information(this, "Ошибка!", "Кажется, что-то пошло не так. Попробуйте позже.");
        qDebug() << "Socket is not open. Cannot request user ID.";
    }
}
