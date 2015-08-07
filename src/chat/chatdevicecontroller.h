#ifndef CHATDEVICECONTROLLER_H
#define CHATDEVICECONTROLLER_H

#include <QtCore/QObject>
#include <QtCore/qglobal.h>

#include <QtNetwork/QTcpSocket>

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

signals:

public slots:
    void onNewData();

protected:

    // Actions
    void onConnect(QByteArray username);

    // Helper
    QByteArray stripRequest(QByteArray data, QByteArray command);

private:
    QTcpSocket * socket;
    ChatController * chatController;
    QByteArray myUsername;
};

#endif // CHATDEVICECONTROLLER_H
