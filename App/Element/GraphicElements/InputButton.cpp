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
        .rotatable = false,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Input/buttonOff.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("InputButton", "PUSH BUTTON");
        meta.translatedName = QT_TRANSLATE_NOOP("InputButton", "Push Button");
        meta.trContext = "InputButton";
        meta.defaultAppearances = QStringList({
            ":/Components/Input/buttonOff.svg",
            ":/Components/Input/buttonOn.svg",
        });
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new InputButton(); });
        return true;
    }();
};

InputButton::InputButton(QGraphicsItem *parent)
    : GraphicElementInput(ElementType::InputButton, parent)
{
    m_locked = false;

    InputButton::setOff();
}

void InputButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Momentary action: button goes HIGH on press, LOW on release (unlike InputSwitch which latches)
    if (!m_locked && (event->button() == Qt::LeftButton)) {
        setOn();
        event->accept();
    }

    QGraphicsItem::mousePressEvent(event);
}

void InputButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_locked && (event->button() == Qt::LeftButton)) {
        setOff();
        event->accept();
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

void InputButton::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("locked", m_locked);

    stream << map;
}

void InputButton::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElement::load(stream, context);

    if ((VersionInfo::hasLockState(context.version)) && (!VersionInfo::hasQMapFormat(context.version))) {
        // v3.1–4.0 stored the locked flag as a bare bool
        stream >> m_locked;
    }

    if (VersionInfo::hasQMapFormat(context.version)) {
        // v4.1+ uses a key-value map
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("locked")) {
            m_locked = map.value("locked").toBool();
        }
    }
}

bool InputButton::isOn(const int port) const
{
    Q_UNUSED(port)
    return m_isOn;
}

void InputButton::setOff()
{
    InputButton::setOn(false);
}

void InputButton::setOn()
{
    InputButton::setOn(true);
}

QList<QPair<int, QString>> InputButton::appearanceStates() const
{
    return {{0, tr("Released")}, {1, tr("Pressed")}};
}

