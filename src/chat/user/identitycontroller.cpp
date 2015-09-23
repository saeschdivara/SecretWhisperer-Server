#include "identitycontroller.h"

#include <QtCore/QDebug>
#include <QtSql/QSqlError>
#include <QtSql/QSqlField>
#include <QtSql/QSqlQuery>

#include "chat/helper/sql.h"

IdentityController::IdentityController(QObject *parent) : QObject(parent)
{

}

void IdentityController::init()
{
    createDatabaseConnection();
    createDatabaseTables();
}

bool IdentityController::hasIdentity(QByteArray &username, QByteArray &publicKey)
{
    QSqlQuery query("SELECT * FROM users WHERE  CAST(user_name AS BLOB) = CAST(? AS BLOB) AND CAST(public_key AS BLOB) = CAST(? AS BLOB)", database);
    query.addBindValue(username);
    query.addBindValue(publicKey);
    query.exec();

    return query.next();
}

bool IdentityController::hasUsername(QByteArray &username)
{
    QSqlQuery query(database);
    query.prepare(QString("SELECT * FROM users WHERE CAST(user_name AS BLOB) = CAST(:username AS BLOB)"));
    query.bindValue(":username", QVariant(username).toString(), QSql::InOut);
    query.exec();

    bool hasNext = query.next();
    return hasNext;
}

void IdentityController::createIdentity(QByteArray &username, QByteArray &publicKey)
{
    QSqlQuery query("INSERT INTO users (user_name, public_key) VALUES(?, ?)", database);
    query.addBindValue(username);
    query.addBindValue(publicKey);

    query.exec();
}

void IdentityController::createDatabaseConnection()
{
    if ( !QSqlDatabase::contains(QStringLiteral("CHAT-DB")) ) {
        database = QSqlDatabase::addDatabase("QSQLITE", QStringLiteral("CHAT-DB"));
        database.setDatabaseName("data.db");
        database.open();
    }
    else {
        database = QSqlDatabase::database(QStringLiteral("CHAT-DB"));
    }

    migrations = new MigrationHelper(database, this);
}

void IdentityController::createDatabaseTables()
{
    loadTable(database, QString("users"));

    // Create migrations
    if ( !migrations->hasMigrationsTable() ) {
        // Create table
        migrations->createMigrationsTable();
    }

    migrations->executeMigrations();
}
