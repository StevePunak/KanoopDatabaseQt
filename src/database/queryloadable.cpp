#include "queryloadable.h"

#include <QDateTime>

QDateTime QueryLoadable::utcTime(const QVariant& value)
{
    QDateTime timestamp = value.toDateTime();
    timestamp.setTimeSpec(Qt::UTC);
    return timestamp;
}
