// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/InputSwitch.h"

#include <QGraphicsSceneMouseEvent>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"

template<>
struct ElementInfo<InputSwitch> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::InputSwitch,
        .group = ElementGroup::Input,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeAppearance = true,
        .hasTrigger = true,
        .hasLabel = true,
        .rotatable = false,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Input/switchOn.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("InputSwitch", "INPUT SWITCH");
        meta.translatedName = QT_TRANSLATE_NOOP("InputSwitch", "Input Switch");
        meta.trContext = "InputSwitch";
        meta.defaultAppearances = QStringList({
            ":/Components/Input/switchOff.svg",
            ":/Components/Input/switchOn.svg",
        });
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new InputSwitch(); });
        return true;
    }();
};

InputSwitch::InputSwitch(QGraphicsItem *parent)
    : GraphicElementInput(ElementType::InputSwitch, parent)
{
    m_locked = false;
}

bool InputSwitch::isOn(const int port) const
{
    Q_UNUSED(port)
    return m_isOn;
}

void InputSwitch::setOff()
{
    // Unlike a button, a switch toggles its latched state rather than forcing a specific value.
    // Both setOff() and setOn() implement toggle so external callers (e.g. scripts, tests)
    // get consistent behavior regardless of which variant they call.
    InputSwitch::setOn(!isOn());
}

void InputSwitch::setOn()
{
    InputSwitch::setOn(!isOn());
}

void InputSwitch::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_locked && (event->button() == Qt::LeftButton)) {
        setOn(!m_isOn);
        event->accept();
    }

    QGraphicsItem::mousePressEvent(event);
}

void InputSwitch::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("isOn", m_isOn);
    map.insert("locked", m_locked);

    stream << map;
}

void InputSwitch::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElement::load(stream, context);

    if (!VersionInfo::hasQMapFormat(context.version)) {
        // v1.x–4.0 stored isOn as a bare bool; locked flag added in v3.1
        stream >> m_isOn;

        if (VersionInfo::hasLockState(context.version)) {
            stream >> m_locked;
        }
    }

    if (VersionInfo::hasQMapFormat(context.version)) {
        // v4.1+ uses a key-value map
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("isOn")) {
            m_isOn = map.value("isOn").toBool();
        }

        if (map.contains("locked")) {
            m_locked = map.value("locked").toBool();
        }
    }

    // Apply the loaded on/off state to port and pixmap
    setOn(m_isOn);
}

QList<QPair<int, QString>> InputSwitch::appearanceStates() const
{
    return {{0, tr("Off")}, {1, tr("On")}};
}

