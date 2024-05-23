#include "Login.h"
#include "Messenger.h"

#include <QDebug>
#include <QTextStream>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QRegularExpression>
#include <QMessageBox>

Login::Login(QTcpSocket* socket, QWidget *parent) : QMainWindow(parent), m_socket(socket)
{
    setupUI();
    connectSignals();
}

void Login::setupUI()
{
    setWindowTitle("Вход");
    resize(window_width, window_height);
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    QWidget *loginWidget = new QWidget();
    QVBoxLayout *loginLayout = new QVBoxLayout(loginWidget);
    QFormLayout *loginFormLayout = new QFormLayout();
    usernameInput = new QLineEdit();
    passwordInput = new QLineEdit();
    passwordInput->setEchoMode(QLineEdit::Password);
    loginButton = new QPushButton("Войти");
    registerPrompt = new QLabel("Нет учетной записи? <a href='register'>Зарегистрироваться</a>");
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
    QWidget *registerWidget = new QWidget();
    QVBoxLayout *registerLayout = new QVBoxLayout(registerWidget);
    QFormLayout *registerFormLayout = new QFormLayout();
    newUsernameInput = new QLineEdit();
    newPasswordInput = new QLineEdit();
    repeatPasswordInput = new QLineEdit();
    newPasswordInput->setEchoMode(QLineEdit::Password);
    repeatPasswordInput->setEchoMode(QLineEdit::Password);
    registerButton = new QPushButton("Зарегистрироваться");
    errorLabel = new QLabel();
    errorLabel->setStyleSheet("QLabel { color: red; }");
    errorLabel->setAlignment(Qt::AlignCenter | Qt::AlignCenter);
    errorLabel->hide();
    registerFormLayout->addRow(new QLabel("Логин:"), newUsernameInput);
    registerFormLayout->addRow(new QLabel("Пароль:"), newPasswordInput);
    registerFormLayout->addRow(new QLabel("Повторите пароль:"), repeatPasswordInput);
    registerFormLayout->addWidget(registerButton);
    registerFormLayout->addRow("", errorLabel);
    backButton = new QPushButton("Назад");
    registerLayout->addLayout(registerFormLayout);
    registerLayout->addWidget(backButton);
    registerWidget->setLayout(registerLayout);
    stackedWidget->addWidget(registerWidget);
}

void Login::connectSignals()
{
    connect(m_socket, &QTcpSocket::readyRead, this, &Login::onReadyRead);
    connect(loginButton, &QPushButton::clicked, this, [this]()
    {
        sendLoginRequest(usernameInput->text(), passwordInput->text());
    });
    connect(registerPrompt, &QLabel::linkActivated, this, [this]()
    {
        stackedWidget->setCurrentIndex(1);
        setWindowTitle("Регистрация");
        usernameInput->clear();
        passwordInput->clear();
        loginErrorLabel->setText("");
    });
    connect(backButton, &QPushButton::clicked, this, [this]()
    {
        stackedWidget->setCurrentIndex(0);
        setWindowTitle("Вход");
        errorLabel->setText("");
        newUsernameInput->clear();
        newPasswordInput->clear();
        repeatPasswordInput->clear();
    });
    connect(registerButton, &QPushButton::clicked, this, [this]()
    {
        QString newUsername = newUsernameInput->text();
        QString newPassword = newPasswordInput->text();
        QString repeatPassword = repeatPasswordInput->text();
        errorLabel->show();
        if (newUsername.isEmpty())
        {
            errorLabel->setText("Логин не может быть пустым");
            return;
        }
        else if (newUsername.contains(QRegularExpression("[А-я]")))
        {
            errorLabel->setText("Логин содержит запрещенные символы");
            return;
        }
        else if (newPassword != repeatPassword)
        {
            errorLabel->setText("Пароли не совпадают");
            return;
        }
        else if (newPassword.length() < 8 ||
                   !newPassword.contains(QRegularExpression("[A-Z]")) ||
                   !newPassword.contains(QRegularExpression("[a-z]")) ||
                   !newPassword.contains(QRegularExpression("[0-9]")) ||
                   newPassword.contains(QRegularExpression("[А-я]")) ||
                   !newPassword.contains(QRegularExpression("[!@#$%^&*(),.?\":{}|<>]")))
        {
            errorLabel->setText("Пароль слишком слабый\nили содержит запрещенные символы");
            return;
        }
        else if (newPassword.length() > 255)
        {
            errorLabel->setText("Пароль слишком длинный");
            return;
        }
        else
        {
            errorLabel->clear();
            errorLabel->hide();
            if (m_socket->isOpen())
            {
                QTextStream stream(m_socket);
                stream << "register:" << newUsername << ":" << newPassword;
            }
            else
            {
                QMessageBox::information(this, "Ошибка!", "Кажется, что-то пошло не так. Попробуйте позже.");
            }
        }
    });
    connect(usernameInput, &QLineEdit::returnPressed, this, [this]()
    {
            sendLoginRequest(usernameInput->text(), passwordInput->text());
    });
    connect(passwordInput, &QLineEdit::returnPressed, this, [this]()
    {
        sendLoginRequest(usernameInput->text(), passwordInput->text());
    });
    connect(newUsernameInput, &QLineEdit::returnPressed, this, [this]()
    {
        registerButton->click();
    });
    connect(newPasswordInput, &QLineEdit::returnPressed, this, [this]()
    {
        registerButton->click();
    });
    connect(repeatPasswordInput, &QLineEdit::returnPressed, this, [this]()
    {
        registerButton->click();
    });
}

void Login::sendLoginRequest(const QString &username, const QString &password)
{
     if (!username.isEmpty() && !password.isEmpty() && m_socket->isOpen())
     {
         QTextStream stream(m_socket);
         login = username;
         stream << "login:" << username << ":" << password << '\n';
         stream.flush();
     }
     else if(m_socket->isOpen())
     {
         QMessageBox::information(this, "Ошибка!", "Не введен логин или пароль.");
     }
     else
     {
         QMessageBox::information(this, "Ошибка!", "Кажется, что-то пошло не так. Попробуйте позже.");
     }
}

void Login::onReadyRead()
{
    QTextStream stream(m_socket);
    QString response = stream.readAll().trimmed();
    QStringList parts = response.split(":");
    if (response.startsWith("login:success"))
    {
        this->hide();
        disconnect(m_socket, &QTcpSocket::readyRead, this, &Login::onReadyRead);
        Messenger *messenger = new Messenger(m_socket, nullptr, login);
        messenger->show();
        messenger->setAttribute(Qt::WA_DeleteOnClose);
        connect(messenger, &Messenger::destroyed, [this]() { this->close(); });
    }
    else if (response.startsWith("login:fail"))
    {
        loginErrorLabel->setText("Введены неверный логин или пароль");
        loginErrorLabel->show();
    }
    else if(response.startsWith("register:fail:username taken"))
    {
        errorLabel->setText("Этот логин уже используется");
        errorLabel->show();
        stackedWidget->setCurrentIndex(1);
    }
    else if(response.startsWith("register:success"))
    {
        stackedWidget->setCurrentIndex(0);
        setWindowTitle("Вход");
        registerSuccessLabel->setText("Регистрация прошла успешно");
        registerSuccessLabel->show();
        QTimer::singleShot(5000, registerSuccessLabel, &QLabel::hide);
    }
}
