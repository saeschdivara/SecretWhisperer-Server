#ifndef PROTOCOLCONTROLLER_H
#define PROTOCOLCONTROLLER_H

#include <QObject>

class ProtocolController : public QObject
{
    Q_OBJECT
public:
    explicit ProtocolController(QObject *parent = 0);

signals:
    void signalConnect(const QByteArray & username, QByteArray & publicKey);
    void signalEncrypt(const QByteArray & username, QByteArray & message);
    void signalSend(const QByteArray & username, QByteArray & message);
    void signalError(QByteArray);

public slots:
    void onServerDataEvent(QByteArray & data);

protected:
    // Helper methods
    QByteArray prepareRequest(QByteArray & data, QByteArray command);
    QByteArray stripRequest(QByteArray data, QByteArray command);
};

#endif // PROTOCOLCONTROLLER_H
