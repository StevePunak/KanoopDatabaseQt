/**
 *  DatabaseCredentials
 *
 *  Represents a set of database credentials for use by the DataSource class
 *
 *  Stephen Punak, January 11 2025
 */
#ifndef DATABASECREDENTIALS_H
#define DATABASECREDENTIALS_H

#include <QString>

class DatabaseCredentials
{
public:
    DatabaseCredentials() {}
    DatabaseCredentials(const QString& host, const QString& schema, const QString& username, const QString& password, const QString& engine) :
        _host(host), _schema(schema), _username(username), _password(password), _engine(engine) {}

    QString host() const { return _host; }
    void setHost(const QString& value) { _host = value; }

    QString schema() const { return _schema; }
    void setSchema(const QString& value) { _schema = value; }

    QString username() const { return _username; }
    void setUsername(const QString& value) { _username = value; }

    QString password() const { return _password; }
    void setPassword(const QString& value) { _password = value; }

    QString engine() const { return _engine; }
    void setEngine(const QString& value) { _engine = value; }

    bool isSqlite() const { return _engine == SQLENG_SQLITE; }
    bool isValid() const { return _schema.isEmpty() == false; }

    static const QString SQLENG_SQLITE;
    static const QString SQLENG_MYSQL;
    static const QString SQLENG_PGSQL;

private:
    QString _host;
    QString _schema;
    QString _username;
    QString _password;
    QString _engine;
};

#endif // DATABASECREDENTIALS_H
