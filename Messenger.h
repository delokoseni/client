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
    explicit Messenger(QTcpSocket* socket, QWidget *parent = nullptr, const QString login = "");
    void connectToServer();

public slots:
    void performSearch(); //Слот для обработки поиска
    void onConnected();
    void onReadyRead();
    void onSearchTextChanged(const QString &text);
    void onUserListItemClicked(QListWidgetItem *item); //Обработчик клика по элементу пользователей
    void onChatListItemClicked(QListWidgetItem *item);//Обработчик клика по элементу чатов
    void onHideInterfaceElements();    // Слот для скрытия элементов интерфейса
    void onShowInterfaceElements();
    void refreshChatsList();

private:
    unsigned int window_width = 800, window_height = 600;
    const QString login;
    QTcpSocket *m_socket;
    QLineEdit *searchEdit;
    QPushButton *exitButton, *searchButton;
    QListWidget *usersListWidget; //Виджет для отображения списка пользователей
    QListWidget *chatsListWidget; //Виджет для отображения списка чатов
    QStackedWidget *stackedWidgets;
    void processServerResponse(const QString &response);

};

#endif // MESSENGER_H
