#ifndef MESSENGER_H
#define MESSENGER_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QTcpSocket>
#include <QStackedWidget>

class Messenger : public QMainWindow {
    Q_OBJECT

public:
    explicit Messenger(const QString &host, int port, QWidget *parent = nullptr, const QString login = "");
    void connectToServer();

public slots:
    void performSearch(); // Слот для обработки поиска
    void onConnected();
    void onReadyRead();
    void onSearchTextChanged(const QString &text);
    void onUserListItemClicked(QListWidgetItem *item); // Обработчик клика по элементу пользователей
    void onChatListItemClicked(QListWidgetItem *item);

private:
    unsigned int window_width = 800, window_height = 600;
    const QString login;
    QString m_host;
    int m_port;
    QTcpSocket *m_socket;
    QLineEdit *searchEdit;
    QPushButton *exitButton;
    QListWidget *usersListWidget; // Виджет для отображения списка пользователей
    QListWidget *chatsListWidget; // Добавлен виджет для отображения существующих чатов
    QStackedWidget *stackedWidgets;
    void processServerResponse(const QString &response);

};

#endif // MESSENGER_H
