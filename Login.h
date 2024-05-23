#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QLabel>
#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QLineEdit>

class Login : public QMainWindow
{
    Q_OBJECT

private:
    unsigned int window_width = 450;
    unsigned int window_height = 300;
    QString login;
    QTcpSocket *m_socket;
    QLabel *loginErrorLabel;
    QLabel *registerSuccessLabel;
    QLabel *errorLabel;
    QLabel *registerPrompt;
    QStackedWidget *stackedWidget;
    QPushButton *loginButton;
    QPushButton *backButton;
    QPushButton *registerButton;
    QLineEdit *usernameInput;
    QLineEdit *passwordInput;
    QLineEdit *newUsernameInput;
    QLineEdit *newPasswordInput;
    QLineEdit *repeatPasswordInput;

    void sendLoginRequest(const QString &username, const QString &password);
    void setupUI();
    void connectSignals();

public:
    explicit Login(QTcpSocket* socket, QWidget *parent = nullptr);

private slots:
    void onReadyRead();

};

#endif // LOGIN_H
