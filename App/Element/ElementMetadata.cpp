// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementMetadata.h"

#include "App/Core/Common.h"

QMap<ElementType, ElementMetadata> &ElementMetadataRegistry::registry()
{
    static QMap<ElementType, ElementMetadata> s_registry;
    return s_registry;
}

void ElementMetadataRegistry::registerMetadata(const ElementMetadata &meta)
{
    registry()[meta.type] = meta;
}

const ElementMetadata &ElementMetadataRegistry::metadata(ElementType type)
{
    auto &reg = registry();

    if (!reg.contains(type)) {
        throw PANDACEPTION_WITH_CONTEXT("ElementMetadataRegistry", "No metadata registered for element type: %1", static_cast<int>(type));
    }

    return reg[type];
}

bool ElementMetadataRegistry::hasMetadata(ElementType type)
{
    return registry().contains(type);
}

