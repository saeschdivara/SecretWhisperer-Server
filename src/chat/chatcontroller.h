#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <QtCore/qglobal.h>
#include <QtCore/QQueue>
#include <QtNetwork/QTcpServer>

class ChatController : public QTcpServer
{
    Q_OBJECT
public:
    explicit ChatController(QObject *parent = 0);

signals:

public slots:
    void ready();
    void onError(QAbstractSocket::SocketError error);

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    QQueue<QTcpSocket *> pendingSockets;
};

#endif // CHATCONTROLLER_H
