// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementfactory.h"

#include "common.h"
#include "graphicelement.h"
#include "qneconnection.h"

#include <QMetaEnum>

ElementFactory *ElementFactory::instance = new ElementFactory();

int ElementFactory::getLastId() const
{
    return m_lastId;
}

ElementType ElementFactory::textToType(const QString &text)
{
    qCDebug(four) << "Creating Element Type conversion text to type.";
    return static_cast<ElementType>(QMetaEnum::fromType<ElementType>().keyToValue(text.toLatin1()));
}

QString ElementFactory::typeToText(ElementType type)
{
    qCDebug(four) << "Creating Element Type conversion type to text.";

    if (type == ElementType::Unknown) {
        return {};
    }

    return QVariant::fromValue(type).toString();
}

QString ElementFactory::typeToTitleText(ElementType type)
{
    qCDebug(four) << "Creating Element Type conversion type to text.";

    if (type == ElementType::Unknown) {
        return {};
    }

    auto *elm = buildElement(type);
    if (elm) { return elm->property("titleText").toString(); }

    return {};
}

QString ElementFactory::translatedName(ElementType type)
{
    if (type == ElementType::Unknown) {
        return {};
    }

    auto *elm = buildElement(type);
    if (elm) { return elm->property("translatedName").toString(); }

    return {};
}

QPixmap ElementFactory::getPixmap(ElementType type)
{
    if (type == ElementType::Unknown) {
        return {};
    }

    auto *elm = buildElement(type);
    if (elm) { return elm->property("pixmap").toString(); }

    return {};
}

ElementFactory::ElementFactory()
{
    clear();
    m_map.clear();
    m_lastId = 0;
}

GraphicElement *ElementFactory::buildElement(ElementType type)
{
    qCDebug(four) << "Creating Element. Building it." << type;

    if (type == ElementType::Unknown) {
        return {};
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    int id = QMetaType::type(typeToText(type).toLatin1());
    if (id == QMetaType::UnknownType) {
        throw std::runtime_error(QObject::tr("Unknown type: ").toStdString() + typeToText(type).toStdString());
    }
    auto *elm = static_cast<GraphicElement *>(QMetaType::create(id));
#else
    const auto metaType = QMetaType::fromName(typeToText(type).toLatin1());
    if (not metaType.isValid() or metaType.id() == QMetaType::UnknownType) {
        throw std::runtime_error(QObject::tr("Unknown type: ").toStdString() + typeToText(type).toStdString());
    }
    auto *elm = static_cast<GraphicElement *>(metaType.create());
#endif
    return elm;
}

QNEConnection *ElementFactory::buildConnection(QGraphicsItem *parent)
{
    return new QNEConnection(parent);
}

ItemWithId *ElementFactory::getItemById(int id)
{
    if (instance->m_map.contains(id)) {
        return instance->m_map[id];
    }
    return nullptr;
}

bool ElementFactory::contains(int id)
{
    return instance->m_map.contains(id);
}

void ElementFactory::addItem(ItemWithId *item)
{
    if (item) {
        int newId = instance->next_id();
        instance->m_map[newId] = item;
        item->setId(newId);
    }
}

void ElementFactory::removeItem(ItemWithId *item)
{
    instance->m_map.remove(item->id());
}

void ElementFactory::updateItemId(ItemWithId *item, int newId)
{
    instance->m_map.remove(item->id());
    instance->m_map[newId] = item;
    item->setId(newId);
}

int ElementFactory::next_id()
{
    return m_lastId++;
}

void ElementFactory::clear()
{
    // qCDebug(zero) << "Element Factory clear.";
    // m_map.clear();
    // m_lastId = 1;
}
