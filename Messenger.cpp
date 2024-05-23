#include "Messenger.h"
#include "Chat.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QMessageBox>

Messenger::Messenger(QTcpSocket* socket, QWidget *parent, const QString login) : QMainWindow(parent), login(login), m_socket(socket)
{
    connect(m_socket, &QTcpSocket::readyRead, this, &Messenger::onReadyRead);

    setWindowTitle("Чаты");
    resize(window_width, window_height);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Поиск...");
    searchLayout->addWidget(searchEdit);

    searchButton = new QPushButton("Найти");
    searchLayout->addWidget(searchButton);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *verticalLayout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    connect(searchButton, &QPushButton::clicked, this, &Messenger::performSearch);
    connect(searchEdit, &QLineEdit::textChanged, this, &Messenger::onSearchTextChanged);

    stackedWidgets = new QStackedWidget(this);
    usersListWidget = new QListWidget();
    chatsListWidget = new QListWidget();
    stackedWidgets->addWidget(chatsListWidget); //Индекс 0
    stackedWidgets->addWidget(usersListWidget); //Индекс 1

    connect(usersListWidget, &QListWidget::itemClicked, this, &Messenger::onUserListItemClicked);
    connect(chatsListWidget, &QListWidget::itemClicked, this, &Messenger::onChatListItemClicked);

    verticalLayout->addLayout(searchLayout);
    verticalLayout->addWidget(stackedWidgets);

    exitButton = new QPushButton("Выйти");
    verticalLayout->addWidget(exitButton);
    connect(exitButton, &QPushButton::clicked, this, &Messenger::close);
    refreshChatsList();
}

void Messenger::performSearch()
{
    QString searchText = searchEdit->text().trimmed();
    if (!searchText.isEmpty())
    {
        if (m_socket->isOpen())
        {
            usersListWidget->clear();
            QTextStream stream(m_socket);
            stream << "search:" << searchText << ":" << login << '\n';
            stream.flush();
            qDebug() << "search:" << searchText << "requested by user:" << login << '\n';
        }
        stackedWidgets->setCurrentIndex(1);
    }
    else
    {
        stackedWidgets->setCurrentIndex(0);
    }
}

void Messenger::onReadyRead()
{
    QTextStream stream(m_socket);
    QString buffer;

    while (!stream.atEnd())
    {
            QString line = stream.readLine().trimmed();
            qDebug() << "Response: " << line;
            if (line == "search_end")
            {
                processServerResponse(buffer.trimmed());
                if(buffer.isEmpty())
                {
                    QString message = "По вашему запросу \"";
                    message += searchEdit->text();
                    message += "\" ничего не нашлось.";
                    QMessageBox::information(this, ":(", message);
                    searchEdit->clear();
                }
                else
                {
                    buffer.clear();
                }
            }
            else
            {
                buffer += line + "\n";
            }
            if (!line.startsWith("search_result:") && !line.isEmpty())
            {
                processServerResponse(line);
            }
            if(line.startsWith("new_message_in_chat:"))
            {
                int receivedChatId = line.section(':', 1).toInt();
                qDebug() << "New message received in chat ID" << receivedChatId << ". Loading new messages...";
                for(int i = 0; i < chatsListWidget->count(); i++)
                {
                    QListWidgetItem *item = chatsListWidget->item(i);

                    if (item->data(Qt::UserRole).toInt() == receivedChatId)
                    {
                        QString itemName = item->text();

                        if(!itemName.contains("(Новое сообщение)"))
                        {
                            itemName += "(Новое сообщение)";
                        }

                        item->setText(itemName);
                        break;
                    }
                }
            }
    }
}

void Messenger::onSearchTextChanged(const QString &text)
{
    if (text.isEmpty())
    {
        stackedWidgets->setCurrentIndex(0);
    }
}

