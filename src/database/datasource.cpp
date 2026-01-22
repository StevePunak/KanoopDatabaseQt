#include "datasource.h"
#include "sqlparser.h"
#include <Kanoop/commonexception.h>
#include <Kanoop/datetimeutil.h>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <QUuid>

DataSource::~DataSource()
{
    if(isOpen()) {
        DataSource::closeConnection();
    }
}

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
            // Special initialization for sqlite
            QFileInfo fileInfo(_credentials.schema());
            if(fileInfo.absoluteDir().exists() == false && QDir().mkpath(fileInfo.absolutePath()) == false) {
                throw CommonException(QString("Failed to create path '%1'").arg(fileInfo.absolutePath()));
            }

            if(fileInfo.exists() == false) {
                if(_createOnOpenFailure == true) {
                    createSqliteDatabase();
                }
                else {
                    throw CommonException("File not found and create disabled");
                }
            }
        }

        _db.setDatabaseName(_credentials.schema());

        if(_db.isOpen() == false && _db.open() == false) {
            throw CommonException("Database open failed");
        }

        if(_credentials.isSqlite()) {
            // sqlite does not enable foreign key checking by default
            setSqliteForeignKeyChecking(true);
        }

        // Let sub-class perform migration
        if(migrate() == false) {
            throw CommonException("Database migration failed");
        }

        if(integrityCheck() == false) {
            throw CommonException("Database integrity check failed");
        }

        result = true;
    }
    catch(const CommonException& e)
    {
        logText(LVL_ERROR, QString("DataSource Open Exception: %1 [%2]").arg(e.message()).arg(QSqlError(_db.lastError()).databaseText()));
        QSqlDatabase::removeDatabase(_connectionName);
        result = false;
    }

    return result;
}

bool DataSource::closeConnection()
{
    bool result = false;

    if(_db.isOpen() == false) {
        _db.close();
        QSqlDatabase::removeDatabase(_connectionName);
        result = true;
    }

    return result;
}

QString DataSource::errorText() const
{
    QString result;
    QTextStream output(&result);
    if(_dataSourceError.isEmpty() == false) {
        output << "(Data Source Error: " << _dataSourceError << ") ";
    }
    if(_databaseError.isEmpty() == false) {
        output << "(DB Error: " << _databaseError << ") ";
    }
    if(_driverError.isEmpty() == false) {
        output << "(Driver Error: " << _driverError << ") ";
    }
    if(_nativeError.isEmpty() == false) {
        output << "(Native Error: " << _nativeError << ") ";
    }
    return result;
}

bool DataSource::isSqlite(const QString& filename)
{
    if (!QFile::exists(filename)) {
        return false;
    }

    QString connectionName = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(filename);

    if (!db.open()) {
        QSqlDatabase::removeDatabase(connectionName);
        return false;
    }

    // Try a simple query to check for validity
    QSqlQuery query(db);
    if (!query.exec("PRAGMA integrity_check;")) {
        db.close();
        QSqlDatabase::removeDatabase(connectionName);
        return false;
    }

    db.close();
    QSqlDatabase::removeDatabase(connectionName);
    return true;
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
    bool result;
    if((result = checkExecutingThread()) == true) {
        if((result = query.exec()) == false) {
            recordQueryError(query);
            logFailure(query);
        }
    }
    return result;
}

bool DataSource::querySuccessful(const QSqlQuery& query)
{
    bool result;
    if((result = checkExecutingThread()) == true) {
        if((result = query.isActive()) == false) {
            recordQueryError(query);
            logFailure(query);
        }
    }
    return result;
}

bool DataSource::executeMultiple(const QStringList& queries)
{
    bool result = false;
    for(const QString& statement : queries) {
        executeQuery(statement, &result);
        if(result == false) {
            break;
        }
    }
    return result;
}

void DataSource::logSql(const char* file, int line, Log::LogLevel level, const QString& sql)
{
    logText(file, line, level, QString("\n%1").arg(sql));
}

void DataSource::logFailure(const QSqlQuery& query) const
{
    logText(LVL_ERROR, QString("QUERY FAILED: %1\nSQL Follows:\n%2")
            .arg(errorText()).arg(query.lastQuery()));
}

