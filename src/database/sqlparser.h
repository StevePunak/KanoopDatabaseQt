#ifndef SQLPARSER_H
#define SQLPARSER_H
#include <QStringList>

class SqlParser
{
public:
    SqlParser(const QString& sql);

    QStringList statements() const { return _statements; }

    bool isValid() const { return _valid; }

private:
    void parse(const QString& sql);
    void startNewStatement();

    QStringList _statements;
    bool _valid;

    QStringList _working;
};

#endif // SQLPARSER_H
