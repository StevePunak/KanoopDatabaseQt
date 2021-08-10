#include "datasource.h"
#include <Kanoop/commonexception.h>
#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <QUuid>

bool DataSource::openConnection()
{
    bool result = false;

    try
    {
        _threadId = (int64_t)QThread::currentThreadId();

        if(QSqlDatabase::isDriverAvailable(_credentials.engine()) == false) {
            throw CommonException(QString("SQL Engine %1 is unsupported in this Qt build").arg(_credentials.engine()));
        }

        if(_connectionName.isEmpty()) {
            _connectionName = QUuid::createUuid().toString(QUuid::WithoutBraces);
        }

        _db = QSqlDatabase::addDatabase(_credentials.engine(), _connectionName);
        if(_db.isValid() == false) {
            throw CommonException(QString("Failed to add %1 database").arg(_credentials.engine()));
        }

        if(_credentials.isSqlite() == false) {
            _db.setHostName(_credentials.host());
            _db.setUserName(_credentials.username());
            _db.setPassword(_credentials.password());
        }
        else {
            if(QFile(_credentials.schema()).exists() == false) {
                throw CommonException(QString("Failed to find sqlite database at %1").arg(_credentials.schema()));
            }
        }

        _db.setDatabaseName(_credentials.schema());

        if(_db.open() == false) {
            throw CommonException("Database open failed");
        }

        result = true;
    }
    catch(const CommonException& e)
    {
        logText(LVL_ERROR, QString("DataSource Open Exception: %1 [%2]").arg(e.message()).arg(QSqlError(_db.lastError()).databaseText()));
        result = false;
    }

    return result;
}

bool DataSource::closeConnection()
{
    bool result = false;

    try
    {
        if(_db.isOpen() == false) {
            throw CommonException("Database is not open");
        }

        _db.close();

        QSqlDatabase::removeDatabase(_connectionName);

        result = true;
    }
    catch(const CommonException& e)
    {
        logText(LVL_ERROR, QString("DataSource Open Exception: %1 [%2]").arg(e.message()).arg(QSqlError(_db.lastError()).databaseText()));
        result = false;
    }

    return result;
}

QString DataSource::errorText() const
{
    return QString("%1 (Driver error: %2) (Native error: %3)")
            .arg(_databaseError).arg(_driverError).arg(_nativeError);
}

QSqlQuery DataSource::prepareQuery(const QString& sql, bool* success)
{
    bool result;
    QSqlQuery query(_db);
    if((result = query.prepare(sql)) == false) {
        recordQueryError(query);
        logFailure(query);
    }

    if(success != nullptr) {
        *success = result;
    }
    return query;
}

QSqlQuery DataSource::executeQuery(const QString& sql, bool* success)
{
    bool result;
    QSqlQuery query = prepareQuery(sql, &result);
    if(result) {
        result = executeQuery(query);
    }

    if(success != nullptr) {
        *success = result;
    }

    return query;
}

bool DataSource::executeQuery(QSqlQuery& query)
{
    checkExecutingThread();

    bool result;
    if((result = query.exec()) == false) {
        recordQueryError(query);
        logFailure(query);
    }
    return result;
}

bool DataSource::querySuccessful(const QSqlQuery& query)
{
    checkExecutingThread();

    bool result;
    if((result = query.isActive()) == false) {
        recordQueryError(query);
        logFailure(query);
    }
    return result;
}

void DataSource::logFailure(const QSqlQuery& query) const
{
    logText(LVL_ERROR, QString("QUERY FAILED: %1\nSQL Follows:\n%2")
            .arg(errorText()).arg(query.lastQuery()));
}

QDateTime DataSource::utcTime(const QVariant& value)
{
    QDateTime timestamp = value.toDateTime();
    timestamp.setTimeSpec(Qt::UTC);
    return timestamp;
}

void DataSource::checkExecutingThread() const
{
    if((int64_t)QThread::currentThreadId() != _threadId) {
        logText(LVL_ERROR, QString(
                    "ERROR: Executing thread for database connection %1 is not the thread use to open the connection. "
                    "QSqlDatabase is not thread-safe.")
                .arg(_connectionName));
    }
}

void DataSource::recordQueryError(const QSqlQuery& query)
{
    _driverError = query.lastError().driverText();
    _databaseError = query.lastError().databaseText();
    _nativeError = query.lastError().nativeErrorCode();
}
