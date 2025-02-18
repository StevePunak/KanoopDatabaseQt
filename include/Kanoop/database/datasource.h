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

class DataSource : public QObject,
                   public LoggingBaseClass
{
    Q_OBJECT
public:
    explicit DataSource() :
        QObject(),
        LoggingBaseClass("db") {}

    explicit DataSource(const DatabaseCredentials& credentials) :
        QObject(),
        LoggingBaseClass("db"),
        _credentials(credentials) {}

    virtual bool openConnection();
    virtual bool closeConnection();

    bool isOpen() { return _db.isOpen(); }

    DatabaseCredentials credentials() const { return _credentials; }
    void setCredentials(const DatabaseCredentials& value) { _credentials = value; }

    QString connectionName() const { return _connectionName; }
    void setConnectionName(const QString& value) { _connectionName = value; }

    QString errorText() const;

protected:
    QSqlQuery prepareQuery(const QString& sql, bool* success = nullptr);
    QSqlQuery executeQuery(const QString& sql, bool* success = nullptr);
    bool executeQuery(QSqlQuery& query);
    bool querySuccessful(const QSqlQuery& query);

    virtual QString createSql() const { return QString(); }

    void logSql(const char* file, int line, Log::LogLevel level, const QString& sql);
    void logFailure(const QSqlQuery& query) const;
    void setDataSourceError(const QString& value) { _dataSourceError = value; }

    static QDateTime utcTime(const QVariant& value);
    static QString currentTimestamp();

    QSqlDatabase _db;

private:
    void checkExecutingThread() const;
    void recordQueryError(const QSqlQuery& query);
    void createSqliteDatabase();
    bool setSqliteForeignKeyChecking(bool value);

    DatabaseCredentials _credentials;
    QString _connectionName;

    QString _dataSourceError;
    QString _driverError;
    QString _databaseError;
    QString _nativeError;

    int64_t _threadId = 0;
};

#endif // DATASOURCE_H
