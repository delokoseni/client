#include "Messenger.h"
#include <QVBoxLayout>
#include <QHBoxLayout> // Добавьте этот заголовочный файл для горизонтального макета
#include <QWidget>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Messenger::Messenger(const QString &host, int port, QWidget *parent)
: QMainWindow(parent), m_host(host), m_port(port), m_socket(new QTcpSocket(this)) {
    setWindowTitle("Чаты");
    resize(window_width, window_height);

    // Основной виджет и вертикальный макет
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *verticalLayout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    // Горизонтальный макет для поиска
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Поиск...");
    searchLayout->addWidget(searchEdit);

    // Создаем кнопку "Найти" и добавляем её в горизонтальный макет
    QPushButton *searchButton = new QPushButton("Найти");
    searchLayout->addWidget(searchButton);

    usersListWidget = new QListWidget(this);
    verticalLayout->addWidget(usersListWidget);

    // Свяжите нажатие кнопки "Найти" со слотом performSearch:
    connect(searchButton, &QPushButton::clicked, this, &Messenger::performSearch);

    // Добавляем горизонтальный макет в вертикальный макет
    verticalLayout->addLayout(searchLayout);

    // Настраиваем макет, чтобы кнопка "Выйти" была внизу
    verticalLayout->addStretch();

    // Добавляем кнопку "Выйти" в нижней части
    exitButton = new QPushButton("Выйти");
    verticalLayout->addWidget(exitButton);

    // Подключаем кнопку "Выйти" к слоту close() окна
    connect(exitButton, &QPushButton::clicked, this, &Messenger::close);
}

void Messenger::connectToServer()
{
    connect(m_socket, &QTcpSocket::connected, this, &Messenger::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &Messenger::onReadyRead); // Устанавливаем соединение сигнала с слотом
    m_socket->connectToHost(m_host, m_port);
}

void Messenger::sendSearchRequest(const QString &searchText) {
    if (m_socket->isOpen() && !searchText.isEmpty()) {
        QTextStream stream(m_socket);
        stream << "search:" << searchText << '\n';
        stream.flush();
    }
}

void Messenger::performSearch() {
    QString searchText = searchEdit->text().trimmed();
    sendSearchRequest(searchText);
}

void Messenger::onConnected()
{
    qDebug() << "Connected to server.";
}

void Messenger::onReadyRead()
{
    qDebug() << "Connected to server.";
}
