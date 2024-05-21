#include "Login.h"

Login::Login(QTcpSocket* socket, QWidget *parent) : QMainWindow(parent), m_socket(socket)
{
    connectToServer();
    setWindowTitle("Вход"); // Установка названия окна
    resize(window_width, window_height); // Установка размеров окна

    // Стек виджетов для переключения между экранами входа и регистрации
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    // Виджеты для экрана входа
    QWidget *loginWidget = new QWidget();
    QVBoxLayout *loginLayout = new QVBoxLayout(loginWidget);
    QFormLayout *loginFormLayout = new QFormLayout();
    QLineEdit *usernameInput = new QLineEdit();
    QLineEdit *passwordInput = new QLineEdit();
    passwordInput->setEchoMode(QLineEdit::Password);
    QPushButton *loginButton = new QPushButton("Войти");

    QLabel *registerPrompt = new QLabel("Нет учетной записи? <a href='register'>Зарегистрироваться</a>");
    registerPrompt->setTextFormat(Qt::RichText);
    registerPrompt->setTextInteractionFlags(Qt::TextBrowserInteraction);

    loginErrorLabel = new QLabel();
    loginErrorLabel->setStyleSheet("QLabel { color: red; }");
    loginErrorLabel->setAlignment(Qt::AlignCenter);
    loginErrorLabel->hide();

    registerSuccessLabel = new QLabel();
    registerSuccessLabel->setStyleSheet("QLabel { color: green; }");
    registerSuccessLabel->setAlignment(Qt::AlignCenter);
    registerSuccessLabel->hide();

    loginFormLayout->addRow(new QLabel("Логин:"), usernameInput);
    loginFormLayout->addRow(new QLabel("Пароль:"), passwordInput);
    loginFormLayout->addWidget(loginButton);
    loginFormLayout->addWidget(registerPrompt);
    loginFormLayout->addRow(registerSuccessLabel);
    loginFormLayout->addWidget(loginErrorLabel);

    loginLayout->addLayout(loginFormLayout);
    loginWidget->setLayout(loginLayout);
    stackedWidget->addWidget(loginWidget);

    // Виджеты для экрана регистрации
    QWidget *registerWidget = new QWidget();
    QVBoxLayout *registerLayout = new QVBoxLayout(registerWidget);
    QFormLayout *registerFormLayout = new QFormLayout();
    QLineEdit *newUsernameInput = new QLineEdit();
    QLineEdit *newPasswordInput = new QLineEdit();
    QLineEdit *repeatPasswordInput = new QLineEdit();
    newPasswordInput->setEchoMode(QLineEdit::Password);
    repeatPasswordInput->setEchoMode(QLineEdit::Password);
    QPushButton *registerButton = new QPushButton("Зарегистрироваться");
    errorLabel = new QLabel();
    errorLabel->setStyleSheet("QLabel { color: red; }");
    errorLabel->setAlignment(Qt::AlignCenter | Qt::AlignCenter);
    errorLabel->hide();

    registerFormLayout->addRow(new QLabel("Логин:"), newUsernameInput);
    registerFormLayout->addRow(new QLabel("Пароль:"), newPasswordInput);
    registerFormLayout->addRow(new QLabel("Повторите пароль:"), repeatPasswordInput);
    registerFormLayout->addWidget(registerButton);
    registerFormLayout->addRow("", errorLabel);
    QPushButton *backButton = new QPushButton("Назад");
    registerLayout->addLayout(registerFormLayout);
    registerLayout->addWidget(backButton);
    registerWidget->setLayout(registerLayout);
    stackedWidget->addWidget(registerWidget);

    connect(loginButton, &QPushButton::clicked, this, [this, usernameInput, passwordInput]() {
        sendLoginRequest(usernameInput->text(), passwordInput->text());
    });

    connect(registerPrompt, &QLabel::linkActivated, this, [this, usernameInput, passwordInput]() {
        stackedWidget->setCurrentIndex(1);
        setWindowTitle("Регистрация");
        usernameInput->clear();
        passwordInput->clear();
        loginErrorLabel->setText("");
    });

    connect(backButton, &QPushButton::clicked, this, [this, newUsernameInput, newPasswordInput, repeatPasswordInput]() {
        stackedWidget->setCurrentIndex(0);
        setWindowTitle("Вход");
        errorLabel->setText("");
        newUsernameInput->clear();
        newPasswordInput->clear();
        repeatPasswordInput->clear();
    });

    connect(registerButton, &QPushButton::clicked, this, [this, newUsernameInput, newPasswordInput, repeatPasswordInput]() {
        QString newUsername = newUsernameInput->text();
        QString newPassword = newPasswordInput->text();
        QString repeatPassword = repeatPasswordInput->text();
        errorLabel->show();
        if (newUsername.isEmpty())
        {
            errorLabel->setText("Логин не может быть пустым");
            return;
        } else if (newUsername.contains(QRegularExpression("[А-я]")))
        {
            errorLabel->setText("Логин содержит запрещенные символы");
            return;
        }
        else if (newPassword != repeatPassword)
        {
            errorLabel->setText("Пароли не совпадают");
            return;
        } else if (newPassword.length() < 8 ||
                   !newPassword.contains(QRegularExpression("[A-Z]")) ||
                   !newPassword.contains(QRegularExpression("[a-z]")) ||
                   !newPassword.contains(QRegularExpression("[0-9]")) ||
                   newPassword.contains(QRegularExpression("[А-я]")) ||
                   !newPassword.contains(QRegularExpression("[!@#$%^&*(),.?\":{}|<>]")))
        {
            errorLabel->setText("Пароль слишком слабый\nили содержит запрещенные символы");
            return;
        } else if (newPassword.length() > 255)
        {
            errorLabel->setText("Пароль слишком длинный");
            return;
        } else {
            // Отправить данные на сервер для регистрации
            errorLabel->clear();
            errorLabel->hide();
            if (m_socket->isOpen()) {
                QTextStream stream(m_socket);
                stream << "register:" << newUsername << ":" << newPassword;
             }
        }
    });

}

