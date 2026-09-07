#ifndef SQLPARSER_H
#define SQLPARSER_H
#include <QStringList>

/** @brief Parses a multi-statement SQL string into individual SQL statements. */
class SqlParser
{
public:
    /** @brief Construct a parser and parse the given SQL string into individual statements.
     *  @param sql The SQL string potentially containing multiple semicolon-delimited statements.
     */
    SqlParser(const QString& sql);

    /** @brief Get the list of parsed SQL statements.
     *  @return The list of individual SQL statements.
     */
    QStringList statements() const { return _statements; }

    /** @brief Return true if the parse extracted at least one statement.
     *  @return true if statements() is non-empty.
     */
    bool isValid() const { return _valid; }

private:
    void parse(const QString& sql);
    void startNewStatement();

    QStringList _statements;
    bool _valid;

    QStringList _working;
};

#endif // SQLPARSER_H
