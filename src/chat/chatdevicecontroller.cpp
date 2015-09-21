#include "chatdevicecontroller.h"
#include "chat/chatcontroller.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>

ChatDeviceController::ChatDeviceController(QSslSocket *socket, ChatController *controller, QByteArray username, QObject *parent) :
    QObject(parent),
    socket(socket),
    chatController(controller),
    myUsername(username),
    connector(new Connector(socket, this)),
    protocol(new ProtocolController(this))
{

}

void ChatDeviceController::listen()
{
    connect( connector, &Connector::newData, protocol, &ProtocolController::onServerDataEvent );
    connect( protocol, &ProtocolController::signalConnect, this, &ChatDeviceController::onConnect );
    connect( protocol, &ProtocolController::signalEncrypt, this, &ChatDeviceController::onEncrypt );
    connect( protocol, &ProtocolController::signalSend, this, &ChatDeviceController::onSend );

    //connect( connector, &Connector::newData, this, &ChatDeviceController::onNewData );
    connect( connector, &Connector::closed, this, &ChatDeviceController::onClose );

    connector->listen();
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

void ChatDeviceController::onSend(const QByteArray &username, QByteArray & message)
{
    chatController->sendMessageToUserFromUser(myUsername, username, message);
}

void ChatDeviceController::onEncrypt(const QByteArray & username, QByteArray & encryptedKey)
{
    chatController->sendEncryptionKey(myUsername, username, encryptedKey);
}

