#include "elementtype.h"

#include <QDataStream>

ElementType &operator++(ElementType &e) { return e = static_cast<ElementType>(static_cast<int>(e) + 1); }

QDataStream &operator>>(QDataStream &ds, ElementType &type) {
    quint64 temp;
    ds >> temp;
    type = static_cast<ElementType>(temp);
    return ds;
}
