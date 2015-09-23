#include "encryptor.h"

#include <QtCore/QCryptographicHash>

#include <botan/botan.h>
#include <botan/pubkey.h>
#include <botan/pkcs8.h>
#include <botan/pk_keys.h>
#include <botan/rsa.h>

Encryptor::Encryptor(QObject *parent) : QObject(parent)
{

}

QByteArray Encryptor::createRandomString()
{
    QByteArray randomString;

    Botan::AutoSeeded_RNG rng;
    const uint RANDOM_DATA_SIZE = 256;
    Botan::byte randomData[RANDOM_DATA_SIZE];

    rng.randomize(randomData, RANDOM_DATA_SIZE);

    for (int index = 0; index < RANDOM_DATA_SIZE; ++index) {
        randomString.append(randomData[index]);
    }

    return randomString;
}

QByteArray Encryptor::encryptAsymmetricly(QByteArray &publicKey, QByteArray &data)
{
    Botan::DataSource_Memory key_pub(publicKey.toStdString());
    auto publicRsaKey = Botan::X509::load_key(key_pub);

    const uint DATA_SIZE = data.size();
    Botan::byte msgtoencrypt[DATA_SIZE];

    for (uint i = 0; i < DATA_SIZE; i++)
    {
        msgtoencrypt[i] = data[i];
    }

    Botan::PK_Encryptor_EME encryptor(*publicRsaKey, "EME1(SHA-256)");
    Botan::AutoSeeded_RNG rng;
    std::vector<Botan::byte> ciphertext = encryptor.encrypt(msgtoencrypt, DATA_SIZE, rng);

    QByteArray keyCipherData;
    keyCipherData.resize(ciphertext.size());

    for ( uint i = 0; i < ciphertext.size(); i++ ) {
        keyCipherData[i] = ciphertext.at(i);
    }

    return keyCipherData;
}

