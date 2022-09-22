#include "enums.h"

#include <QDataStream>

ElementType &operator++(ElementType &type)
{
    return type = static_cast<ElementType>(static_cast<int>(type) + 1);
}

QDataStream &operator>>(QDataStream &stream, ElementType &type)
{
    quint64 temp; stream >> temp;
    type = static_cast<ElementType>(temp);
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const ElementType &type)
{
    stream << static_cast<quint64>(type);
    return stream;
}
