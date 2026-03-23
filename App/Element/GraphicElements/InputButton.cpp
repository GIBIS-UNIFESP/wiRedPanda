// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/InputButton.h"

#include <QGraphicsSceneMouseEvent>

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicInput.h"
#include "App/GlobalProperties.h"
#include "App/Nodes/QNEPort.h"
#include "App/Versions.h"

template<>
struct ElementInfo<InputButton> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::InputButton,
        .group = ElementGroup::Input,
        .minInputSize = 0,
        .maxInputSize = 0,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeSkin = true,
        .hasColors = false,
        .hasAudio = false,
        .hasAudioBox = false,
        .hasTrigger = true,
        .hasFrequency = false,
        .hasDelay = false,
        .hasLabel = true,
        .hasTruthTable = false,
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
        meta.defaultSkins = QStringList({
            ":/Components/Input/buttonOff.svg",
            ":/Components/Input/buttonOn.svg",
        });
        meta.logicCreator = [](GraphicElement *elm) { return std::make_shared<LogicInput>(false, elm->outputSize()); };
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

    if ((Versions::V_3_1 <= context.version) && (context.version < Versions::V_4_1)) {
        // v3.1–4.0 stored the locked flag as a bare bool
        stream >> m_locked;
    }

    if (context.version >= Versions::V_4_1) {
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

void InputButton::setOn(const bool value, const int port)
{
    Q_UNUSED(port)
    m_isOn = value;
    // Pixmap index 0 = button-off SVG, index 1 = button-on SVG (matches bool→int cast)
    setPixmap(static_cast<int>(m_isOn));
    outputPort()->setStatus(static_cast<Status>(m_isOn));
}

void InputButton::setSkin(const bool defaultSkin, const QString &fileName)
{
    if (defaultSkin) {
        m_alternativeSkins = m_defaultSkins;
    } else {
        m_alternativeSkins[static_cast<int>(m_isOn)] = fileName;
    }

    m_usingDefaultSkin = defaultSkin;
    setPixmap(static_cast<int>(m_isOn));
}

