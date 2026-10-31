// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/InputButton.h"

#include <QGraphicsSceneMouseEvent>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"

template<>
struct ElementInfo<InputButton> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::InputButton,
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
        descriptor.pixmapPath = QStringLiteral(":/Components/Input/buttonOff.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("InputButton", "PUSH BUTTON");
        descriptor.translatedName = QT_TRANSLATE_NOOP("InputButton", "Push Button");
        descriptor.trContext = "InputButton";
        descriptor.defaultAppearances = QStringList({
            ":/Components/Input/buttonOff.svg",
            ":/Components/Input/buttonOn.svg",
        });
        // A button is held only while pressed: its level is no state of the circuit
        descriptor.parameters = {ElementInput::levelSpec(nullptr), ElementInput::lockSpec()};
        descriptor.readBeforeMap = [](Element &element, QDataStream &stream, const SerializationContext &context) {
            // 3.1 to 4.0 held the lock as a bare bool
            if (VersionInfo::hasLockState(context.version)) {
                bool locked = false;
                stream >> locked;
                element.setParam(ParamId::Locked, locked);
            }
        };
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new InputButton(); });
        return true;
    }();
};

InputButton::InputButton(QGraphicsItem *parent)
    : ElementInput(ElementType::InputButton, parent)
{
    InputButton::setOff();
}

void InputButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Momentary action: button goes HIGH on press, LOW on release (unlike InputSwitch which latches)
    if (!isLocked() && (event->button() == Qt::LeftButton)) {
        setOn();
        event->accept();
    }

    QGraphicsItem::mousePressEvent(event);
}

void InputButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!isLocked() && (event->button() == Qt::LeftButton)) {
        setOff();
        event->accept();
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

bool InputButton::isOn(const int port) const
{
    Q_UNUSED(port)
    return param<bool>(ParamId::On);
}

void InputButton::setOff()
{
    InputButton::setOn(false);
}

void InputButton::setOn()
{
    InputButton::setOn(true);
}

QList<std::pair<int, QString>> InputButton::appearanceStates() const
{
    return {{0, tr("Released")}, {1, tr("Pressed")}};
}
