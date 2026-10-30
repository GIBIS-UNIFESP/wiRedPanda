// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/InputSwitch.h"

#include <QGraphicsSceneMouseEvent>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/IO/Serialization.h"
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
        .rotatesGraphic = false,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = []{ return QStringLiteral(":/Components/Input/switchOn.svg"); };
        descriptor.titleText = QT_TRANSLATE_NOOP("InputSwitch", "INPUT SWITCH");
        descriptor.translatedName = QT_TRANSLATE_NOOP("InputSwitch", "Input Switch");
        descriptor.trContext = "InputSwitch";
        descriptor.defaultAppearances = QStringList({
            ":/Components/Input/switchOff.svg",
            ":/Components/Input/switchOn.svg",
        });
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new InputSwitch(); });
        return true;
    }();
};

InputSwitch::InputSwitch(QGraphicsItem *parent)
    : ElementInput(ElementType::InputSwitch, parent)
{
    // A switch drives a definite level from birth: push the OFF state to the
    // output port, which otherwise stays at the undriven Unknown default.
    // Base-qualified because the unqualified setOn()/setOff() toggle.
    ElementInput::setOn(false);
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

void InputSwitch::save(QDataStream &stream, SerializationOptions options) const
{
    Element::save(stream, options);

    const bool slim = (options.purpose == SerializationPurpose::PortableFile);

    QMap<QString, QVariant> map;
    // PortableFile streams omit defaults; fresh-loaded elements start there
    // anyway. Snapshots write unconditionally (reloaded into live elements).
    if (!slim || m_isOn) {
        map.insert("isOn", m_isOn);
    }
    if (!slim || m_locked) {
        map.insert("locked", m_locked);
    }

    stream << map;
}

void InputSwitch::load(QDataStream &stream, SerializationContext &context)
{
    Element::load(stream, context);

    if (!VersionInfo::hasQMapFormat(context.version)) {
        // v1.x–4.0 stored isOn as a bare bool; locked flag added in v3.1
        stream >> m_isOn;

        if (VersionInfo::hasLockState(context.version)) {
            stream >> m_locked;
        }
    }

    if (VersionInfo::hasQMapFormat(context.version)) {
        // v4.1+ uses a key-value map
        QMap<QString, QVariant> map = Serialization::readBoundedMetadata(stream);

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

QList<std::pair<int, QString>> InputSwitch::appearanceStates() const
{
    return {{0, tr("Off")}, {1, tr("On")}};
}
