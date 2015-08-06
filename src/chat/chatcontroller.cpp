#include "chatcontroller.h"

#include <QtCore/QFile>

#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslCipher>
#include <QtNetwork/QSslKey>
#include <QtNetwork/QSslSocket>

ChatController::ChatController(QObject *parent) : QTcpServer(parent)
{

}

void ChatController::ready()
{
    qDebug() << "On connection ready";
    QTcpSocket * socket = pendingSockets.dequeue();
    qDebug() << "Socket: " << socket;

    qDebug() << socket->readAll();

    //socket->close();
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

