#include "Messenger.h"
#include <QVBoxLayout>
#include <QHBoxLayout> // Добавьте этот заголовочный файл для горизонтального макета
#include <QWidget>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Messenger::Messenger(const QString &host, int port, QWidget *parent)
: QMainWindow(parent), m_host(host), m_port(port), m_socket(new QTcpSocket(this))
{
    connectToServer(); //???

        setWindowTitle("Чаты");
        resize(window_width, window_height);

        QHBoxLayout *searchLayout = new QHBoxLayout();
        searchEdit = new QLineEdit();
        searchEdit->setPlaceholderText("Поиск...");
        searchLayout->addWidget(searchEdit);

        QPushButton *searchButton = new QPushButton("Найти");
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
}

void Messenger::onReadyRead()
{
    QTextStream stream(m_socket);
    QString buffer;

    while (!stream.atEnd())
    {
        QString line = stream.readLine().trimmed();
        qDebug() << "Response: " << line;
        buffer += line + "\n"; // Добавляем перенос строки для корректного разделения

        if (line == "search_end") // Если обнаружен конец сообщения
        {
            processServerResponse(buffer.trimmed()); // Обрабатываем весь собранный ответ
            buffer.clear(); // Очищаем буфер после обработки
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
    QStringList lines = response.split('\n'); // Разделяем буфер по строкам
    for (const QString &line : lines) {
        if (line.startsWith("search_result:")) {
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

