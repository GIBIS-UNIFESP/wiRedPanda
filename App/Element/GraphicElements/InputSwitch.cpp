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
        descriptor.parameters = {ElementInput::levelSpec("isOn"), ElementInput::lockSpec()};
        descriptor.readBeforeMap = [](Element &element, QDataStream &stream, const SerializationContext &context) {
            // Before 4.1 the level was a bare bool, then from 3.1 the lock another
            bool on = false;
            stream >> on;
            element.setParam(ParamId::On, on);
            if (VersionInfo::hasLockState(context.version)) {
                bool locked = false;
                stream >> locked;
                element.setParam(ParamId::Locked, locked);
            }
        };
        // Every load drives the level the switch has on its output and picture, an unchanged one included
        descriptor.loaded = [](Element &element) {
            auto &input = static_cast<ElementInput &>(element);
            input.setOn(input.isOn());
        };
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
    return param<bool>(ParamId::On);
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
    if (!isLocked() && (event->button() == Qt::LeftButton)) {
        setOn(!isOn());
        event->accept();
    }

    QGraphicsItem::mousePressEvent(event);
}

QList<std::pair<int, QString>> InputSwitch::appearanceStates() const
{
    return {{0, tr("Off")}, {1, tr("On")}};
}
