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
    connect( socket, &QTcpSocket::disconnected, this, &ChatDeviceController::onClose );
}

void ChatDeviceController::onNewData()
{
    const QByteArray seperator("\r\n");
    QByteArray data = socket->readAll();

    qDebug() << "Received user data";

    if ( data.indexOf("CONNECT:") == 0 ) {
        QByteArray connectStatement = stripRequest(data, "CONNECT:");

        if ( connectStatement.length() == 0 ) {
            socket->write(QByteArrayLiteral("ERROR:NO USER\r\n\r\n"));
            return;
        }

        int seperatorIndex = connectStatement.indexOf(seperator);

        if ( seperatorIndex == -1 ) {
            socket->write(QByteArrayLiteral("ERROR:MISSING SEPERATOR\r\n\r\n"));
            return;
        }

        if ( seperatorIndex == 0 ) {
            socket->write(QByteArrayLiteral("ERROR:MISSING USER\r\n\r\n"));
            return;
        }

        QByteArray username = connectStatement.left(seperatorIndex);
        QByteArray publicKey = connectStatement.mid(seperatorIndex + seperator.length());

        onConnect(username, publicKey);
    }
    else if ( data.indexOf("SEND:") == 0 ) {
        QByteArray sendStatement = stripRequest(data, "SEND:");

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
    else if ( data.indexOf("ENCRYPT:") == 0 ) {
        QByteArray encryptStatement = stripRequest(data, "ENCRYPT:");

        if ( encryptStatement.length() == 0 ) {
            socket->write(QByteArrayLiteral("ERROR:NO USER\r\n\r\n"));
            return;
        }

        int seperatorIndex = encryptStatement.indexOf(seperator);

        if ( seperatorIndex == -1 ) {
            socket->write(QByteArrayLiteral("ERROR:MISSING SEPERATOR\r\n\r\n"));
            return;
        }

        if ( seperatorIndex == 0 ) {
            socket->write(QByteArrayLiteral("ERROR:MISSING USER\r\n\r\n"));
            return;
        }

        QByteArray username = encryptStatement.left(seperatorIndex);
        QByteArray encryptedKey = encryptStatement.mid(seperatorIndex + seperator.length());

        chatController->sendEncryptionKey(myUsername, username, encryptedKey);
    }
    else {
        socket->write(QByteArrayLiteral("ERROR:UNKNOWN ACTION\r\n\r\n"));
        return;
    }
}

void ChatDeviceController::onClose()
{
    qDebug() << "Closing on user: " << myUsername;
    chatController->closeUser(myUsername);
}

void ChatDeviceController::onConnect(const QByteArray & username, const QByteArray & publicKey)
{
    chatController->connectingUsers(myUsername, username, publicKey);
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