bool DataSource::recreateSqliteDatabase()
{
    if(_db.isOpen()) {
        _db.close();
    }

    try
    {
        createSqliteDatabase();
    }
    catch(const CommonException& e)
    {
        logText(LVL_ERROR, QString("Database recreation failed: %1").arg(e.message()));
        return false;
    }
    return true;
}

QDateTime DataSource::utcTime(const QVariant& value)
{
    QDateTime timestamp = value.toDateTime();
    timestamp.setTimeZone(QTimeZone::utc());
    return timestamp;
}

QString DataSource::currentTimestamp()
{
    return DateTimeUtil::currentToStandardString();
}

QString DataSource::commaDelimitedIntList(const QList<int>& list)
{
    QString result;
    QTextStream output(&result);
    for(int i = 0;i < list.length();i++) {
        output << '\''
               << list.at(i)
               << '\'';
        if(i < list.length() - 1) {
            output << ',';
        }
    }
    return result;
}

QString DataSource::commaDelimitedUuidList(const QList<QUuid>& list)
{
    QString result;
    QTextStream output(&result);
    for(int i = 0;i < list.length();i++) {
        output << '\''
               << list.at(i).toString(QUuid::WithoutBraces)
               << '\'';
        if(i < list.length() - 1) {
            output << ',';
        }
    }
    return result;
}

QString DataSource::commaDelimitedStringList(const QStringList& list)
{
    QString result;
    QTextStream output(&result);
    for(int i = 0;i < list.length();i++) {
        output << '\''
               << list.at(i)
               << '\'';
        if(i < list.length() - 1) {
            output << ',';
        }
    }
    return result;
}

QString DataSource::escapedString(const QString& unescaped)
{
    QString utfString = unescaped.toLocal8Bit();
    QString result;
    QTextStream output(&result);

    for(int i = 0;i < utfString.length();i++) {
        QChar thisChar = utfString.at(i).toLatin1();
        if(thisChar == '\'') {
            output << '\'';
            output << thisChar;
        }
        else if(thisChar == 0) {
            output << '?';
        }
        else {
            output << thisChar;
        }
    }
    return result;
}

bool DataSource::checkExecutingThread() const
{
    bool result = (int64_t)QThread::currentThreadId() == _threadId;
    if(result == false) {
        logText(LVL_ERROR, QString(
                    "ERROR: Executing thread for database connection %1 is not the thread use to open the connection. "
                    "QSqlDatabase is not thread-safe.")
                .arg(_connectionName));
    }
    return result;
}

void DataSource::recordQueryError(const QSqlQuery& query)
{
    _driverError = query.lastError().driverText();
    _databaseError = query.lastError().databaseText();
    _nativeError = query.lastError().nativeErrorCode();
}

void DataSource::createSqliteDatabase()
{
    _db.setDatabaseName(_credentials.schema());
    if(_db.open() == false) {
        throw CommonException("Failed to open");
    }

    QString sql = createSql();
    if(sql.isEmpty()) {
        throw CommonException("No createSql() implemented for dynamic creation");
    }

    SqlParser parser(sql);
    if(parser.isValid() == false) {
        throw CommonException(QString("Failed to parse create SQL\n%1").arg(sql));
    }

    if(executeMultiple(parser.statements()) == false) {
        throw CommonException("Failed to execute one or more create queries");
    }

    if(executePostCreateScripts() == false) {
        throw CommonException("Failed to execute post create scripts");
    }
}

bool DataSource::setSqliteForeignKeyChecking(bool value)
{
    QString sql = QString("PRAGMA foreign_keys = %1;").arg(value ? "ON" : "OFF");
    bool result;
    executeQuery(sql, &result);
    return result;
}

template<typename T>
QString DataSource::commaDelimitedList(const QList<T>& list)
{
    QString result;
    QTextStream output(&result);
    for(int i = 0;i < list.length();i++) {
        output << '\''
               << list.at(i)
               << '\'';
        if(i < list.length() - 1) {
            output << ',';
        }
    }
    return result;
}

// Instantiations to allow in-source templates
template QString DataSource::commaDelimitedList(const QList<uint8_t>&);
template QString DataSource::commaDelimitedList(const QList<int>&);
template QString DataSource::commaDelimitedList(const QList<uint32_t>&);
template QString DataSource::commaDelimitedList(const QList<uint64_t>&);
