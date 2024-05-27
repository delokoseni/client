#include "Login.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QString host = "192.168.30.134";
    unsigned int port = 3000;
    QTcpSocket *socket = new QTcpSocket();
    socket->connectToHost(host, port);
    Login client(socket);
    client.show();
    return app.exec();
}
