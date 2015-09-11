#include "chatcontroller.h"

#include <QtCore/QFile>
#include <QtCore/QThread>


ChatController::ChatController(QObject *parent) : QTcpServer(parent)
{
    init();
}

void ChatController::connectingUsers(const QByteArray &sender, const QByteArray &receiver, const QByteArray &publicKey)
{
    // Check if both user are already registered
    if ( deviceList.contains(sender) && deviceList.contains(receiver) ) {
        Connector * connector = deviceList.value(receiver)->getConnector();

        connector->send(
                        QByteArrayLiteral("STARTUP:") +
                        sender +
                        QByteArrayLiteral("\r\n") +
                        publicKey +
                        QByteArrayLiteral("\r\n\r\n")
                    );
    }
}

void ChatController::sendMessageToUserFromUser(const QByteArray &sender,
                                               const QByteArray &receiver,
                                               const QByteArray &message)
{
    // Check if both user are already registered
    if ( deviceList.contains(sender) && deviceList.contains(receiver) ) {

        qDebug() << "send message to user";

        Connector * connector = deviceList.value(receiver)->getConnector();
        connector->send(
                        QByteArrayLiteral("MESSAGE:") +
                        sender +
                        QByteArrayLiteral("\r\n") +
                        message +
                        QByteArrayLiteral("\r\n\r\n")
                    );
    }
}

void ChatController::sendEncryptionKey(const QByteArray &sender,
                                       const QByteArray &receiver,
                                       const QByteArray &encryptedKey)
{
    // Check if both user are already registered
    if ( deviceList.contains(sender) && deviceList.contains(receiver) ) {

        qDebug() << "send encryption key to user";

        Connector * connector = deviceList.value(receiver)->getConnector();
        connector->send(
                        QByteArrayLiteral("ENCRYPT:") +
                        sender +
                        QByteArrayLiteral("\r\n") +
                        encryptedKey +
                        QByteArrayLiteral("\r\n\r\n")
                    );
    }
}

void ChatController::closeUser(const QByteArray &username)
{
    deviceList.value(username)->deleteLater();
    deviceList.remove(username);
}

void ChatController::ready()
{
    const QByteArray userLiteral("USER:");
    const QByteArray endLiteral("\r\n\r\n");

    qDebug() << "On connection ready";
    QTcpSocket * socket = pendingSockets.dequeue();

    QByteArray data = socket->readAll();

    qDebug() << "Disconnect: " << disconnect(socket, SIGNAL(readyRead()), this, SLOT(ready()));

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

    if ( !deviceList.contains(data) ) {
        ChatDeviceController * deviceController = new ChatDeviceController(socket, this, data);
        deviceController->listen();
        deviceList.insert(data, deviceController);
    }
    else {
        socket->write(QByteArrayLiteral("ERROR:WRONG USER\r\n\r\n"));
        socket->close();
        return;
    }
}

void ChatController::onError(QAbstractSocket::SocketError error)
{
    qDebug() << "SOCKET ERROR: " << error;
}

void ChatController::onSslErrors(QList<QSslError> errors)
{
    qDebug() << "SSL ERRORS: " << errors;
}

void ChatController::init()
{
    // Save certificate
    QFile certificateFile("certificates/cert.pem");
    if ( !certificateFile.open(QIODevice::ReadOnly) ) {
        qWarning() << "CERTIFICATE FILE ERROR: Could not open (" << certificateFile.errorString() << ")";
    }

    QByteArray data = certificateFile.readAll();
    certificateFile.close();

    certificate = QSslCertificate(data);

    // Allowed ciphers
    ciphers << QSslCipher("ECDHE-RSA-AES256-GCM-SHA384");

    // Private key

    QFile privateKeyFile("certificates/key.pem");
    if ( !privateKeyFile.open(QIODevice::ReadOnly) ) {
        qWarning() << "PRIVATE KEY FILE ERROR: Could not open (" << privateKeyFile.errorString() << ")";
    }

    data = privateKeyFile.readAll();

    certificateFile.close();

    privateKey = QSslKey(data,
                         QSsl::Rsa,
                         QSsl::Pem,
                         QSsl::PrivateKey,
                         QByteArrayLiteral("1234"));
}

void ChatController::incomingConnection(qintptr socketDescriptor)
{
    QSslSocket *serverSocket = new QSslSocket;

    qDebug() << "New connection";

    if (serverSocket->setSocketDescriptor(socketDescriptor)) {
        connect(serverSocket, SIGNAL(readyRead()), this, SLOT(ready()));

        connect(serverSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(onError(QAbstractSocket::SocketError)));

        connect(serverSocket, SIGNAL(sslErrors(QList<QSslError>)),
                this, SLOT(onSslErrors(QList<QSslError>)));

        qDebug() << "Connected to encrypted signal";

        serverSocket->setPrivateKey(privateKey);
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
