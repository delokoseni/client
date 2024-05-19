#include "Messenger.h"
#include <QVBoxLayout>
#include <QHBoxLayout> // Добавьте этот заголовочный файл для горизонтального макета
#include <QWidget>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include "Chat.h"

Messenger::Messenger(const QString &host, int port, QWidget *parent, const QString login)
: QMainWindow(parent), login(login), m_host(host), m_port(port), m_socket(new QTcpSocket(this))
{
    connectToServer(); //???

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

        // Инициализация и добавление QStackedWidget
        stackedWidgets = new QStackedWidget(this);
        usersListWidget = new QListWidget();
        chatsListWidget = new QListWidget();
        stackedWidgets->addWidget(chatsListWidget); // Индекс 0
        stackedWidgets->addWidget(usersListWidget); // Индекс 1

        // Здесь могут быть добавлены существующие чаты в chatsListWidget...

        // В конструкторе Messenger после инициализации виджетов добавляем следующие строки:
        connect(usersListWidget, &QListWidget::itemClicked, this, &Messenger::onUserListItemClicked);
        connect(chatsListWidget, &QListWidget::itemClicked, this, &Messenger::onChatListItemClicked);

        verticalLayout->addLayout(searchLayout);
        verticalLayout->addWidget(stackedWidgets);

        exitButton = new QPushButton("Выйти");
        verticalLayout->addWidget(exitButton);
        connect(exitButton, &QPushButton::clicked, this, &Messenger::close);
}

void Messenger::connectToServer()
{
    connect(m_socket, &QTcpSocket::connected, this, &Messenger::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &Messenger::onReadyRead); // Устанавливаем соединение сигнала с слотом
    m_socket->connectToHost(m_host, m_port);
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
            stream << "search:" << searchText << '\n';
            stream.flush();
            qDebug() << "search:" << searchText << '\n';
        }
        stackedWidgets->setCurrentIndex(1); // Показываем результаты поиска
    }
    else
    {
        stackedWidgets->setCurrentIndex(0); // Показываем список чатов, если поле поиска пустое
    }
}

void Messenger::onConnected()
{
    qDebug() << "Connected to server.";
    refreshChatsList();
}

void Messenger::onReadyRead()
{
    QTextStream stream(m_socket);
    QString buffer;

    while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();
            qDebug() << "Response: " << line;
            if (line == "search_end") { // Проверяем, является ли line сигналом окончания блока поиска
                processServerResponse(buffer.trimmed()); // Обрабатываем весь собранный ответ как блок
                buffer.clear(); // Сбрасываем buffer после обработки
            } else {
                buffer += line + "\n"; // Если это не конец, продолжаем "сборку" блока
            }
            if (!line.startsWith("search_result:") && !line.isEmpty()) {
                processServerResponse(line); // Обрабатываем каждую независимую строку отдельно
            }
        }
}

void Messenger::onSearchTextChanged(const QString &text)
{
    if (text.isEmpty())
    {
        stackedWidgets->setCurrentIndex(0); // Показываем список существующих чатов
    }
}

void Messenger::processServerResponse(const QString &response)
{
    QStringList lines = response.split('\n');
        for (const QString &line : lines) {
            if (line.startsWith("chat_list_item:")) {
                QStringList parts = line.split(":");
                if (parts.count() < 3) continue;

                QString chatId = parts.at(1);
                QString chatName = parts.at(2);
                QListWidgetItem *chatItem = new QListWidgetItem(chatName);
                chatItem->setData(Qt::UserRole, chatId);
                chatsListWidget->addItem(chatItem);
            } else if (line.startsWith("search_result:")) {
            QString username = line.section(':', 1, 1); // Получаем часть между двумя ':'
            usersListWidget->addItem(new QListWidgetItem(username));
        }
        // Другие условия, например, для команд login, register и других
    }
    if (response.contains("search_end"))
    {
        // Обработка завершения поиска
    }
}

void Messenger::onHideInterfaceElements() {
    searchEdit->hide();
    exitButton->hide();
    searchButton->hide();
}

void Messenger::onShowInterfaceElements() {
    searchEdit->show();
    exitButton->show();
    searchButton->show();
}

void Messenger::onUserListItemClicked(QListWidgetItem *item) {
    if (item) {
        onHideInterfaceElements();
        qDebug() << "Выбран пользователь: " << item->text();
        setWindowTitle(item->text());

        QString selectedUserLogin = item->text();

        // отправляем запрос на сервер для создания нового чата с выбранным пользователем
                if (m_socket->isOpen()) {
                    QTextStream stream(m_socket);
                    qDebug() << "Login: " << login << "\n";
                    stream << "create_chat:" << login + selectedUserLogin<< ":personal:" << login << ":" << selectedUserLogin << "\n";
                    stream.flush();
                }


        Chat *chatWidget = new Chat(this); // Создаем виджет чата

        // Подписываемся на сигнал для возврата к списку чатов
        connect(chatWidget, &Chat::backToChatsList, this, [this, chatWidget]() {
            stackedWidgets->setCurrentWidget(chatsListWidget); // Возвращаемся к списку чатов
            onShowInterfaceElements();
            searchEdit->clear();
            setWindowTitle("Чаты");
            chatWidget->deleteLater(); // Запрос удаления виджета чата
        });

        stackedWidgets->addWidget(chatWidget); // Добавляем виджет чата в стек
        stackedWidgets->setCurrentWidget(chatWidget); // Показываем виджет чата
    }
}

// Пример реализации слота, работающего со списком чатов
void Messenger::onChatListItemClicked(QListWidgetItem *item) {
    if (item) {
        qDebug() << "Выбран чат: " << item->text();
        setWindowTitle(item->text());
        int chatId = item->data(Qt::UserRole).toInt(); // Убедитесь, что chatId установлен в data НЕ УСТАНОВЛЕН!!!
        Chat *chatWidget = new Chat(this, chatId);
        connect(chatWidget, &Chat::backToChatsList, this, [this, chatWidget]() {
            stackedWidgets->setCurrentWidget(chatsListWidget);
            chatWidget->deleteLater();
        });

        stackedWidgets->addWidget(chatWidget);
        stackedWidgets->setCurrentWidget(chatWidget);
    }
}

void Messenger::refreshChatsList() {
    if (m_socket->isOpen()) {
        QTextStream stream(m_socket);
        stream << "get_chats:" << login << "\n"; // login - логин текущего пользователя
        stream.flush();
    }
}

