#include "chatcontroller.h"

#include <QtCore/QFile>

#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslCipher>
#include <QtNetwork/QSslKey>
#include <QtNetwork/QSslSocket>

ChatController::ChatController(QObject *parent) : QTcpServer(parent)
{

}

void ChatController::connectingUsers(const QByteArray &user1, const QByteArray &user2)
{
    // Check if both user are already registered
    if ( deviceList.contains(user1) && deviceList.contains(user2) ) {
        //
    }
}

void ChatController::sendMessageToUserFromUser(const QByteArray &sender,
                                               const QByteArray &receiver,
                                               const QByteArray &message)
{
    // Check if both user are already registered
    if ( deviceList.contains(user1) && deviceList.contains(user2) ) {
        QTcpSocket * receiverSocket = deviceList.value(receiver);
        receiverSocket->write(message);
    }
}

void ChatController::ready()
{
    const QByteArray userLiteral("USER:");
    const QByteArray endLiteral("\r\n\r\n");

    qDebug() << "On connection ready";
    QTcpSocket * socket = pendingSockets.dequeue();

    QByteArray data = socket->readAll();

    disconnect(socket, SIGNAL(readyRead()), this, SLOT(ready()));

    if ( data.indexOf(userLiteral) != 0 ) {
        socket->write(QByteArrayLiteral("ERROR:NO USER\r\n\r\n"));
        socket->close();
        return;
    }

    data = data.remove(0, userLiteral.length());

    int endIndex = data.indexOf(endLiteral);
    if ( endIndex == -1 ) {
        socket->write(QByteArrayLiteral("ERROR:NO END\r\n\r\n"));
        socket->close();
        return;
    }

    // Get user name
    data = data.remove(endIndex, endLiteral.length());

    if ( data.length() == 0 ) {
        socket->write(QByteArrayLiteral("ERROR:NO USER\r\n\r\n"));
        socket->close();
        return;
    }

    ChatDeviceController * deviceController = new ChatDeviceController(socket, this, data);
    deviceList.insert(data, deviceController);
}

void ChatController::onError(QAbstractSocket::SocketError error)
{
    qDebug() << error;
}

void ChatController::incomingConnection(qintptr socketDescriptor)
{
    QSslSocket *serverSocket = new QSslSocket;

    qDebug() << "New connection";

    if (serverSocket->setSocketDescriptor(socketDescriptor)) {
        connect(serverSocket, SIGNAL(readyRead()), this, SLOT(ready()));

        connect(serverSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(onError(QAbstractSocket::SocketError)));

        qDebug() << "Connected to encrypted signal";

        // Ciphers allowed
        QList<QSslCipher> ciphers;

        ciphers << QSslCipher("ECDHE-RSA-AES256-GCM-SHA384");

        serverSocket->setPrivateKey("certificates/key.pem",
                                    QSsl::Rsa,
                                    QSsl::Pem,
                                    QByteArrayLiteral("1234"));

        QFile certificateFile("certificates/cert.pem");
        certificateFile.open(QIODevice::ReadOnly);

        QByteArray data = certificateFile.readAll();

        certificateFile.close();

        QSslCertificate certificate(data);

        serverSocket->setLocalCertificate(certificate);

        // Update config
        QSslConfiguration config = serverSocket->sslConfiguration();
        config.setProtocol(QSsl::TlsV1_2);
        config.setCiphers(ciphers);

        serverSocket->setSslConfiguration(config);

        serverSocket->startServerEncryption();

        pendingSockets.enqueue(serverSocket);
    } else {
        delete serverSocket;
    }
}
