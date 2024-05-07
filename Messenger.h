#ifndef MESSENGER_H
#define MESSENGER_H

#include <QMainWindow>

class Messenger : public QMainWindow {
    Q_OBJECT

public:
    explicit Messenger(QWidget *parent = nullptr);
};

#endif // MESSENGER_H
