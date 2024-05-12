#ifndef CHAT_H
#define CHAT_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSqlDatabase>
#include <QSqlQuery>

class Chat : public QWidget {
    Q_OBJECT

public:
    explicit Chat(QWidget *parent = nullptr, int chatId = -1);
    ~Chat() override;

signals:
    void backToChatsList(); // Сигнал для возврата к списку чатов

private slots:
    void sendMessage();
    void loadMessages();
    void onBackButtonClicked();

private:
    int chatId;
    QTextEdit *messagesHistoryWidget;
    QLineEdit *messageInputWidget;
    QPushButton *sendMessageButton;
    QPushButton *backButton;
    QVBoxLayout *layout;

    void setupUi();
    void connectSignalsAndSlots();
    void retrieveMessages();
};

#endif // CHAT_H
