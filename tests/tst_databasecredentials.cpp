#include <QTest>
#include <Kanoop/database/databasecredentials.h>

class TstDatabaseCredentials : public QObject
{
    Q_OBJECT

private slots:
    void defaultConstructor_isInvalid()
    {
        DatabaseCredentials creds;
        QVERIFY(!creds.isValid());
        QVERIFY(creds.host().isEmpty());
        QVERIFY(creds.schema().isEmpty());
        QVERIFY(creds.username().isEmpty());
        QVERIFY(creds.password().isEmpty());
        QVERIFY(creds.engine().isEmpty());
    }

    void sqliteConstructor_setsEngineAndSchema()
    {
        DatabaseCredentials creds("/tmp/test.db");
        QCOMPARE(creds.schema(), QStringLiteral("/tmp/test.db"));
        QCOMPARE(creds.engine(), DatabaseCredentials::SQLENG_SQLITE);
        QVERIFY(creds.isSqlite());
        QVERIFY(creds.isValid());
    }

    void fullConstructor_setsAllFields()
    {
        DatabaseCredentials creds("localhost", "mydb", "user", "pass", DatabaseCredentials::SQLENG_MYSQL);
        QCOMPARE(creds.host(), QStringLiteral("localhost"));
        QCOMPARE(creds.schema(), QStringLiteral("mydb"));
        QCOMPARE(creds.username(), QStringLiteral("user"));
        QCOMPARE(creds.password(), QStringLiteral("pass"));
        QCOMPARE(creds.engine(), DatabaseCredentials::SQLENG_MYSQL);
        QVERIFY(!creds.isSqlite());
        QVERIFY(creds.isValid());
    }

    void setters_modifyFields()
    {
        DatabaseCredentials creds;
        creds.setHost("host1");
        creds.setSchema("schema1");
        creds.setUsername("user1");
        creds.setPassword("pass1");
        creds.setEngine(DatabaseCredentials::SQLENG_PGSQL);

        QCOMPARE(creds.host(), QStringLiteral("host1"));
        QCOMPARE(creds.schema(), QStringLiteral("schema1"));
        QCOMPARE(creds.username(), QStringLiteral("user1"));
        QCOMPARE(creds.password(), QStringLiteral("pass1"));
        QCOMPARE(creds.engine(), DatabaseCredentials::SQLENG_PGSQL);
    }

    void isValid_emptySchema_returnsFalse()
    {
        DatabaseCredentials creds;
        creds.setHost("localhost");
        creds.setEngine(DatabaseCredentials::SQLENG_MYSQL);
        QVERIFY(!creds.isValid());
    }

    void isValid_withSchema_returnsTrue()
    {
        DatabaseCredentials creds;
        creds.setSchema("testdb");
        QVERIFY(creds.isValid());
    }

    void isSqlite_sqliteEngine_returnsTrue()
    {
        DatabaseCredentials creds;
        creds.setEngine(DatabaseCredentials::SQLENG_SQLITE);
        QVERIFY(creds.isSqlite());
    }

    void isSqlite_mysqlEngine_returnsFalse()
    {
        DatabaseCredentials creds;
        creds.setEngine(DatabaseCredentials::SQLENG_MYSQL);
        QVERIFY(!creds.isSqlite());
    }

    void isSqlite_pgsqlEngine_returnsFalse()
    {
        DatabaseCredentials creds;
        creds.setEngine(DatabaseCredentials::SQLENG_PGSQL);
        QVERIFY(!creds.isSqlite());
    }

    void engineConstants_haveExpectedValues()
    {
        QCOMPARE(DatabaseCredentials::SQLENG_SQLITE, QStringLiteral("QSQLITE"));
        QCOMPARE(DatabaseCredentials::SQLENG_MYSQL, QStringLiteral("QMYSQL"));
        QCOMPARE(DatabaseCredentials::SQLENG_PGSQL, QStringLiteral("QPSQL"));
    }
};

QTEST_MAIN(TstDatabaseCredentials)
#include "tst_databasecredentials.moc"
