#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>

class ChatClient : public QMainWindow {
    Q_OBJECT

public:
    explicit ChatClient(const QString &host, int port, QWidget *parent = nullptr);
    void connectToServer();

private slots:
    void onConnected();
    void onReadyRead();

private:
    QString m_host;
    int m_port;
    QTcpSocket *m_socket;
    QLineEdit *usernameInput;
    QLineEdit *passwordInput;
    QLineEdit *newUsernameInput;
    QLineEdit *newPasswordInput;
    QLineEdit *repeatPasswordInput;
    QLabel *registerSuccessLabel;
    QLabel *errorLabel;
    QStackedWidget *stackedWidget;

    void setupUi();
    void setupLoginUI();
    void setupRegisterUI();
};

#endif // CHATCLIENT_H
