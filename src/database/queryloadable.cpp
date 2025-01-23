#include "queryloadable.h"

#include <QDateTime>
#include <QTimeZone>

QDateTime QueryLoadable::utcTime(const QVariant& value)
{
    QDateTime timestamp = value.toDateTime();
    timestamp.setTimeZone(QTimeZone::utc());
    return timestamp;
}
