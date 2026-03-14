#include <QTest>
#include <Kanoop/database/sqlparser.h>

class TstSqlParser : public QObject
{
    Q_OBJECT

private slots:
    void emptyString_isInvalid()
    {
        SqlParser parser("");
        QVERIFY(!parser.isValid());
        QCOMPARE(parser.statements().count(), 0);
    }

    void singleStatement_parsed()
    {
        SqlParser parser("CREATE TABLE foo (id INTEGER);");
        QVERIFY(parser.isValid());
        QCOMPARE(parser.statements().count(), 1);
        QVERIFY(parser.statements().at(0).contains("CREATE TABLE foo"));
    }

    void multipleStatements_parsed()
    {
        QString sql =
            "CREATE TABLE foo (id INTEGER);\n"
            "CREATE TABLE bar (id INTEGER);\n"
            "INSERT INTO foo VALUES (1);";

        SqlParser parser(sql);
        QVERIFY(parser.isValid());
        QCOMPARE(parser.statements().count(), 3);
        QVERIFY(parser.statements().at(0).contains("foo"));
        QVERIFY(parser.statements().at(1).contains("bar"));
        QVERIFY(parser.statements().at(2).contains("INSERT"));
    }

    void dashDashComments_stripped()
    {
        QString sql =
            "-- This is a comment\n"
            "CREATE TABLE foo (id INTEGER);";

        SqlParser parser(sql);
        QVERIFY(parser.isValid());
        QCOMPARE(parser.statements().count(), 1);
        QVERIFY(!parser.statements().at(0).contains("--"));
    }

    void hashComments_stripped()
    {
        QString sql =
            "# This is a comment\n"
            "CREATE TABLE foo (id INTEGER);";

        SqlParser parser(sql);
        QVERIFY(parser.isValid());
        QCOMPARE(parser.statements().count(), 1);
        QVERIFY(!parser.statements().at(0).contains("#"));
    }

    void blankLines_ignored()
    {
        QString sql =
            "\n"
            "\n"
            "CREATE TABLE foo (id INTEGER);\n"
            "\n"
            "CREATE TABLE bar (id INTEGER);";

        SqlParser parser(sql);
        QVERIFY(parser.isValid());
        QCOMPARE(parser.statements().count(), 2);
    }

    void multiLineStatement_joinedCorrectly()
    {
        QString sql =
            "CREATE TABLE foo (\n"
            "    id INTEGER PRIMARY KEY,\n"
            "    name TEXT NOT NULL\n"
            ");";

        SqlParser parser(sql);
        QVERIFY(parser.isValid());
        QCOMPARE(parser.statements().count(), 1);
        QVERIFY(parser.statements().at(0).contains("id INTEGER PRIMARY KEY"));
        QVERIFY(parser.statements().at(0).contains("name TEXT NOT NULL"));
    }

    void commentsInsideStatement_skipped()
    {
        QString sql =
            "CREATE TABLE foo (\n"
            "-- this is a mid-statement comment\n"
            "    id INTEGER\n"
            ");";

        SqlParser parser(sql);
        QVERIFY(parser.isValid());
        QCOMPARE(parser.statements().count(), 1);
        QVERIFY(!parser.statements().at(0).contains("--"));
    }

    void onlyComments_isInvalid()
    {
        QString sql =
            "-- just a comment\n"
            "# another comment\n";

        SqlParser parser(sql);
        QVERIFY(!parser.isValid());
        QCOMPARE(parser.statements().count(), 0);
    }

    void onlyBlankLines_isInvalid()
    {
        SqlParser parser("\n\n\n");
        QVERIFY(!parser.isValid());
        QCOMPARE(parser.statements().count(), 0);
    }

    void statementWithoutTrailingSemicolon_parsed()
    {
        SqlParser parser("SELECT 1");
        QVERIFY(parser.isValid());
        QCOMPARE(parser.statements().count(), 1);
    }

    void mixedCommentsAndStatements()
    {
        QString sql =
            "-- Header comment\n"
            "CREATE TABLE a (id INTEGER);\n"
            "# Another comment\n"
            "\n"
            "CREATE TABLE b (id INTEGER);\n"
            "-- Trailing comment\n";

        SqlParser parser(sql);
        QVERIFY(parser.isValid());
        QCOMPARE(parser.statements().count(), 2);
    }
};

QTEST_MAIN(TstSqlParser)
#include "tst_sqlparser.moc"
