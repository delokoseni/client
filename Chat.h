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
    explicit Chat(const QString &host, int port, QWidget *parent = nullptr, int chatId = -1);
    ~Chat() override;
    void connectToServer();

signals:
    void backToChatsList(); //Сигнал для возврата к списку чатов

private slots:
    void sendMessage();
    void loadMessages();
    void onBackButtonClicked();
    void onConnected();
    void onReadyRead();

private:
    int chatId, userId;
    QTextEdit *messagesHistoryWidget;
    QLineEdit *messageInputWidget;
    QPushButton *sendMessageButton;
    QPushButton *backButton;
    QVBoxLayout *layout;
    int m_port;
    QString m_host;
    QTcpSocket *m_socket;

    void setupUi();
    void connectSignalsAndSlots();
    void retrieveMessages();
};

#endif // CHAT_H
