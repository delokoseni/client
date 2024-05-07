#include <QApplication>
#include <QMainWindow>
#include <QTcpSocket>
#include <QDebug>
#include <QTextStream>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QTimer>
#include <QRegularExpression>
#include "Messenger.h"

class ChatClient : public QMainWindow {
    Q_OBJECT

public:
    explicit ChatClient(const QString &host, int port, QWidget *parent = nullptr)
        : QMainWindow(parent), m_host(host), m_port(port), m_socket(new QTcpSocket(this)) {
        setWindowTitle("ChatClient"); // Установка названия окна
        resize(400, 300); // Установка размеров окна

        // Стек виджетов для переключения между экранами входа и регистрации
        QStackedWidget *stackedWidget = new QStackedWidget(this);
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
        //Новый код
        QLabel *registerSuccessLabel = new QLabel();
        registerSuccessLabel->setStyleSheet("QLabel { color: green; }");
        registerSuccessLabel->setAlignment(Qt::AlignCenter);
        registerSuccessLabel->hide();

        loginFormLayout->addRow(new QLabel("Логин:"), usernameInput);
        loginFormLayout->addRow(new QLabel("Пароль:"), passwordInput);
        loginFormLayout->addWidget(loginButton);
        loginFormLayout->addWidget(registerPrompt);
        //loginFormLayout->addWidget(registerSuccessLabel);
        loginFormLayout->addRow(registerSuccessLabel);

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
        QLabel *errorLabel = new QLabel();
        errorLabel->setStyleSheet("QLabel { color: red; }");
        errorLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        errorLabel->hide();

        registerFormLayout->addRow(new QLabel("Логин:"), newUsernameInput);
        registerFormLayout->addRow(new QLabel("Пароль:"), newPasswordInput);
        registerFormLayout->addRow(new QLabel("Повторите пароль:"), repeatPasswordInput);
        registerFormLayout->addRow("", errorLabel);
        registerFormLayout->addWidget(registerButton);
        QPushButton *backButton = new QPushButton("Назад");
        registerLayout->addLayout(registerFormLayout);
        registerLayout->addWidget(backButton);
        registerWidget->setLayout(registerLayout);
        stackedWidget->addWidget(registerWidget);

        connect(loginButton, &QPushButton::clicked, this, [this, usernameInput, passwordInput]() {
            sendLoginRequest(usernameInput->text(), passwordInput->text());
        });

        connect(registerPrompt, &QLabel::linkActivated, this, [stackedWidget]() {
                    stackedWidget->setCurrentIndex(1);
        });

        connect(backButton, &QPushButton::clicked, this, [stackedWidget]() {
            stackedWidget->setCurrentIndex(0);
        });

        connect(registerButton, &QPushButton::clicked, this, [this, newUsernameInput, newPasswordInput, repeatPasswordInput, errorLabel]() {
            QString newUsername = newUsernameInput->text();
            QString newPassword = newPasswordInput->text();
            QString repeatPassword = repeatPasswordInput->text();
            errorLabel->show();

            if (newPassword != repeatPassword) {
                errorLabel->setText("Пароли не совпадают");
            } else if (newPassword.length() < 8 ||
                       !newPassword.contains(QRegularExpression("[A-Z]")) ||
                       !newPassword.contains(QRegularExpression("[a-z]")) ||
                       !newPassword.contains(QRegularExpression("[0-9]")) ||
                       !newPassword.contains(QRegularExpression("[!@#$%^&*(),.?\":{}|<>]"))) {
                errorLabel->setText("Пароль слишком слабый");
            } else if (newPassword.length() > 255) {
                errorLabel->setText("Пароль слишком длинный");
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

        connect(m_socket, &QTcpSocket::connected, this, &ChatClient::onConnected);

        connect(m_socket, &QTcpSocket::readyRead, this, &ChatClient::onReadyRead);


        connect(m_socket, &QTcpSocket::readyRead, this, [this, newUsernameInput, newPasswordInput, repeatPasswordInput, errorLabel, stackedWidget, registerSuccessLabel]() {
                    QTextStream stream(m_socket);
                    QString response = stream.readAll().trimmed();
                    if(response.startsWith("register:fail:username taken")) {
                        errorLabel->setText("Этот логин уже используется");
                        errorLabel->show();
                        stackedWidget->setCurrentIndex(1);
                    } else if(response.startsWith("register:success")) {
                        //errorLabel->clear();
                        //errorLabel->hide();
                        stackedWidget->setCurrentIndex(0);
                        registerSuccessLabel->setText("Регистрация прошла успешно");
                        registerSuccessLabel->show();
                        QTimer::singleShot(5000, registerSuccessLabel, &QLabel::hide);
                    }
        });
    }

    void connectToServer() {
            connect(m_socket, &QTcpSocket::connected, this, &ChatClient::onConnected);
            connect(m_socket, &QTcpSocket::readyRead, this, &ChatClient::onReadyRead); // Устанавливаем соединение сигнала с слотом
            m_socket->connectToHost(m_host, m_port);
        }

    //void connectToServer() {
        //m_socket->connectToHost(m_host, m_port);
    //}

    void sendLoginRequest(const QString &username, const QString &password) {
        if(m_socket->isOpen()) {
            QTextStream stream(m_socket);
            stream << "login:" << username << ":" << password << '\n'; // "\n" обозначает конец команды
            stream.flush();
        } else {
            qDebug() << "Socket is not open. Cannot send login request.";
        }
    }

private slots:
    void onConnected() {
        qDebug() << "Connected to server.";
    }

    void onReadyRead() {
        QTextStream stream(m_socket);
        QString response = stream.readAll().trimmed();
        // Проверка ответа сервера на успешный вход
        if (response.startsWith("login:success")) {
            // Создаем окно чата
            this->hide();
            Messenger *messenger = new Messenger(this);
            messenger->show();
            messenger->setAttribute(Qt::WA_DeleteOnClose); // Установить флаг для автоматического удаления
            // Закрыть текущее окно входа
            connect(messenger, &Messenger::destroyed, [this]() { this->close(); });
        } else if (response.startsWith("login:fail")) {
            // Ошибка входа, информировать пользователя
            // ...
        }

    }

private:
    QString m_host;
    int m_port;
    QTcpSocket *m_socket;
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ChatClient client("127.0.0.1", 3000);
    client.show();
    client.connectToServer();
    return app.exec();
}
