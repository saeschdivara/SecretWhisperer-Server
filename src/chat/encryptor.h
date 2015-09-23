#ifndef ENCRYPTOR_H
#define ENCRYPTOR_H

#include <QObject>

class Encryptor : public QObject
{
    Q_OBJECT
public:
    explicit Encryptor(QObject *parent = 0);

    QByteArray createRandomString();
    QByteArray encryptAsymmetricly(QByteArray & publicKey, QByteArray & data);
};

#endif // ENCRYPTOR_H
