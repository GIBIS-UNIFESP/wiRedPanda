// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementFactory.h"

#include <QCoreApplication>
#include <QMetaEnum>

#include "App/Core/Common.h"
#include "App/Element/ElementMetadata.h"
#include "App/Element/GraphicElement.h"

ElementType ElementFactory::textToType(const QString &text)
{
    // QMetaEnum provides compile-time–safe string↔int conversion for Q_ENUM types;
    // returns -1 on failure, which maps to the first enum value (Unknown).
    qCDebug(four) << text;
    return static_cast<ElementType>(QMetaEnum::fromType<ElementType>().keyToValue(text.toLatin1()));
}

QString ElementFactory::typeToText(const ElementType type)
{
    qCDebug(four) << type;

    if (type == ElementType::Unknown) {
        return "UNKNOWN";
    }

    // QVariant leverages the Q_ENUM registration to produce the enum key name.
    return QVariant::fromValue(type).toString();
}

QString ElementFactory::typeToTitleText(const ElementType type)
{
    qCDebug(four) << type;

    if (type == ElementType::Unknown) {
        return tr("MULTIPLE TYPES");
    }

    const auto &meta = ElementMetadataRegistry::metadata(type);
    return QCoreApplication::translate(meta.trContext, meta.titleText);
}

QString ElementFactory::translatedName(const ElementType type)
{
    if (type == ElementType::Unknown) {
        return tr("Unknown");
    }

    const auto &meta = ElementMetadataRegistry::metadata(type);
    return QCoreApplication::translate(meta.trContext, meta.translatedName);
}

QPixmap ElementFactory::pixmap(const ElementType type)
{
    if (type == ElementType::Unknown) {
        return {};
    }

    return QPixmap{ElementMetadataRegistry::metadata(type).pixmapPath()};
}

GraphicElement *ElementFactory::buildElement(const ElementType type)
{
    qCDebug(four) << type;

    if (type == ElementType::Unknown) {
        throw PANDACEPTION("Unknown element type: %1", typeToText(type));
    }

    const auto it = instance().m_creatorMap.constFind(type);

    if (it == instance().m_creatorMap.constEnd()) {
        throw PANDACEPTION("Unknown type: %1", typeToText(type));
    }

    return it.value()();
}

void ElementFactory::registerCreator(ElementType type, std::function<GraphicElement *()> creator)
{
    instance().m_creatorMap[type] = std::move(creator);
}

bool ElementFactory::hasCreator(ElementType type)
{
    return instance().m_creatorMap.contains(type);
}
