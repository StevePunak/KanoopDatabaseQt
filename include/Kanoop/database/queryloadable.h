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

/** @brief Pure abstract interface for classes that can populate themselves from an active QSqlQuery. */
class QueryLoadable
{
public:
    /** @brief Load the object's state from the current row of a query result.
     *  @param query The active QSqlQuery positioned at the row to load from.
     *  @return true on success, false on failure.
     */
    virtual bool loadFromQuery(const QSqlQuery& query) = 0;

protected:
    /** @brief Convert a QVariant database value to a UTC QDateTime.
     *  @param value The QVariant containing the datetime value.
     *  @return The corresponding QDateTime in UTC.
     */
    static QDateTime utcTime(const QVariant& value);

    /** @brief Convert a single-character string to an enum value by casting the first character.
     *  @param value The string representation of the enum.
     *  @return The enum value, or a default-constructed T if the string is empty.
     */
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
