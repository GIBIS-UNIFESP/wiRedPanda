// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementTypeDescriptor.h"

#include "App/Core/Common.h"

QMap<ElementType, ElementTypeDescriptor> &ElementTypeDescriptor::registry()
{
    // QMap (not QHash): registrations happen via static-inline lambdas in each
    // element's .cpp translation unit before QApplication::exec() runs, and
    // iteration order over this registry is otherwise unspecified between
    // runs/platforms unless insertion order is preserved — some serialization
    // paths rely on that. Single-threaded at startup; concurrent access after
    // is undefined.
    static QMap<ElementType, ElementTypeDescriptor> s_registry;
    return s_registry;
}

void ElementTypeDescriptor::add(const ElementTypeDescriptor &descriptor)
{
    registry()[descriptor.type] = descriptor;
}

const ElementTypeDescriptor &ElementTypeDescriptor::of(ElementType type)
{
    auto &reg = registry();

    // Every type the interface and the factory name has a descriptor; a stream's type is checked before it gets here
    if (!reg.contains(type)) {
        Q_ASSERT_X(false, "ElementTypeDescriptor::of", "no descriptor added for this element type");
        qCWarning(zero) << "No descriptor added for element type" << static_cast<int>(type);
        static const ElementTypeDescriptor none;
        return none;
    }

    return reg[type];
}

bool ElementTypeDescriptor::has(ElementType type)
{
    return registry().contains(type);
}
