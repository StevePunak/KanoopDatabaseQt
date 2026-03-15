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

/** @brief Represents a set of database connection credentials for use by the DataSource class. */
class DatabaseCredentials
{
public:
    /** @brief Construct default (empty) credentials. */
    DatabaseCredentials() {}

    /** @brief Construct credentials with all connection parameters.
     *  @param host The database server hostname or IP address.
     *  @param schema The database/schema name.
     *  @param username The database username.
     *  @param password The database password.
     *  @param engine The database engine identifier (e.g. SQLENG_SQLITE, SQLENG_MYSQL).
     */
    DatabaseCredentials(const QString& host, const QString& schema, const QString& username, const QString& password, const QString& engine) :
        _host(host), _schema(schema), _username(username), _password(password), _engine(engine) {}

    /** @brief Construct SQLite credentials with only a schema (file path).
     *  @param schema The SQLite database file path.
     */
    DatabaseCredentials(const QString& schema) :
        _schema(schema), _engine(SQLENG_SQLITE) {}

    /** @brief Get the database server hostname.
     *  @return The hostname or IP address.
     */
    QString host() const { return _host; }
    /** @brief Set the database server hostname.
     *  @param value The hostname or IP address.
     */
    void setHost(const QString& value) { _host = value; }

    /** @brief Get the database/schema name.
     *  @return The schema name or SQLite file path.
     */
    QString schema() const { return _schema; }
    /** @brief Set the database/schema name.
     *  @param value The schema name or SQLite file path.
     */
    void setSchema(const QString& value) { _schema = value; }

    /** @brief Get the database username.
     *  @return The username string.
     */
    QString username() const { return _username; }
    /** @brief Set the database username.
     *  @param value The username.
     */
    void setUsername(const QString& value) { _username = value; }

    /** @brief Get the database password.
     *  @return The password string.
     */
    QString password() const { return _password; }
    /** @brief Set the database password.
     *  @param value The password.
     */
    void setPassword(const QString& value) { _password = value; }

    /** @brief Get the database engine identifier.
     *  @return The engine string (e.g. SQLENG_SQLITE, SQLENG_MYSQL).
     */
    QString engine() const { return _engine; }
    /** @brief Set the database engine identifier.
     *  @param value The engine string (e.g. SQLENG_SQLITE).
     */
    void setEngine(const QString& value) { _engine = value; }

    /** @brief Return true if the engine is SQLite.
     *  @return true if the engine is SQLite.
     */
    bool isSqlite() const { return _engine == SQLENG_SQLITE; }
    /** @brief Return true if the credentials have a non-empty schema.
     *  @return true if a schema has been set.
     */
    bool isValid() const { return _schema.isEmpty() == false; }

    /** @brief Engine identifier string for SQLite. */
    static const QString SQLENG_SQLITE;
    /** @brief Engine identifier string for MySQL. */
    static const QString SQLENG_MYSQL;
    /** @brief Engine identifier string for PostgreSQL. */
    static const QString SQLENG_PGSQL;

private:
    QString _host;
    QString _schema;
    QString _username;
    QString _password;
    QString _engine;
};

#endif // DATABASECREDENTIALS_H
