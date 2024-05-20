#include "Messenger.h"
#include "Login.h"

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Login client("127.0.0.1", 3000);
    client.show();
    return app.exec();
}
