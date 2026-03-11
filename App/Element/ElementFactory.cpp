// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementFactory.h"

#include <QMetaEnum>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/LogicElements/LogicAnd.h"
#include "App/Element/LogicElements/LogicDemux.h"
#include "App/Element/LogicElements/LogicDFlipFlop.h"
#include "App/Element/LogicElements/LogicDLatch.h"
#include "App/Element/LogicElements/LogicInput.h"
#include "App/Element/LogicElements/LogicJKFlipFlop.h"
#include "App/Element/LogicElements/LogicMux.h"
#include "App/Element/LogicElements/LogicNand.h"
#include "App/Element/LogicElements/LogicNode.h"
#include "App/Element/LogicElements/LogicNone.h"
#include "App/Element/LogicElements/LogicNor.h"
#include "App/Element/LogicElements/LogicNot.h"
#include "App/Element/LogicElements/LogicOr.h"
#include "App/Element/LogicElements/LogicOutput.h"
#include "App/Element/LogicElements/LogicSRFlipFlop.h"
#include "App/Element/LogicElements/LogicSRLatch.h"
#include "App/Element/LogicElements/LogicTFlipFlop.h"
#include "App/Element/LogicElements/LogicTruthTable.h"
#include "App/Element/LogicElements/LogicXnor.h"
#include "App/Element/LogicElements/LogicXor.h"
#include "App/GlobalProperties.h"

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

    return instance().getStaticProperties(type).titleText;
}

QString ElementFactory::translatedName(const ElementType type)
{
    if (type == ElementType::Unknown) {
        return tr("Unknown");
    }

    return instance().getStaticProperties(type).translatedName;
}

QPixmap ElementFactory::pixmap(const ElementType type)
{
    if (type == ElementType::Unknown) {
        return {};
    }

    return QPixmap{instance().getStaticProperties(type).pixmapPath};
}

GraphicElement *ElementFactory::buildElement(const ElementType type)
{
    qCDebug(four) << type;

    if (type == ElementType::Unknown) {
        throw PANDACEPTION("Unknown type 1: %1", typeToText(type));
    }

    // Each GraphicElement subclass is registered with Q_DECLARE_METATYPE and
    // qRegisterMetaType so that QMetaType can instantiate it by name without a
    // manual switch/case.  typeToText() produces the exact string used at registration.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const int id = QMetaType::type(typeToText(type).toLatin1());

    if (id == QMetaType::UnknownType) {
        throw PANDACEPTION("Unknown type: %1", typeToText(type));
    }

    auto *elm = static_cast<GraphicElement *>(QMetaType::create(id));
#else
    // Qt 6 replaced the integer-based API with QMetaType value objects.
    const auto metaType = QMetaType::fromName(typeToText(type).toLatin1());

    if (!metaType.isValid() || (metaType.id() == QMetaType::UnknownType)) {
        throw PANDACEPTION("Unknown type 2: %1", typeToText(type));
    }

    auto *elm = static_cast<GraphicElement *>(metaType.create());
#endif

    return elm;
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

void ElementFactory::clearCache()
{
    instance().m_propertyCache.clear();
}

int ElementFactory::nextId()
{
    return ++m_lastId;
}

void ElementFactory::setLastId(const int newLastId)
{
    instance().m_lastId = newLastId;
}

std::shared_ptr<LogicElement> ElementFactory::buildLogicElement(GraphicElement *elm)
{
    switch (elm->elementType()) {
    case ElementType::Clock:
    case ElementType::InputButton:
    case ElementType::InputRotary:
    case ElementType::InputSwitch: return std::make_shared<LogicInput>(false, elm->outputSize());

    case ElementType::AudioBox:
    case ElementType::Buzzer:
    case ElementType::Display14:
    case ElementType::Display16:
    case ElementType::Display7:
    case ElementType::Led:         return std::make_shared<LogicOutput>(elm->inputSize());

    case ElementType::And:         return std::make_shared<LogicAnd>(elm->inputSize());
    case ElementType::DFlipFlop:   return std::make_shared<LogicDFlipFlop>();
    case ElementType::DLatch:      return std::make_shared<LogicDLatch>();
    case ElementType::Demux:       return std::make_shared<LogicDemux>();
    case ElementType::InputGnd:    return std::make_shared<LogicInput>(false);
    case ElementType::InputVcc:    return std::make_shared<LogicInput>(true);
    case ElementType::JKFlipFlop:  return std::make_shared<LogicJKFlipFlop>();
    case ElementType::Mux:         return std::make_shared<LogicMux>();
    case ElementType::Nand:        return std::make_shared<LogicNand>(elm->inputSize());
    case ElementType::Node:        return std::make_shared<LogicNode>();
    case ElementType::Nor:         return std::make_shared<LogicNor>(elm->inputSize());
    case ElementType::Not:         return std::make_shared<LogicNot>();
    case ElementType::Or:          return std::make_shared<LogicOr>(elm->inputSize());
    case ElementType::SRFlipFlop:  return std::make_shared<LogicSRFlipFlop>();
    case ElementType::SRLatch:     return std::make_shared<LogicSRLatch>();
    case ElementType::TFlipFlop:   return std::make_shared<LogicTFlipFlop>();
    case ElementType::TruthTable: {
        auto *truthTable = qobject_cast<TruthTable*>(elm);
        if (!truthTable) {
            throw PANDACEPTION("Failed to cast element to TruthTable");
        }
        return std::make_shared<LogicTruthTable>(elm->inputSize(), elm->outputSize(), truthTable->key());
    }
    case ElementType::Xnor:        return std::make_shared<LogicXnor>(elm->inputSize());
    case ElementType::Xor:         return std::make_shared<LogicXor>(elm->inputSize());

    // Line and Text are purely decorative; LogicNone produces no outputs and is
    // never added to the simulation's update queue.
    case ElementType::Line:
    case ElementType::Text:        return std::make_shared<LogicNone>();

    default:                       throw PANDACEPTION("Not implemented yet: %1", elm->objectName());
    }
}

const ElementStaticProperties& ElementFactory::getStaticProperties(ElementType type) {
    return instance().ensurePropertiesCached(type);
}

const ElementStaticProperties& ElementFactory::ensurePropertiesCached(ElementType type) {
    if (auto it = m_propertyCache.find(type); it != m_propertyCache.end()) {
        return it.value();
    }

    // skipInit suppresses the full GraphicElement constructor body (port creation,
    // pixmap loading, scene-item flags) so we can harvest static Q_PROPERTY values
    // cheaply without allocating Qt scene infrastructure.
    GlobalProperties::skipInit = true;
    auto *elm = buildElement(type);
    GlobalProperties::skipInit = false;

    ElementStaticProperties props;
    props.pixmapPath = elm->property("pixmapPath").toString();
    props.titleText = elm->property("titleText").toString();
    props.translatedName = elm->property("translatedName").toString();

    delete elm;

    m_propertyCache[type] = props;

    return m_propertyCache.find(type).value();
}