void Messenger::processServerResponse(const QString &response)
{
    QStringList lines = response.split('\n');
    for (const QString &line : lines)
    {
        if (line.startsWith("chat_list_item:"))
        {
            QStringList parts = line.split(":");
            if (parts.count() < 4) continue;

            QString chatId = parts.at(1);
            QString chatName = parts.at(2);
            QString chatItemText = chatName;
            if(parts.at(3) == "has_new_messages")
            {
                chatItemText += " (Новое сообщение)";
            }
            QListWidgetItem *chatItem = new QListWidgetItem(chatItemText);
            chatItem->setData(Qt::UserRole, chatId);
            chatsListWidget->addItem(chatItem);

        }
        else if (line.startsWith("search_result:"))
        {
            QString username = line.section(':', 1, 1); //Получаем часть между двумя ':'
            usersListWidget->addItem(new QListWidgetItem(username));
        }
    }
    if (response.contains("search_end"))
    {
        qDebug() << response << "\n";
    }
}

void Messenger::onHideInterfaceElements()
{
    searchEdit->hide();
    exitButton->hide();
    searchButton->hide();
}

void Messenger::onShowInterfaceElements()
{
    searchEdit->show();
    exitButton->show();
    searchButton->show();
    refreshChatsList();
}

void Messenger::onUserListItemClicked(QListWidgetItem *item)
{
    if (item)
    {
        onHideInterfaceElements();
        qDebug() << "Выбран пользователь: " << item->text();
        setWindowTitle(item->text());

        QString selectedUserLogin = item->text();

        if (m_socket->isOpen()) {
            QTextStream stream(m_socket);
            qDebug() << "Login: " << login << "\n";
            stream << "create_chat:" << login + selectedUserLogin<< ":personal:" << login << ":" << selectedUserLogin << "\n";
            stream.flush();
        }
        qDebug() << "Login: " << login << "\n";

        disconnect(m_socket, &QTcpSocket::readyRead, this, &Messenger::onReadyRead);

        Chat *chatWidget = new Chat(m_socket, 0, login);

        connect(chatWidget, &Chat::backToChatsList, this, [this, chatWidget]()
        {
            stackedWidgets->setCurrentWidget(chatsListWidget);
            onShowInterfaceElements();
            searchEdit->clear();
            setWindowTitle("Чаты");

            connect(m_socket, &QTcpSocket::readyRead, this, &Messenger::onReadyRead);

            chatWidget->deleteLater();
        });

        stackedWidgets->addWidget(chatWidget);
        stackedWidgets->setCurrentWidget(chatWidget);
    }
}

void Messenger::onChatListItemClicked(QListWidgetItem *item)
{
    if (item)
    {
        onHideInterfaceElements();
        qDebug() << "Выбран чат: " << item->text();
        setWindowTitle(item->text());
        qDebug() << "Login: " << login << "\n";
        int chatId = item->data(Qt::UserRole).toInt();

        disconnect(m_socket, &QTcpSocket::readyRead, this, &Messenger::onReadyRead);

        Chat *chatWidget = new Chat(m_socket, chatId, login);
        connect(chatWidget, &Chat::backToChatsList, this, [this, chatWidget] ()
        {
            stackedWidgets->setCurrentWidget(chatsListWidget);
            onShowInterfaceElements();
            searchEdit->clear();
            setWindowTitle("Чаты");

            connect(m_socket, &QTcpSocket::readyRead, this, &Messenger::onReadyRead);
            chatWidget->deleteLater();

        });
        stackedWidgets->addWidget(chatWidget);
        stackedWidgets->setCurrentWidget(chatWidget);
    }
}

void Messenger::refreshChatsList()
{
    if (m_socket->isOpen())
    {
        QTextStream stream(m_socket);
        chatsListWidget->clear();
        stream << "get_chats:" << login << "\n";
        stream.flush();
    }
    else
    {
        QMessageBox::information(this, "Ошибка!", "Кажется, что-то пошло не так. Попробуйте позже.");
    }
}
