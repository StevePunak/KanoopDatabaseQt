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

    void logFailure(const QSqlQuery& query) const;

    static QDateTime utcTime(const QVariant& value);

    QSqlDatabase _db;

private:
    void checkExecutingThread() const;
    void recordQueryError(const QSqlQuery& query);

    DatabaseCredentials _credentials;
    QString _connectionName;

    QString _driverError;
    QString _databaseError;
    QString _nativeError;

    int64_t _threadId = 0;
};

#endif // DATASOURCE_H
