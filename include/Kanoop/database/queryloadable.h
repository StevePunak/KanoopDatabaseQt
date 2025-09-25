/**
 *  QueryLoadable
 *
 *  A pure abstract class to provide an interface to classes which
 *  are able to load themselves via an active QSqlQuery.
 *
 *  Stephen Punak, January 11 2025
 */
#ifndef QUERYLOADABLE_H
#define QUERYLOADABLE_H
#include <QSqlQuery>

class QueryLoadable
{
public:
    virtual bool loadFromQuery(const QSqlQuery& query) = 0;

protected:
    static QDateTime utcTime(const QVariant& value);

    template <typename T>
    static T enumFromString(const QString& value)
    {
        T result = T{};
        if(value.length() > 0) {
            result = static_cast<T>(value.at(0).toLatin1());
        }
        return result;
    }
};

#endif // QUERYLOADABLE_H
