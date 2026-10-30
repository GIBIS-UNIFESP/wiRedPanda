// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/InputButton.h"

#include <QGraphicsSceneMouseEvent>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/IO/Serialization.h"
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
        descriptor.pixmapPath = []{ return QStringLiteral(":/Components/Input/buttonOff.svg"); };
        descriptor.titleText = QT_TRANSLATE_NOOP("InputButton", "PUSH BUTTON");
        descriptor.translatedName = QT_TRANSLATE_NOOP("InputButton", "Push Button");
        descriptor.trContext = "InputButton";
        descriptor.defaultAppearances = QStringList({
            ":/Components/Input/buttonOff.svg",
            ":/Components/Input/buttonOn.svg",
        });
        // A button is held only while pressed: its level is no state of the circuit
        descriptor.parameters = {ElementInput::levelSpec(nullptr), ElementInput::lockSpec()};
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

void InputButton::save(QDataStream &stream, SerializationOptions options) const
{
    Element::save(stream, options);

    QMap<QString, QVariant> map;
    // PortableFile streams omit the default; fresh-loaded elements start there
    // anyway. Snapshots write unconditionally (reloaded into live elements).
    if (options.purpose != SerializationPurpose::PortableFile || isLocked()) {
        map.insert("locked", isLocked());
    }

    stream << map;
}

void InputButton::load(QDataStream &stream, SerializationContext &context)
{
    Element::load(stream, context);

    if ((VersionInfo::hasLockState(context.version)) && (!VersionInfo::hasQMapFormat(context.version))) {
        // v3.1–4.0 stored the locked flag as a bare bool
        bool locked = false;
        stream >> locked;
        setLocked(locked);
    }

    if (VersionInfo::hasQMapFormat(context.version)) {
        // v4.1+ uses a key-value map
        QMap<QString, QVariant> map = Serialization::readBoundedMetadata(stream);

        if (map.contains("locked")) {
            setLocked(map.value("locked").toBool());
        }
    }
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
