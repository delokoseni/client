#ifndef MESSENGER_H
#define MESSENGER_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QTcpSocket>
#include <QStackedWidget>

class Messenger : public QMainWindow
{
    Q_OBJECT

private:
    int newChatId;
    unsigned int window_width = 800;
    unsigned int window_height = 600;
    const QString login;
    QTcpSocket *m_socket;
    QLineEdit *searchEdit;
    QPushButton *exitButton, *searchButton;
    QListWidget *usersListWidget; //Виджет для отображения списка пользователей (в поиске)
    QListWidget *chatsListWidget; //Виджет для отображения списка чатов (которые уже есть)
    QStackedWidget *stackedWidgets;

    void processServerResponse(const QString &response);
    void onHideInterfaceElements();
    void onShowInterfaceElements();
    void refreshChatsList();

public:
    explicit Messenger(QTcpSocket* socket, QWidget *parent = nullptr, const QString login = "");

private slots:
    void performSearch();
    void onReadyRead();
    void onSearchTextChanged(const QString &text);
    void onUserListItemClicked(QListWidgetItem *item); //Обработчик клика по элементу пользователей (в поиске)
    void onChatListItemClicked(QListWidgetItem *item); //Обработчик клика по элементу чатов (которые уже есть)

};

#endif // MESSENGER_H
