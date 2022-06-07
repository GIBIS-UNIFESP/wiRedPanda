// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementfactory.h"

#include "common.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "qneconnection.h"

#include <QMetaEnum>

ElementType ElementFactory::textToType(const QString &text)
{
    qCDebug(four) << text;
    return static_cast<ElementType>(QMetaEnum::fromType<ElementType>().keyToValue(text.toLatin1()));
}

QString ElementFactory::typeToText(const ElementType type)
{
    qCDebug(four) << type;

    if (type == ElementType::Unknown) {
        return "UNKNOWN";
    }

    return QVariant::fromValue(type).toString();
}

QString ElementFactory::typeToTitleText(const ElementType type)
{
    qCDebug(four) << type;

    if (type == ElementType::Unknown) {
        return tr("<b>MULTIPLE TYPES</b>");
    }

    GlobalProperties::skipInit = true;
    if (auto *elm = buildElement(type)) {
        GlobalProperties::skipInit = false;
        return elm->property("titleText").toString();
    }

    return tr("<b>MULTIPLE TYPES</b>");
}

QString ElementFactory::translatedName(const ElementType type)
{
    if (type == ElementType::Unknown) {
        return tr("Unknown");
    }

    GlobalProperties::skipInit = true;
    if (auto *elm = buildElement(type)) {
        GlobalProperties::skipInit = false;
        return elm->property("translatedName").toString();
    }

    return tr("Unknown");
}

QPixmap ElementFactory::pixmap(const ElementType type)
{
    if (type == ElementType::Unknown) {
        return {};
    }

    GlobalProperties::skipInit = true;
    if (auto *elm = buildElement(type)) {
        GlobalProperties::skipInit = false;
        return QPixmap{elm->property("pixmapPath").toString()};
    }

    return {};
}

GraphicElement *ElementFactory::buildElement(const ElementType type)
{
    qCDebug(four) << type;

    if (type == ElementType::Unknown) {
        throw Pandaception(tr("Unknown type: ") + typeToText(type));
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    int id = QMetaType::type(typeToText(type).toLatin1());
    if (id == QMetaType::UnknownType) {
        throw Pandaception(tr("Unknown type: ") + typeToText(type));
    }
    auto *elm = static_cast<GraphicElement *>(QMetaType::create(id));
#else
    const auto metaType = QMetaType::fromName(typeToText(type).toLatin1());
    if (!metaType.isValid() || metaType.id() == QMetaType::UnknownType) {
        throw Pandaception(tr("Unknown type: ") + typeToText(type));
    }
    auto *elm = static_cast<GraphicElement *>(metaType.create());
#endif

    return elm;
}

QNEConnection *ElementFactory::buildConnection(QGraphicsItem *parent)
{
    return new QNEConnection(parent);
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
        const int newId = instance().next_id();
        instance().m_map[newId] = item;
        item->setId(newId);
    }
}

void ElementFactory::removeItem(ItemWithId *item)
{
    instance().m_map.remove(item->id());
}

void ElementFactory::updateItemId(ItemWithId *item, const int newId)
{
    instance().m_map.remove(item->id());
    instance().m_map[newId] = item;
    item->setId(newId);
}

int ElementFactory::next_id()
{
    return m_lastId++;
}
