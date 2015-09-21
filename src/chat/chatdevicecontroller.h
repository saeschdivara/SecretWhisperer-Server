#ifndef CHATDEVICECONTROLLER_H
#define CHATDEVICECONTROLLER_H

#include <QtCore/QObject>
#include <QtCore/qglobal.h>

#include <QtNetwork/QTcpSocket>

#include "chat/connector.h"
#include "chat/protocolcontroller.h"

class ChatController;

class ChatDeviceController : public QObject
{
    Q_OBJECT
public:
    explicit ChatDeviceController(QSslSocket *socket,
            ChatController * controller,
            QByteArray username,
            QObject *parent = 0);
    void listen();

    QTcpSocket * getSocket() {
        return socket;
    }

    Connector * getConnector() {
        return connector;
    }

signals:

public slots:
    void onClose();

protected:

    // Actions
    void onConnect(const QByteArray &username, const QByteArray &publicKey);
    void onSend(const QByteArray &username, QByteArray &message);
    void onEncrypt(const QByteArray &username, QByteArray &encryptedKey);

private:

    Connector * connector;
    ProtocolController * protocol;

    QSslSocket * socket;
    ChatController * chatController;
    QByteArray myUsername;
};

#endif // CHATDEVICECONTROLLER_H
