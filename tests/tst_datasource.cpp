#include <QTest>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <QSqlQuery>
#include <Kanoop/database/datasource.h>

// Concrete subclass for testing protected members
class TestDataSource : public DataSource
{
public:
    TestDataSource() : DataSource() {}
    TestDataSource(const DatabaseCredentials& creds) : DataSource(creds) {}

    // Expose protected methods for testing
    using DataSource::prepareQuery;
    using DataSource::executeQuery;
    using DataSource::querySuccessful;
    using DataSource::executeMultiple;
    using DataSource::escapedString;
    using DataSource::commaDelimitedIntList;
    using DataSource::commaDelimitedUuidList;
    using DataSource::commaDelimitedStringList;
    using DataSource::utcTime;
    using DataSource::currentTimestamp;
    using DataSource::_db;

    QString testCreateSql;

protected:
    QString createSql() const override { return testCreateSql; }
};

class TstDataSource : public QObject
{
    Q_OBJECT

private slots:
    void defaultConstructor_notOpen()
    {
        TestDataSource ds;
        QVERIFY(!ds.isOpen());
        QVERIFY(ds.connectionName().isEmpty());
    }

    void credentialsConstructor_storesCredentials()
    {
        DatabaseCredentials creds("/tmp/test.db");
        TestDataSource ds(creds);
        QCOMPARE(ds.credentials().schema(), QStringLiteral("/tmp/test.db"));
        QVERIFY(ds.credentials().isSqlite());
    }

    void setCredentials_works()
    {
        TestDataSource ds;
        DatabaseCredentials creds("host", "db", "user", "pass", DatabaseCredentials::SQLENG_MYSQL);
        ds.setCredentials(creds);
        QCOMPARE(ds.credentials().host(), QStringLiteral("host"));
        QCOMPARE(ds.credentials().schema(), QStringLiteral("db"));
    }

    void setConnectionName_works()
    {
        TestDataSource ds;
        ds.setConnectionName("test-conn");
        QCOMPARE(ds.connectionName(), QStringLiteral("test-conn"));
    }

    void createOnOpenFailure_defaultTrue()
    {
        TestDataSource ds;
        QVERIFY(ds.createOnOpenFailure());
    }

    void setCreateOnOpenFailure_works()
    {
        TestDataSource ds;
        ds.setCreateOnOpenFailure(false);
        QVERIFY(!ds.createOnOpenFailure());
    }

