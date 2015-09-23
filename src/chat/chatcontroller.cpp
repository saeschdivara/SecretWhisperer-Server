#include "chatcontroller.h"

#include <QtCore/QFile>
#include <QtCore/QThread>


ChatController::ChatController(QObject *parent) : QTcpServer(parent),
    identity(new IdentityController(this)),
    encryptor(new Encryptor(this))
{
    init();
    identity->init();
}

void ChatController::connectingUsers(const QByteArray &sender, const QByteArray &receiver, const QByteArray &publicKey)
{
    // Check if both user are already registered
    if ( deviceList.contains(sender) && deviceList.contains(receiver) ) {
        Connector * connector = deviceList.value(receiver)->getConnector();

        connector->onMessage(QByteArrayLiteral("STARTUP:"), sender, publicKey);
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
        connector->onMessage(
                        QByteArrayLiteral("MESSAGE:"),
                        sender,
                        message
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
        connector->onMessage(
                        QByteArrayLiteral("ENCRYPT:"),
                        sender,
                        encryptedKey
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
    const QByteArray seperatorLiteral("\r\n");
    const QByteArray endLiteral("\r\n\r\n");

    qDebug() << "On connection ready";
    QTcpSocket * socket = pendingSockets.dequeue();

    QSslSocket * sslSocket = qobject_cast<QSslSocket *>(socket);
    qDebug() << sslSocket->sessionCipher();

    QByteArray data = socket->readAll();

    disconnect(socket, SIGNAL(readyRead()), this, SLOT(ready()));

    if ( data.indexOf(userLiteral) != 0 ) {
        socket->write(QByteArrayLiteral("ERROR:NO USER\r\n\r\n"));
        socket->close();
        return;
    }

    data = data.remove(0, userLiteral.length());

    int nameSeperatorIndex = data.indexOf(seperatorLiteral);
    if ( nameSeperatorIndex == -1 ) {
        socket->write(QByteArrayLiteral("ERROR:NO SEPERATOR\r\n\r\n"));
        socket->close();
        return;
    }

    // Get user name
    data = data.remove(nameSeperatorIndex, seperatorLiteral.length());
    QByteArray username = data.left(nameSeperatorIndex);
    data = data.remove(0, username.size());

    if ( username.length() == 0 ) {
        socket->write(QByteArrayLiteral("ERROR:NO USER\r\n\r\n"));
        socket->close();
        return;
    }

    int endIndex = data.indexOf(endLiteral);
    if ( endIndex == -1 ) {
        socket->write(QByteArrayLiteral("ERROR:NO END\r\n\r\n"));
        socket->close();
        return;
    }

    // Get public key
    QByteArray publicKey = data.remove(endIndex, endLiteral.length());

    if ( publicKey.length() == 0 ) {
        socket->write(QByteArrayLiteral("ERROR:NO PUBLIC KEY\r\n\r\n"));
        socket->close();
        return;
    }

    if ( !deviceList.contains(username) ) {

        // Check if the user not exists
        if ( !identity->hasUsername(username) ) {
            qDebug() << "This username (" << username << ") has not been used yet";

            // Register the user
            identity->createIdentity(username, publicKey);

            // Now we listen to the user
            ChatDeviceController * deviceController = new ChatDeviceController(sslSocket, this, data);
            deviceController->listen();
            deviceList.insert(data, deviceController);
        }
        // Username does exists
        else {
            // Check if the public key does not match the registered public key
            if ( !identity->hasIdentity(username, publicKey) ) {
                socket->write(QByteArrayLiteral("ERROR:WRONG IDENTITY\r\n\r\n"));
                socket->close();
                return;
            }
            // Username and public key match
            else {
                connect( sslSocket, &QSslSocket::readyRead, this, &ChatController::onAuthentication );

                QByteArray randomString = encryptor->createRandomString();
                QByteArray randomStringHash = QCryptographicHash::hash(randomString, QCryptographicHash::Sha3_512);
                QByteArray encryptedHash = encryptor->encryptAsymmetricly(publicKey, randomStringHash);

                qDebug() << "Random string: " << randomString;
                qDebug() << "String hash: " << randomStringHash;
                qDebug() << "Encrypted hash: " << encryptedHash;

                notAuthenticatedDevices.insert(sslSocket, QPair<QByteArray, QByteArray>(username, randomStringHash));

                socket->write(QByteArrayLiteral("IDENTITY-CHECK:") + encryptedHash + endLiteral);
            }
        }

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

void ChatController::onAuthentication()
{
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
    ciphers << QSslCipher(QStringLiteral("ECDHE-RSA-AES256-GCM-SHA384"), QSsl::TlsV1_2);

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
