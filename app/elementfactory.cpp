// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementfactory.h"

#include "common.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "logicand.h"
#include "logicdemux.h"
#include "logicdflipflop.h"
#include "logicdlatch.h"
#include "logicinput.h"
#include "logicjkflipflop.h"
#include "logicmux.h"
#include "logicnand.h"
#include "logicnode.h"
#include "logicnone.h"
#include "logicnor.h"
#include "logicnot.h"
#include "logicor.h"
#include "logicoutput.h"
#include "logicsrflipflop.h"
#include "logictflipflop.h"
#include "logictruthtable.h"
#include "logicxnor.h"
#include "logicxor.h"
#include "truth_table.h"

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
        return tr("MULTIPLE TYPES");
    }

    return property(type, "titleText");
}

QString ElementFactory::translatedName(const ElementType type)
{
    if (type == ElementType::Unknown) {
        return tr("Unknown");
    }

    return property(type, "translatedName");
}

QPixmap ElementFactory::pixmap(const ElementType type)
{
    if (type == ElementType::Unknown) {
        return {};
    }

    return QPixmap{property(type, "pixmapPath")};
}

GraphicElement *ElementFactory::buildElement(const ElementType type)
{
    qCDebug(four) << type;

    if (type == ElementType::Unknown) {
        throw Pandaception(tr("Unknown type: ") + typeToText(type));
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const int id = QMetaType::type(typeToText(type).toLatin1());

    if (id == QMetaType::UnknownType) {
        throw Pandaception(tr("Unknown type: ") + typeToText(type));
    }

    auto *elm = static_cast<GraphicElement *>(QMetaType::create(id));
#else
    const auto metaType = QMetaType::fromName(typeToText(type).toLatin1());

    if (!metaType.isValid() || (metaType.id() == QMetaType::UnknownType)) {
        throw Pandaception(tr("Unknown type: ") + typeToText(type));
    }

    auto *elm = static_cast<GraphicElement *>(metaType.create());
#endif

    return elm;
}

QString ElementFactory::property(const ElementType type, const QString &property)
{
    GlobalProperties::skipInit = true;
    auto *elm = buildElement(type);
    GlobalProperties::skipInit = false;

    QString value = elm->property(property.toUtf8()).toString();

    delete elm;

    return value;
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
    instance().m_map.remove(item->id());
    instance().m_map[newId] = item;
    item->setId(newId);
}

int ElementFactory::nextId()
{
    return m_lastId++;
}

std::shared_ptr<LogicElement> ElementFactory::buildLogicElement(GraphicElement *elm)
{
    switch (elm->elementType()) {
    case ElementType::Clock:
    case ElementType::InputButton:
    case ElementType::InputRotary:
    case ElementType::InputSwitch: return std::make_shared<LogicInput>(false, elm->outputSize());

    case ElementType::Buzzer:
    case ElementType::AudioBox:
    case ElementType::Display14:
    case ElementType::Display7:
    case ElementType::Led:         return std::make_shared<LogicOutput>(elm->inputSize());

    case ElementType::And:         return std::make_shared<LogicAnd>(elm->inputSize());
    case ElementType::DFlipFlop:   return std::make_shared<LogicDFlipFlop>();
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
    case ElementType::TFlipFlop:   return std::make_shared<LogicTFlipFlop>();
    case ElementType::Xnor:        return std::make_shared<LogicXnor>(elm->inputSize());
    case ElementType::Xor:         return std::make_shared<LogicXor>(elm->inputSize());
    case ElementType::TruthTable:  return std::make_shared<LogicTruthTable>(elm->inputSize(), elm->outputSize(), (dynamic_cast<TruthTable*>(elm))->key());

    case ElementType::DLatch:      return std::make_shared<LogicDLatch>();

    case ElementType::Line:
    case ElementType::Text:        return std::make_shared<LogicNone>();

    default:                       throw Pandaception(tr("Not implemented yet: ") + elm->objectName());
    }
}
