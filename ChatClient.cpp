#include "ChatClient.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTimer>
#include <QRegularExpression>

ChatClient::ChatClient(const QString &host, int port, QWidget *parent) :
    QMainWindow(parent), m_host(host), m_port(port),
    m_socket(new QTcpSocket(this)), usernameInput(new QLineEdit(this)),
    passwordInput(new QLineEdit(this)), newUsernameInput(new QLineEdit(this)),
    newPasswordInput(new QLineEdit(this)), repeatPasswordInput(new QLineEdit(this)),
    registerSuccessLabel(new QLabel(this)), errorLabel(new QLabel(this)),
    stackedWidget(new QStackedWidget(this))
{
    setupUi();
    connectToServer();
}

void ChatClient::connectToServer() {
    connect(m_socket, &QTcpSocket::connected, this, &ChatClient::onConnected);
    m_socket->connectToHost(m_host, m_port);
}

void ChatClient::onConnected() {
    qDebug() << "Connected to server.";
}

void ChatClient::onReadyRead() {
    QTextStream stream(m_socket);
    QString response = stream.readAll().trimmed();
    qDebug() << "Server says:" << response;

    // Обработка ответа сервера
    if (response.startsWith("register:fail:username taken")) {
        errorLabel->setText("Этот логин уже используется");
        errorLabel->show();
        stackedWidget->setCurrentIndex(1);
    } else if (response.startsWith("register:success")) {
        stackedWidget->setCurrentIndex(0);
        registerSuccessLabel->setText("Регистрация прошла успешно");
        registerSuccessLabel->show();
        QTimer::singleShot(5000, registerSuccessLabel, &QLabel::hide);
    }
}

void ChatClient::setupUi() {
    setWindowTitle("ChatClient");
    resize(400, 300);
    setCentralWidget(stackedWidget);

    setupLoginUI();
    setupRegisterUI();
}

void ChatClient::setupLoginUI() {
    QWidget *loginWidget = new QWidget(this);
    QVBoxLayout *loginLayout = new QVBoxLayout(loginWidget);
    QFormLayout *loginFormLayout = new QFormLayout();

    passwordInput->setEchoMode(QLineEdit::Password);
    QPushButton *loginButton = new QPushButton("Войти", this);
    QLabel *registerPrompt = new QLabel("Нет учетной записи? <a href='register'>Зарегистрироваться</a>", this);

    registerSuccessLabel->setStyleSheet("QLabel { color: green; }");
    registerSuccessLabel->setAlignment(Qt::AlignCenter);
    registerSuccessLabel->hide();

    loginFormLayout->addRow(new QLabel("Логин:", this), usernameInput);
    loginFormLayout->addRow(new QLabel("Пароль:", this), passwordInput);
    loginFormLayout->addWidget(loginButton);
    loginFormLayout->addWidget(registerPrompt);
    loginFormLayout->addRow(registerSuccessLabel);

    loginLayout->addLayout(loginFormLayout);
    loginWidget->setLayout(loginLayout);

    stackedWidget->addWidget(loginWidget);

    // Signal connections for login UI
    connect(loginButton, &QPushButton::clicked, this, [this]() {
        // Логика аутентификации
    });
    connect(registerPrompt, &QLabel::linkActivated, this, [this]() {
        stackedWidget->setCurrentIndex(1);
    });
}

void ChatClient::setupRegisterUI() {
    QWidget *registerWidget = new QWidget(this);
    QVBoxLayout *registerLayout = new QVBoxLayout(registerWidget);
    QFormLayout *registerFormLayout = new QFormLayout();

    newPasswordInput->setEchoMode(QLineEdit::Password);
    repeatPasswordInput->setEchoMode(QLineEdit::Password);
    QPushButton *registerButton = new QPushButton("Зарегистрироваться", this);
    QPushButton *backButton = new QPushButton("Назад", this);

    errorLabel->setStyleSheet("QLabel { color: red; }");
    errorLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    errorLabel->hide();

    registerFormLayout->addRow(new QLabel("Логин:", this), newUsernameInput);
    registerFormLayout->addRow(new QLabel("Пароль:", this), newPasswordInput);
    registerFormLayout->addRow(new QLabel("Повторите пароль:", this), repeatPasswordInput);
    registerFormLayout->addRow("", errorLabel);
    registerFormLayout->addWidget(registerButton);
    registerLayout->addLayout(registerFormLayout);
    registerLayout->addWidget(backButton);
    registerWidget->setLayout(registerLayout);

    stackedWidget->addWidget(registerWidget);

    // Signal connections for registration UI
    connect(backButton, &QPushButton::clicked, this, [this]() {
        stackedWidget->setCurrentIndex(0);
    });
    connect(registerButton, &QPushButton::clicked, this, [this]() {
        QString username = newUsernameInput->text();
        QString password = newPasswordInput->text();
        QString repeatPassword = repeatPasswordInput->text();

        if (password != repeatPassword) {
            errorLabel->setText("Пароли не совпадают");
            errorLabel->show();
        } else {
            // Validation can be performed here and server communication for registration
        }
    });
}
