#ifndef MESSENGER_H
#define MESSENGER_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QTcpSocket>

class Messenger : public QMainWindow {
    Q_OBJECT

public:
    explicit Messenger(const QString &host, int port, QWidget *parent = nullptr);
    void connectToServer();

public slots:
    void performSearch(); // Слот для обработки поиска
    void onConnected();
    void onReadyRead();

private:
    unsigned int window_width = 800, window_height = 600;
    QString m_host;
    int m_port;
    QTcpSocket *m_socket;
    QLineEdit *searchEdit;
    QPushButton *exitButton;
    QListWidget *usersListWidget; // Виджет для отображения списка пользователей

};

#endif // MESSENGER_H
