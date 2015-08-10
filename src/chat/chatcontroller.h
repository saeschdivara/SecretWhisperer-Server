#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <QtCore/qglobal.h>
#include <QtCore/QHash>
#include <QtCore/QQueue>
#include <QtNetwork/QTcpServer>

#include "chat/chatdevicecontroller.h"

class ChatController : public QTcpServer
{
    Q_OBJECT
public:
    explicit ChatController(QObject *parent = 0);

    void connectingUsers(const QByteArray & sender,
                         const QByteArray & receiver,
                         const QByteArray &publicKey);

    void sendMessageToUserFromUser(const QByteArray & sender,
                                   const QByteArray & receiver,
                                   const QByteArray & message);

    void sendEncryptionKey(const QByteArray & sender,
                                   const QByteArray & receiver,
                                   const QByteArray & encryptedKey);

signals:

public slots:
    void ready();
    void onError(QAbstractSocket::SocketError error);

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    QQueue<QTcpSocket *> pendingSockets;
    QHash<QByteArray, ChatDeviceController *> deviceList;
};

#endif // CHATCONTROLLER_H
