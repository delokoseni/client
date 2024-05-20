#ifndef CHAT_H
#define CHAT_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTcpSocket>

class Chat : public QWidget {
    Q_OBJECT

public:
    explicit Chat(QTcpSocket* socket, int chatId, const QString& login, QWidget* parent = nullptr);

signals:
    void backToChatsList(); //Сигнал для возврата к списку чатов

private slots:
    void sendMessage();
    void loadMessages();
    void onBackButtonClicked();
    void onConnected();
    void onReadyRead();

private:
    int chatId, userId = 0;
    QTextEdit *messagesHistoryWidget;
    QLineEdit *messageInputWidget;
    QPushButton *sendMessageButton;
    QPushButton *backButton;
    QVBoxLayout *layout;
    QTcpSocket *m_socket;
    const QString login;

    void setupUi();
    void connectSignalsAndSlots();
    void retrieveMessages();
    void requestUserId(const QString &login);
};

#endif // CHAT_H
