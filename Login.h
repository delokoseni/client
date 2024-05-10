#ifndef LOGIN_H
#define LOGIN_H

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

class Login : public QMainWindow
{
    Q_OBJECT

public:
    explicit Login(const QString &host, int port, QWidget *parent = nullptr);

    void connectToServer();

    void sendLoginRequest(const QString &username, const QString &password);

private slots:
    void onConnected();

    void onReadyRead();

private:
    unsigned int window_width = 400, window_height = 300;
    QString m_host;
    int m_port;
    QTcpSocket *m_socket;
    QLabel *loginErrorLabel;
};

#endif // LOGIN_H