    void openConnection_sqlite_createsDatabase()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());

        QString dbPath = tmpDir.path() + "/test.db";
        DatabaseCredentials creds(dbPath);
        TestDataSource ds(creds);
        ds.testCreateSql = "CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT);";

        QVERIFY(ds.openConnection());
        QVERIFY(ds.isOpen());
        QVERIFY(QFile::exists(dbPath));

        ds.closeConnection();
        QVERIFY(!ds.isOpen());
    }

    void openConnection_sqlite_autoGeneratesConnectionName()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());

        QString dbPath = tmpDir.path() + "/test.db";
        DatabaseCredentials creds(dbPath);
        TestDataSource ds(creds);
        ds.testCreateSql = "CREATE TABLE test (id INTEGER PRIMARY KEY);";

        QVERIFY(ds.openConnection());
        QVERIFY(!ds.connectionName().isEmpty());
        ds.closeConnection();
    }

    void openConnection_existingDb_opens()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());
        QString dbPath = tmpDir.path() + "/existing.db";

        // Create the database first
        {
            DatabaseCredentials creds1(dbPath);
            TestDataSource ds1(creds1);
            ds1.testCreateSql = "CREATE TABLE test (id INTEGER PRIMARY KEY);";
            QVERIFY(ds1.openConnection());
            ds1.closeConnection();
        }

        // Open existing database
        {
            DatabaseCredentials creds2(dbPath);
            TestDataSource ds2(creds2);
            QVERIFY(ds2.openConnection());
            QVERIFY(ds2.isOpen());
            ds2.closeConnection();
        }
    }

    void closeConnection_whenNotOpen_returnsFalse()
    {
        TestDataSource ds;
        QVERIFY(!ds.closeConnection());
    }

    void executeQuery_validSql_succeeds()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());
        QString dbPath = tmpDir.path() + "/query_test.db";

        DatabaseCredentials creds(dbPath);
        TestDataSource ds(creds);
        ds.testCreateSql = "CREATE TABLE items (id INTEGER PRIMARY KEY, value TEXT);";
        QVERIFY(ds.openConnection());

        bool success = false;
        ds.executeQuery("INSERT INTO items (id, value) VALUES (1, 'hello')", &success);
        QVERIFY(success);

        QSqlQuery query = ds.executeQuery("SELECT value FROM items WHERE id = 1", &success);
        QVERIFY(success);
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("hello"));

        ds.closeConnection();
    }

    void executeQuery_invalidSql_fails()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());
        QString dbPath = tmpDir.path() + "/bad_query.db";

        DatabaseCredentials creds(dbPath);
        TestDataSource ds(creds);
        ds.testCreateSql = "CREATE TABLE items (id INTEGER PRIMARY KEY);";
        QVERIFY(ds.openConnection());

        bool success = true;
        ds.executeQuery("SELECT * FROM nonexistent_table", &success);
        QVERIFY(!success);

        ds.closeConnection();
    }

    void executeMultiple_allSucceed()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());
        QString dbPath = tmpDir.path() + "/multi.db";

        DatabaseCredentials creds(dbPath);
        TestDataSource ds(creds);
        ds.testCreateSql = "CREATE TABLE items (id INTEGER PRIMARY KEY, value TEXT);";
        QVERIFY(ds.openConnection());

        QStringList queries = {
            "INSERT INTO items (id, value) VALUES (1, 'a');",
            "INSERT INTO items (id, value) VALUES (2, 'b');",
            "INSERT INTO items (id, value) VALUES (3, 'c');"
        };
        QVERIFY(ds.executeMultiple(queries));

        bool success = false;
        QSqlQuery query = ds.executeQuery("SELECT COUNT(*) FROM items", &success);
        QVERIFY(success);
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 3);

        ds.closeConnection();
    }

    void executeMultiple_failsOnBadQuery()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());
        QString dbPath = tmpDir.path() + "/multi_fail.db";

        DatabaseCredentials creds(dbPath);
        TestDataSource ds(creds);
        ds.testCreateSql = "CREATE TABLE items (id INTEGER PRIMARY KEY);";
        QVERIFY(ds.openConnection());

        QStringList queries = {
            "INSERT INTO items (id) VALUES (1);",
            "INSERT INTO nonexistent (id) VALUES (2);",
            "INSERT INTO items (id) VALUES (3);"
        };
        QVERIFY(!ds.executeMultiple(queries));

        ds.closeConnection();
    }

    void prepareQuery_validSql_succeeds()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());
        QString dbPath = tmpDir.path() + "/prepare.db";

        DatabaseCredentials creds(dbPath);
        TestDataSource ds(creds);
        ds.testCreateSql = "CREATE TABLE items (id INTEGER PRIMARY KEY, value TEXT);";
        QVERIFY(ds.openConnection());

        bool success = false;
        QSqlQuery query = ds.prepareQuery("INSERT INTO items (id, value) VALUES (?, ?)", &success);
        QVERIFY(success);

        query.addBindValue(1);
        query.addBindValue("test");
        QVERIFY(ds.executeQuery(query));

        QSqlQuery selectQuery = ds.executeQuery("SELECT value FROM items WHERE id = 1", &success);
        QVERIFY(success);
        QVERIFY(selectQuery.next());
        QCOMPARE(selectQuery.value(0).toString(), QStringLiteral("test"));

        ds.closeConnection();
    }

    void escapedString_singleQuotes_doubled()
    {
        QCOMPARE(TestDataSource::escapedString("it's"), QStringLiteral("it''s"));
    }

    void escapedString_noSpecialChars_unchanged()
    {
        QCOMPARE(TestDataSource::escapedString("hello world"), QStringLiteral("hello world"));
    }

    void escapedString_emptyString_returnsEmpty()
    {
        QCOMPARE(TestDataSource::escapedString(""), QStringLiteral(""));
    }

    void escapedString_multipleSingleQuotes()
    {
        QCOMPARE(TestDataSource::escapedString("it's a 'test'"), QStringLiteral("it''s a ''test''"));
    }

    void commaDelimitedIntList_multipleInts()
    {
        QList<int> list = {1, 2, 3};
        QCOMPARE(TestDataSource::commaDelimitedIntList(list), QStringLiteral("1,2,3"));
    }

    void commaDelimitedIntList_singleInt()
    {
        QList<int> list = {42};
        QCOMPARE(TestDataSource::commaDelimitedIntList(list), QStringLiteral("42"));
    }

    void commaDelimitedIntList_emptyList()
    {
        QList<int> list;
        QCOMPARE(TestDataSource::commaDelimitedIntList(list), QStringLiteral(""));
    }

    void commaDelimitedStringList_quotesAndEscapes()
    {
        QStringList list = {"hello", "world"};
        QString result = TestDataSource::commaDelimitedStringList(list);
        QCOMPARE(result, QStringLiteral("'hello','world'"));
    }

    void commaDelimitedStringList_escapesQuotes()
    {
        QStringList list = {"it's"};
        QString result = TestDataSource::commaDelimitedStringList(list);
        QCOMPARE(result, QStringLiteral("'it''s'"));
    }

    void commaDelimitedStringList_emptyList()
    {
        QStringList list;
        QCOMPARE(TestDataSource::commaDelimitedStringList(list), QStringLiteral(""));
    }

    void commaDelimitedUuidList_formatsCorrectly()
    {
        QUuid uuid = QUuid::createUuid();
        QList<QUuid> list = {uuid};
        QString result = TestDataSource::commaDelimitedUuidList(list);
        QString expected = QString("'%1'").arg(uuid.toString(QUuid::WithoutBraces));
        QCOMPARE(result, expected);
    }

    void commaDelimitedUuidList_multipleUuids()
    {
        QUuid u1 = QUuid::createUuid();
        QUuid u2 = QUuid::createUuid();
        QList<QUuid> list = {u1, u2};
        QString result = TestDataSource::commaDelimitedUuidList(list);
        QString expected = QString("'%1','%2'")
            .arg(u1.toString(QUuid::WithoutBraces))
            .arg(u2.toString(QUuid::WithoutBraces));
        QCOMPARE(result, expected);
    }

    void utcTime_setsUtcTimezone()
    {
        QDateTime localTime = QDateTime::currentDateTime();
        QVariant variant(localTime);
        QDateTime result = TestDataSource::utcTime(variant);
        QCOMPARE(result.timeZone(), QTimeZone::utc());
    }

    void currentTimestamp_notEmpty()
    {
        QString ts = TestDataSource::currentTimestamp();
        QVERIFY(!ts.isEmpty());
    }

    void errorText_whenEmpty_returnsEmpty()
    {
        TestDataSource ds;
        QVERIFY(ds.errorText().isEmpty());
    }

    void isSqlite_static_nonExistentFile_returnsFalse()
    {
        QVERIFY(!DataSource::isSqlite("/tmp/nonexistent_file_12345.db"));
    }

    void isSqlite_static_validSqliteFile_returnsTrue()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());
        QString dbPath = tmpDir.path() + "/valid.db";

        // Create a valid SQLite database
        DatabaseCredentials creds(dbPath);
        TestDataSource ds(creds);
        ds.testCreateSql = "CREATE TABLE test (id INTEGER PRIMARY KEY);";
        QVERIFY(ds.openConnection());
        ds.closeConnection();

        QVERIFY(DataSource::isSqlite(dbPath));
    }

    void isSqlite_static_nonSqliteFile_returnsFalse()
    {
        QTemporaryFile tmpFile;
        QVERIFY(tmpFile.open());
        tmpFile.write("This is not a SQLite database");
        tmpFile.close();

        QVERIFY(!DataSource::isSqlite(tmpFile.fileName()));
    }

    void foreignKeyEnforcement_works()
    {
        QTemporaryDir tmpDir;
        QVERIFY(tmpDir.isValid());
        QString dbPath = tmpDir.path() + "/fk_test.db";

        DatabaseCredentials creds(dbPath);
        TestDataSource ds(creds);
        ds.testCreateSql =
            "CREATE TABLE parent (id INTEGER PRIMARY KEY);\n"
            "CREATE TABLE child (id INTEGER PRIMARY KEY, parent_id INTEGER REFERENCES parent(id));";
        QVERIFY(ds.openConnection());

        // Insert into parent
        bool success = false;
        ds.executeQuery("INSERT INTO parent (id) VALUES (1)", &success);
        QVERIFY(success);

        // Insert child with valid FK
        ds.executeQuery("INSERT INTO child (id, parent_id) VALUES (1, 1)", &success);
        QVERIFY(success);

        // Insert child with invalid FK should fail (foreign keys enabled)
        ds.executeQuery("INSERT INTO child (id, parent_id) VALUES (2, 999)", &success);
        QVERIFY(!success);

        ds.closeConnection();
    }
};

QTEST_MAIN(TstDataSource)
#include "tst_datasource.moc"
