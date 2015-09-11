#include "chatdevicecontroller.h"
#include "chat/chatcontroller.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>

const quint64 MAX_READ_RATE = 1024;

const QByteArray seperator = QByteArrayLiteral("\r\n");

ChatDeviceController::ChatDeviceController(QTcpSocket *socket, ChatController *controller, QByteArray username, QObject *parent) :
    QObject(parent),
    socket(socket),
    chatController(controller),
    myUsername(username),
    connector(new Connector(socket, this))
{

}

void ChatDeviceController::listen()
{
    connect( connector, &Connector::newData, this, &ChatDeviceController::onNewData );
    connect( connector, &Connector::closed, this, &ChatDeviceController::onClose );

    connector->listen();
}

void ChatDeviceController::onNewData(QByteArray & data)
{

    qDebug() << "Received user data";

    if ( data.indexOf("CONNECT:") == 0 ) {
        QByteArray connectStatement = stripRequest(data, "CONNECT:");

        if ( connectStatement.length() == 0 ) {
            connector->send(QByteArrayLiteral("ERROR:NO USER\r\n\r\n"));
            return;
        }

        int seperatorIndex = connectStatement.indexOf(seperator);

        if ( seperatorIndex == -1 ) {
            connector->send(QByteArrayLiteral("ERROR:MISSING SEPERATOR\r\n\r\n"));
            return;
        }

        if ( seperatorIndex == 0 ) {
            connector->send(QByteArrayLiteral("ERROR:MISSING USER\r\n\r\n"));
            return;
        }

        QByteArray username = connectStatement.left(seperatorIndex);
        QByteArray publicKey = connectStatement.mid(seperatorIndex + seperator.length());

        onConnect(username, publicKey);
    }
    else if ( data.indexOf("SEND:") == 0 ) {
        qDebug() << "On Send";
        onSend(data);
    }
    else if ( data.indexOf("ENCRYPT:") == 0 ) {
        qDebug() << "On Encrypt";
        onEncrypt(data);
    }
    else {
        connector->send(QByteArrayLiteral("ERROR:UNKNOWN ACTION\r\n\r\n"));
        return;
    }
}

void ChatDeviceController::onClose()
{
    qDebug() << QStringLiteral("User has disconnected");
    chatController->closeUser(myUsername);
}

void ChatDeviceController::onConnect(const QByteArray & username, const QByteArray & publicKey)
{
    chatController->connectingUsers(myUsername, username, publicKey);
}

void ChatDeviceController::onSend(const QByteArray &data)
{
    QByteArray sendStatement = stripRequest(data, "SEND:");

    if ( sendStatement.length() == 0 ) {
        connector->send(QByteArrayLiteral("ERROR:NO USER\r\n\r\n"));
        return;
    }

    int seperatorIndex = sendStatement.indexOf(seperator);

    if ( seperatorIndex == -1 ) {
        connector->send(QByteArrayLiteral("ERROR:MISSING SEPERATOR\r\n\r\n"));
        return;
    }

    if ( seperatorIndex == 0 ) {
        connector->send(QByteArrayLiteral("ERROR:MISSING USER\r\n\r\n"));
        return;
    }

    QByteArray username = sendStatement.left(seperatorIndex);
    QByteArray message = sendStatement.mid(seperatorIndex + seperator.length());

    chatController->sendMessageToUserFromUser(myUsername, username, message);
}

void ChatDeviceController::onEncrypt(const QByteArray &data)
{
    QByteArray encryptStatement = stripRequest(data, "ENCRYPT:");

    if ( encryptStatement.length() == 0 ) {
        connector->send(QByteArrayLiteral("ERROR:NO USER\r\n\r\n"));
        return;
    }

    int seperatorIndex = encryptStatement.indexOf(seperator);

    if ( seperatorIndex == -1 ) {
        connector->send(QByteArrayLiteral("ERROR:MISSING SEPERATOR\r\n\r\n"));
        return;
    }

    if ( seperatorIndex == 0 ) {
        connector->send(QByteArrayLiteral("ERROR:MISSING USER\r\n\r\n"));
        return;
    }

    QByteArray username = encryptStatement.left(seperatorIndex);
    QByteArray encryptedKey = encryptStatement.mid(seperatorIndex + seperator.length());

    chatController->sendEncryptionKey(myUsername, username, encryptedKey);
}

QByteArray ChatDeviceController::stripRequest(QByteArray data, QByteArray command)
{
    const QByteArray endLiteral("\r\n\r\n");

    if ( data.indexOf(command) != 0 ) {
        connector->send(QByteArrayLiteral("ERROR:UNKNOWN COMMAND\r\n\r\n"));
        socket->close();
        return QByteArrayLiteral("");
    }

    data = data.remove(0, command.length());

    int endIndex = data.indexOf(endLiteral);

    if ( endIndex == -1 ) {
        connector->send(QByteArrayLiteral("ERROR:NO END\r\n\r\n"));
        socket->close();
        return QByteArrayLiteral("");
    }

    data = data.remove(endIndex, endLiteral.length());

    return data;
}

