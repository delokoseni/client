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
        loginFormLayout->addRow(new QLabel("Логин:"), usernameInput);
        loginFormLayout->addRow(new QLabel("Пароль:"), passwordInput);
        loginFormLayout->addWidget(loginButton);
        loginLayout->addLayout(loginFormLayout);
        loginWidget->setLayout(loginLayout);

        // Ссылка для перехода к экрану регистрации
        QLabel *registerPrompt = new QLabel("Нет учётной записи? <a href='register'>Зарегистрироваться</a>");
        registerPrompt->setTextFormat(Qt::RichText);
        registerPrompt->setTextInteractionFlags(Qt::TextBrowserInteraction);
         loginFormLayout->addWidget(registerPrompt);

        // Виджеты для экрана регистрации
        QWidget *registerWidget = new QWidget();
        QVBoxLayout *registerLayout = new QVBoxLayout(registerWidget);
        QFormLayout *registerFormLayout = new QFormLayout();
        QLineEdit *newUsernameInput = new QLineEdit();
        QLineEdit *newPasswordInput = new QLineEdit();
        newPasswordInput->setEchoMode(QLineEdit::Password);
        QLineEdit *repeatPasswordInput = new QLineEdit();
        repeatPasswordInput->setEchoMode(QLineEdit::Password);
        QPushButton *registerButton = new QPushButton("Зарегистрироваться");
        registerFormLayout->addRow(new QLabel("Логин:"), newUsernameInput);
        registerFormLayout->addRow(new QLabel("Пароль:"), newPasswordInput);
        registerFormLayout->addRow(new QLabel("Повторите пароль:"), repeatPasswordInput);
        registerFormLayout->addWidget(registerButton);
        registerLayout->addLayout(registerFormLayout);
        QPushButton *backButton = new QPushButton("Назад");
        registerLayout->addWidget(backButton);
        registerWidget->setLayout(registerLayout);

        // Добавление виджетов на стек
        stackedWidget->addWidget(loginWidget);
        stackedWidget->addWidget(registerWidget);

        // Сигналы и слоты
        connect(loginButton, &QPushButton::clicked, this, [this, usernameInput, passwordInput]() {
            QString username = usernameInput->text();
            QString password = passwordInput->text();
            // Здесь реализовать логику аутентификации
        });

        connect(registerPrompt, &QLabel::linkActivated, this, [stackedWidget]() {
            stackedWidget->setCurrentIndex(1);
        });

        connect(backButton, &QPushButton::clicked, this, [stackedWidget]() {
            stackedWidget->setCurrentIndex(0);
        });

        connect(registerButton, &QPushButton::clicked, this, [this, newUsernameInput, newPasswordInput, repeatPasswordInput]() {
            QString newUsername = newUsernameInput->text();
            QString newPassword = newPasswordInput->text();
            QString repeatPassword = repeatPasswordInput->text();
            // Здесь реализовать логику регистрации новых пользователей
        });

        connect(m_socket, &QTcpSocket::connected, this, &ChatClient::onConnected);
        connect(m_socket, &QTcpSocket::readyRead, this, &ChatClient::onReadyRead);
    }

    void connectToServer() {
        m_socket->connectToHost(m_host, m_port);
    }

private slots:
    void onConnected() {
        qDebug() << "Connected to server.";
    }

    void onReadyRead() {
        QTextStream stream(m_socket);
        qDebug() << "Server says:" << stream.readAll();
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
    return app.exec();
}
