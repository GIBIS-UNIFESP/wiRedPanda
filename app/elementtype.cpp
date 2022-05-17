#include "elementtype.h"

#include <QDataStream>

ElementType &operator++(ElementType &e) { return e = static_cast<ElementType>(static_cast<int>(e) + 1); }

QDataStream &operator>>(QDataStream &stream, ElementType &type)
{
    quint64 temp;
    stream >> temp;
    type = static_cast<ElementType>(temp);
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const ElementType &type)
{
    stream << static_cast<quint64>(type);
    return stream;
}
