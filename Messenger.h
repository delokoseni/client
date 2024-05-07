#ifndef MESSENGER_H
#define MESSENGER_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>

class Messenger : public QMainWindow {
    Q_OBJECT

public:
    explicit Messenger(QWidget *parent = nullptr);

private:
    QLineEdit *searchEdit;
    QPushButton *exitButton;
};

#endif // MESSENGER_H
