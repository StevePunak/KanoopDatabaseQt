#include "sqlparser.h"

#include <QTextStream>


SqlParser::SqlParser(const QString& sql)
{
    parse(sql);
}

void SqlParser::parse(const QString& sql)
{
    QTextStream input(sql.toUtf8());

    while(!input.atEnd()) {
        QString line = input.readLine();
        if( line.startsWith("--") ||
            line.startsWith('#') ||
            line.isEmpty()) {
            startNewStatement();
            continue;
        }

        _working.append(line);

        if(line.endsWith(';')) {
            startNewStatement();
        }
    }

    startNewStatement();

    _valid = _statements.count() > 0;
}

void SqlParser::startNewStatement()
{
    if(_working.count() == 0) {
        return;
    }

    QString statement;
    for(const QString& line : _working) {
        statement.append(line);
        statement.append('\n');
    }

    _statements.append(statement);
    _working.clear();
}
