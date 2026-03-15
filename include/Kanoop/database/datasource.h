/**
 *  DataSource
 *
 *  An abstraction model for databases.
 *
 *  Subclass this class to provide a Controller in the MVC programming paradigm.
 *
 *  Stephen Punak, January 11 2025
 */
#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <Kanoop/utility/loggingbaseclass.h>
#include <Kanoop/database/databasecredentials.h>
#include <QSqlDatabase>

/** @brief Abstract database access layer providing connection management, query execution, and utility methods.
 *
 *  Subclass this class to provide a Controller in the MVC programming paradigm.
 */
class DataSource : public QObject,
                   public LoggingBaseClass
{
    Q_OBJECT
public:
    /** @brief Construct a DataSource with default (empty) credentials. */
    explicit DataSource() :
        QObject(),
        LoggingBaseClass("db") {}

    /** @brief Construct a DataSource with the given database credentials.
     *  @param credentials The database connection credentials.
     */
    explicit DataSource(const DatabaseCredentials& credentials) :
        QObject(),
        LoggingBaseClass("db"),
        _credentials(credentials) {}

    /** @brief Destructor. Closes the database connection if open. */
    virtual ~DataSource();

    /** @brief Open a database connection using the stored credentials.
     *  @return true if the connection was opened successfully.
     */
    virtual bool openConnection();

    /** @brief Close the current database connection.
     *  @return true if the connection was closed successfully.
     */
    virtual bool closeConnection();

    /** @brief Return true if the database connection is currently open.
     *  @return true if connected.
     */
    bool isOpen() { return _db.isOpen(); }

    /** @brief Get the current database credentials.
     *  @return The stored DatabaseCredentials.
     */
    DatabaseCredentials credentials() const { return _credentials; }
    /** @brief Set the database credentials.
     *  @param value The new credentials.
     */
    void setCredentials(const DatabaseCredentials& value) { _credentials = value; }

    /** @brief Get the Qt connection name for this data source.
     *  @return The connection name string.
     */
    QString connectionName() const { return _connectionName; }
    /** @brief Set the Qt connection name.
     *  @param value The connection name string.
     */
    void setConnectionName(const QString& value) { _connectionName = value; }

    /** @brief Return true if the database should be created when opening fails.
     *  @return true if auto-creation on failure is enabled.
     */
    bool createOnOpenFailure() const { return _createOnOpenFailure; }
    /** @brief Set whether to create the database when opening fails.
     *  @param value true to auto-create on failure.
     */
    void setCreateOnOpenFailure(bool value) { _createOnOpenFailure = value; }

    /** @brief Get a human-readable string describing the last error.
     *  @return The error description string.
     */
    QString errorText() const;

    /** @brief Check whether the given file is a valid SQLite database.
     *  @param filename The file path to check.
     *  @return true if the file is a SQLite database.
     */
    static bool isSqlite(const QString& filename);

protected:
    /** @brief Prepare a QSqlQuery from the given SQL string.
     *  @param sql The SQL statement to prepare.
     *  @param success Optional pointer set to true on success, false on failure.
     *  @return The prepared QSqlQuery.
     */
    QSqlQuery prepareQuery(const QString& sql, bool* success = nullptr);

    /** @brief Execute a SQL string and return the resulting query.
     *  @param sql The SQL statement to execute.
     *  @param success Optional pointer set to true on success, false on failure.
     *  @return The executed QSqlQuery.
     */
    QSqlQuery executeQuery(const QString& sql, bool* success = nullptr);

    /** @brief Execute an already-prepared QSqlQuery.
     *  @param query The query to execute.
     *  @return true if execution succeeded.
     */
    bool executeQuery(QSqlQuery& query);

    /** @brief Check whether a query completed without error.
     *  @param query The query to check.
     *  @return true if the query was successful.
     */
    bool querySuccessful(const QSqlQuery& query);

    /** @brief Execute multiple SQL statements in sequence.
     *  @param queries The list of SQL statements to execute.
     *  @return true if all statements executed successfully.
     */
    bool executeMultiple(const QStringList& queries);

    /** @brief Return the SQL used to create the database schema. Override in subclasses.
     *  @return The SQL creation string, or an empty string by default.
     */
    virtual QString createSql() const { return QString(); }

    /** @brief Perform any necessary database migrations. Override in subclasses.
     *  @return true on success.
     */
    virtual bool migrate() { return true; }

    /** @brief Run an integrity check on the database. Override in subclasses.
     *  @return true if the database passes the integrity check.
     */
    virtual bool integrityCheck() { return true; }

    /** @brief Log a SQL statement at the given log level.
     *  @param file The source file name (use __FILE__).
     *  @param line The source line number (use __LINE__).
     *  @param level The log level.
     *  @param sql The SQL statement to log.
     */
    void logSql(const char* file, int line, Log::LogLevel level, const QString& sql);

    /** @brief Log details of a failed query.
     *  @param query The failed QSqlQuery.
     */
    void logFailure(const QSqlQuery& query) const;

    /** @brief Set the data source error string.
     *  @param value The error description.
     */
    void setDataSourceError(const QString& value) { _dataSourceError = value; }

    /** @brief Delete and recreate the SQLite database file from createSql().
     *  @return true on success.
     */
    bool recreateSqliteDatabase();

    /** @brief Execute any post-creation scripts after a new database is created. Override in subclasses.
     *  @return true on success.
     */
    virtual bool executePostCreateScripts() { return true; }

    /** @brief Return true if the current credentials specify a SQLite engine.
     *  @return true if the engine is SQLite.
     */
    bool isSqlite() const { return _credentials.isSqlite(); }

    /** @brief Convert a QVariant database value to a UTC QDateTime.
     *  @param value The QVariant containing the datetime value.
     *  @return The corresponding QDateTime in UTC.
     */
    static QDateTime utcTime(const QVariant& value);

    /** @brief Get the current UTC timestamp as a formatted string suitable for SQL.
     *  @return The formatted timestamp string.
     */
    static QString currentTimestamp();

    /** @brief The underlying QSqlDatabase connection object. */
    QSqlDatabase _db;

    /** @brief Build a comma-delimited string from a list of values.
     *  @param list The list of values to join.
     *  @return A comma-separated string representation.
     */
    template <typename T>
    static QString commaDelimitedList(const QList<T> &list);

    /** @brief Build a comma-delimited string from a list of integers.
     *  @param list The list of integers.
     *  @return A comma-separated string of integers.
     */
    static QString commaDelimitedIntList(const QList<int> &list);

    /** @brief Build a comma-delimited string from a list of UUIDs.
     *  @param list The list of QUuid values.
     *  @return A comma-separated string of UUIDs.
     */
    static QString commaDelimitedUuidList(const QList<QUuid>& list);

    /** @brief Build a comma-delimited string from a list of strings.
     *  @param list The string list.
     *  @return A comma-separated, quoted string.
     */
    static QString commaDelimitedStringList(const QStringList& list);

    /** @brief Escape special characters in a string for safe inclusion in SQL.
     *  @param unescaped The raw string.
     *  @return The escaped string.
     */
    static QString escapedString(const QString& unescaped);


private:
    bool checkExecutingThread() const;
    void recordQueryError(const QSqlQuery& query);
    void createSqliteDatabase();
    bool setSqliteForeignKeyChecking(bool value);

    DatabaseCredentials _credentials;
    QString _connectionName;

    bool _createOnOpenFailure = true;

    QString _dataSourceError;
    QString _driverError;
    QString _databaseError;
    QString _nativeError;

    int64_t _threadId = 0;
};

#endif // DATASOURCE_H
