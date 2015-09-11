#ifndef CHATDEVICECONTROLLER_H
#define CHATDEVICECONTROLLER_H

#include <QtCore/QObject>
#include <QtCore/qglobal.h>

#include <QtNetwork/QTcpSocket>

#include "chat/connector.h"

class ChatController;

class ChatDeviceController : public QObject
{
    Q_OBJECT
public:
    explicit ChatDeviceController(
            QTcpSocket * socket,
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
    void onNewData(QByteArray &data);
    void onClose();

protected:

    // Actions
    void onConnect(const QByteArray &username, const QByteArray &publicKey);
    void onSend(const QByteArray &data);
    void onEncrypt(const QByteArray &data);

    // Helper
    QByteArray stripRequest(QByteArray data, QByteArray command);

private:

    Connector * connector;

    QTcpSocket * socket;
    ChatController * chatController;
    QByteArray myUsername;
};

#endif // CHATDEVICECONTROLLER_H
