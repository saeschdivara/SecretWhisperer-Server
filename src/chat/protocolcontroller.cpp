#include "protocolcontroller.h"

ProtocolController::ProtocolController(QObject *parent) : QObject(parent)
{

}

void ProtocolController::onServerDataEvent(QByteArray &data)
{
    const QByteArray seperator("\r\n");

    if ( data.indexOf("SEND:") == 0 ) {
        QByteArray preparedData = prepareRequest(data, QByteArrayLiteral("SEND:"));

        if ( preparedData.size() > 0 ) {

            int seperatorIndex = preparedData.indexOf(seperator);

            QByteArray username = preparedData.left(seperatorIndex);
            QByteArray message = preparedData.mid(seperatorIndex + seperator.length());

            signalSend(username, message);
        }
    }
    else if ( data.indexOf("CONNECT:") == 0 ) {
        QByteArray preparedData = prepareRequest(data, QByteArrayLiteral("CONNECT:"));

        if ( preparedData.size() > 0 ) {

            int seperatorIndex = preparedData.indexOf(seperator);

            QByteArray username = preparedData.left(seperatorIndex);
            QByteArray publicKey = preparedData.mid(seperatorIndex + seperator.length());

            signalConnect(username, publicKey);
        }
    }
    else if ( data.indexOf("ENCRYPT:") == 0 ) {
        QByteArray preparedData = prepareRequest(data, QByteArrayLiteral("ENCRYPT:"));

        if ( preparedData.size() > 0 ) {

            int seperatorIndex = preparedData.indexOf(seperator);

            QByteArray username = preparedData.left(seperatorIndex);
            QByteArray message = preparedData.mid(seperatorIndex + seperator.length());

            signalEncrypt(username, message);
        }
    }
    else {
        signalError(QByteArrayLiteral("UNKNOWN ACTION"));
    }
}

QByteArray ProtocolController::prepareRequest(QByteArray &data, QByteArray command)
{
    QByteArray messageData = stripRequest(data, command);
    const QByteArray seperator("\r\n");

    int seperatorIndex = messageData.indexOf(seperator);

    if ( seperatorIndex == -1 ) {
        signalError(QByteArrayLiteral("MISSING SEPERATOR"));
        return QByteArray();
    }

    if ( seperatorIndex == 0 ) {
        signalError(QByteArrayLiteral("MISSING USER"));
        return QByteArray();
    }

    return messageData;
}

QByteArray ProtocolController::stripRequest(QByteArray data, QByteArray command)
{
    const QByteArray endLiteral("\r\n\r\n");

    if ( data.indexOf(command) != 0 ) {
        signalError(QByteArrayLiteral("UNKNOWN COMMAND"));
        return QByteArray();
    }

    data = data.remove(0, command.length());

    int endIndex = data.indexOf(endLiteral);
    if ( endIndex == -1 ) {
        signalError(QByteArrayLiteral("NO END"));
        return QByteArray();
    }

    // Get user name
    data = data.remove(endIndex, endLiteral.length());

    return data;
}
