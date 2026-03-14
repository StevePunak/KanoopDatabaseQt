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
            // Only flush if we have no accumulated lines (between statements).
            // If we're mid-statement, just skip the comment/blank line.
            if(_working.isEmpty()) {
                continue;
            }
            // Inside a statement — skip comment but don't flush
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
