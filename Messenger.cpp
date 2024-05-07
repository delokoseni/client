#include "Messenger.h"
#include <QVBoxLayout>
#include <QHBoxLayout> // Добавьте этот заголовочный файл для горизонтального макета
#include <QWidget>

Messenger::Messenger(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Чаты");
    resize(800, 600);

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
