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

ItemWithId *ElementFactory::itemById(const int id)
{
    return instance().m_map.contains(id) ? instance().m_map.value(id) : nullptr;
}

bool ElementFactory::contains(const int id)
{
    return instance().m_map.contains(id);
}

void ElementFactory::addItem(ItemWithId *item)
{
    if (item) {
        // Assign the next monotonically increasing ID; IDs are never reused, so
        // a stale reference to an old ID will simply not find anything in the map.
        item->setId(instance().nextId());
        instance().m_map[item->id()] = item;
    }
}

void ElementFactory::removeItem(ItemWithId *item)
{
    instance().m_map.remove(item->id());
}

void ElementFactory::updateItemId(ItemWithId *item, const int newId)
{
    // Used by MorphCommand to preserve the logical identity of an element across
    // a type change: the old pointer is gone but the ID stays the same so that
    // undo/redo can locate the replacement element by ID.
    instance().m_map.remove(item->id());
    instance().m_map[newId] = item;
    item->setId(newId);
}

void ElementFactory::registerCreator(ElementType type, std::function<GraphicElement *()> creator)
{
    instance().m_creatorMap[type] = std::move(creator);
}

bool ElementFactory::hasCreator(ElementType type)
{
    return instance().m_creatorMap.contains(type);
}

int ElementFactory::nextId()
{
    return ++m_lastId;
}

void ElementFactory::setLastId(const int newLastId)
{
    instance().m_lastId = newLastId;
}

