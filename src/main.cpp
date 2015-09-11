#include <QCoreApplication>

#include "chat/chatcontroller.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "Starting application";

    ChatController server;
    server.listen(QHostAddress::LocalHost, 8888);

    if ( !server.isListening() ) {
        qWarning() << "Server is not listening";
    }
    else {
        qDebug() << "Your server is listening";
    }

    return a.exec();
}
