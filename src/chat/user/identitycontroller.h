#ifndef IDENTITYCONTROLLER_H
#define IDENTITYCONTROLLER_H

#include <QObject>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlTableModel>

#include "chat/helper/migrationhelper.h"

class IdentityController : public QObject
{
    Q_OBJECT
public:
    explicit IdentityController(QObject *parent = 0);
    void init();

    bool hasIdentity(QByteArray & username, QByteArray & publicKey);
    bool hasUsername(QByteArray & username);
    void createIdentity(QByteArray & username, QByteArray & publicKey);

protected:
    void createDatabaseConnection();
    void createDatabaseTables();

private:
    // Database
    QSqlDatabase database;

    // Helpers
    MigrationHelper * migrations;
};

#endif // IDENTITYCONTROLLER_H