void Login::connectToServer()
{
    connect(m_socket, &QTcpSocket::connected, this, &Login::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &Login::onReadyRead); // Устанавливаем соединение сигнала с слотом
}

void Login::sendLoginRequest(const QString &username, const QString &password)
{
    if (!username.isEmpty() && !password.isEmpty() && m_socket->isOpen())
    {
         QTextStream stream(m_socket);
         login = username;
         stream << "login:" << username << ":" << password << '\n';
         stream.flush();
     } else
     {
         qDebug() << "Login and password must be entered or the socket is not open.";
     }
}

void Login::onConnected()
{
    qDebug() << "Connected to server.";
}

void Login::onReadyRead()
{
    QTextStream stream(m_socket);
    QString response = stream.readAll().trimmed();
    // Проверка ответа сервера на успешный вход
    QStringList parts = response.split(":");
    if (response.startsWith("login:success")) {
        // Создаем окно чата
        this->hide();
        disconnect(m_socket, &QTcpSocket::connected, this, &Login::onConnected);
        disconnect(m_socket, &QTcpSocket::readyRead, this, &Login::onReadyRead);
        Messenger *messenger = new Messenger(m_socket, nullptr, login);
        messenger->show();
        messenger->setAttribute(Qt::WA_DeleteOnClose); // Установить флаг для автоматического удаления
        // Закрыть текущее окно входа
        connect(messenger, &Messenger::destroyed, [this]() { this->close(); });
    } else if (response.startsWith("login:fail")) {
        loginErrorLabel->setText("Введены неверный логин или пароль");
        loginErrorLabel->show();
    } else if(response.startsWith("register:fail:username taken")) {
        errorLabel->setText("Этот логин уже используется");
        errorLabel->show();
        stackedWidget->setCurrentIndex(1);
    } else if(response.startsWith("register:success")) {
        stackedWidget->setCurrentIndex(0);
        setWindowTitle("Вход");
        registerSuccessLabel->setText("Регистрация прошла успешно");
        registerSuccessLabel->show();
        QTimer::singleShot(5000, registerSuccessLabel, &QLabel::hide);
    }
}
