#include "Login.h"

#include <QApplication>

#include "main.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QString host = "127.0.0.1";
    unsigned int port = 3000;
    QTcpSocket *socket = new QTcpSocket();
    socket->connectToHost(host, port);
    Login client(socket);
    client.show();
    return app.exec();
}
