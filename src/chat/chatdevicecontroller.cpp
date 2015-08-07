#include "chatdevicecontroller.h"
#include "chat/chatcontroller.h"

ChatDeviceController::ChatDeviceController(QTcpSocket *socket, ChatController *controller, QByteArray username, QObject *parent) :
    QObject(parent),
    socket(socket),
    chatController(controller),
    myUsername(username)
{

}

void ChatDeviceController::listen()
{
    connect( socket, &QTcpSocket::readyRead, this, &ChatDeviceController::onNewData );
}

void ChatDeviceController::onNewData()
{
    QByteArray data = socket->readAll();

    qDebug() << "Received user data";

    if ( data.indexOf("CONNECT:") == 0 ) {
        QByteArray connectStatement = stripRequest(data, "CONNECT:");

        if ( connectStatement.length() == 0 ) {
            socket->write(QByteArrayLiteral("ERROR:NO USER\r\n\r\n"));
            return;
        }

        onConnect(connectStatement);
    }
    else if ( data.indexOf("SEND:") == 0 ) {
        QByteArray sendStatement = stripRequest(data, "SEND:");
        const QByteArray seperator("\r\n");

        if ( sendStatement.length() == 0 ) {
            socket->write(QByteArrayLiteral("ERROR:NO USER\r\n\r\n"));
            return;
        }

        int seperatorIndex = sendStatement.indexOf(seperator);

        if ( seperatorIndex == -1 ) {
            socket->write(QByteArrayLiteral("ERROR:MISSING SEPERATOR\r\n\r\n"));
            return;
        }

        if ( seperatorIndex == 0 ) {
            socket->write(QByteArrayLiteral("ERROR:MISSING USER\r\n\r\n"));
            return;
        }

        QByteArray username = sendStatement.left(seperatorIndex);
        QByteArray message = sendStatement.mid(seperatorIndex + seperator.length());

        chatController->sendMessageToUserFromUser(myUsername, username, message);
    }
    else {
        socket->write(QByteArrayLiteral("ERROR:UNKNOWN ACTION\r\n\r\n"));
        return;
    }
}

void ChatDeviceController::onConnect(QByteArray username)
{
    chatController->connectingUsers(myUsername, username);
}

QByteArray ChatDeviceController::stripRequest(QByteArray data, QByteArray command)
{
    const QByteArray endLiteral("\r\n\r\n");

    if ( data.indexOf(command) != 0 ) {
        socket->write(QByteArrayLiteral("ERROR:UNKNOWN COMMAND\r\n\r\n"));
        socket->close();
        return QByteArrayLiteral("");
    }

    data = data.remove(0, command.length());

    int endIndex = data.indexOf(endLiteral);
    if ( endIndex == -1 ) {
        socket->write(QByteArrayLiteral("ERROR:NO END\r\n\r\n"));
        socket->close();
        return QByteArrayLiteral("");
    }

    // Get user name
    data = data.remove(endIndex, endLiteral.length());

    return data;
}

