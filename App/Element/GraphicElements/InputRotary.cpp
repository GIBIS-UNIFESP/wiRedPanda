// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/InputRotary.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicSource.h"
#include "App/Nodes/QNEPort.h"
#include "App/Versions.h"

template<>
struct ElementInfo<InputRotary> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::InputRotary,
        .group = ElementGroup::Input,
        .minInputSize = 0,
        .maxInputSize = 0,
        .minOutputSize = 2,
        .maxOutputSize = 16,
        .canChangeSkin = true,
        .hasColors = false,
        .hasAudio = false,
        .hasAudioBox = false,
        .hasTrigger = true,
        .hasFrequency = false,
        .hasDelay = false,
        .hasLabel = true,
        .hasTruthTable = false,
        .rotatable = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Input/rotary_icon.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("InputRotary", "ROTARY SWITCH");
        meta.translatedName = QT_TRANSLATE_NOOP("InputRotary", "Rotary Switch");
        meta.trContext = "InputRotary";
        meta.defaultSkins = QStringList({
            ":/Components/Input/rotary.svg",
            ":/Components/Input/rotary_arrow.svg",
        });
        meta.logicCreator = [](GraphicElement *elm) { return std::make_shared<LogicSource>(false, elm->outputSize()); };
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new InputRotary(); });
        return true;
    }();
};

InputRotary::InputRotary(QGraphicsItem *parent)
    : GraphicElementInput(ElementType::InputRotary, parent)
{
    m_rotary = m_defaultSkins.at(0);
    m_arrow  = m_defaultSkins.at(1);
    setLocked(false);
    InputRotary::updatePortsProperties();
}

void InputRotary::refresh()
{
    // Guard against stale currentPort if output size was reduced (e.g. via undo)
    if (m_currentPort >= InputRotary::outputSize()) {
        m_currentPort = 0;
    }

    update();
}

void InputRotary::updatePortsProperties()
{
    // Ports are placed around the perimeter of the 64x64 bounding box,
    // evenly distributed to match the visual rotary positions on the SVG.
    // Only specific output counts (2, 3, 4, 6, 8, 10, 12, 16) have defined
    // layouts; these correspond to standard rotary switch configurations.
    switch (InputRotary::outputSize()) {
    case 2: {
        outputPort(0)->setPos(32,  0);    outputPort(0)->setName("0");
        outputPort(1)->setPos(32, 64);    outputPort(1)->setName("1");
        break;
    }
    case 3: {
        outputPort(0)->setPos(32,  0);   outputPort( 0)->setName("0");
        outputPort(1)->setPos(64, 48);   outputPort( 1)->setName("1");
        outputPort(2)->setPos(0,  48);   outputPort( 2)->setName("2");
        break;
    }
    case 4: {
        outputPort(0)->setPos(32,  0);    outputPort(0)->setName("0");
        outputPort(1)->setPos(64, 32);    outputPort(1)->setName("1");
        outputPort(2)->setPos(32, 64);    outputPort(2)->setName("2");
        outputPort(3)->setPos( 0, 32);    outputPort(3)->setName("3");
        break;
    }
    case 6: {
        outputPort(0)->setPos(32,  0);   outputPort( 0)->setName("0");
        outputPort(1)->setPos(64, 16);   outputPort( 1)->setName("1");
        outputPort(2)->setPos(64, 48);   outputPort( 2)->setName("2");
        outputPort(3)->setPos(32, 64);   outputPort( 3)->setName("3");
        outputPort(4)->setPos( 0, 48);   outputPort( 4)->setName("4");
        outputPort(5)->setPos( 0, 16);   outputPort( 5)->setName("5");
        break;
    }
    case 8: {
        outputPort(0)->setPos(32,  0);    outputPort(0)->setName("0");
        outputPort(1)->setPos(64,  0);    outputPort(1)->setName("1");
        outputPort(2)->setPos(64, 32);    outputPort(2)->setName("2");
        outputPort(3)->setPos(64, 64);    outputPort(3)->setName("3");
        outputPort(4)->setPos(32, 64);    outputPort(4)->setName("4");
        outputPort(5)->setPos( 0, 64);    outputPort(5)->setName("5");
        outputPort(6)->setPos( 0, 32);    outputPort(6)->setName("6");
        outputPort(7)->setPos( 0,  0);    outputPort(7)->setName("7");
        break;
    }
    case 10: {
        outputPort(0)->setPos(32,  0);    outputPort(0)->setName("0");
        outputPort(1)->setPos(56,  0);    outputPort(1)->setName("1");
        outputPort(2)->setPos(64, 24);    outputPort(2)->setName("2");
        outputPort(3)->setPos(64, 40);    outputPort(3)->setName("3");
        outputPort(4)->setPos(56, 64);    outputPort(4)->setName("4");
        outputPort(5)->setPos(32, 64);    outputPort(5)->setName("5");
        outputPort(6)->setPos( 8, 64);    outputPort(6)->setName("6");
        outputPort(7)->setPos( 0, 40);    outputPort(7)->setName("7");
        outputPort(8)->setPos( 0, 24);    outputPort(8)->setName("8");
        outputPort(9)->setPos( 8,  0);    outputPort(9)->setName("9");
        break;
    }
    case 12: {
        outputPort( 0)->setPos(32,  0);   outputPort( 0)->setName("0");
        outputPort( 1)->setPos(48,  0);   outputPort( 1)->setName("1");
        outputPort( 2)->setPos(64, 16);   outputPort( 2)->setName("2");
        outputPort( 3)->setPos(64, 32);   outputPort( 3)->setName("3");
        outputPort( 4)->setPos(64, 48);   outputPort( 4)->setName("4");
        outputPort( 5)->setPos(48, 64);   outputPort( 5)->setName("5");
        outputPort( 6)->setPos(32, 64);   outputPort( 6)->setName("6");
        outputPort( 7)->setPos(16, 64);   outputPort( 7)->setName("7");
        outputPort( 8)->setPos( 0, 48);   outputPort( 8)->setName("8");
        outputPort( 9)->setPos( 0, 32);   outputPort( 9)->setName("9");
        // Ports >= 10 use hex labels (A, B, ...) to stay single-character
        outputPort(10)->setPos( 0, 16);   outputPort(10)->setName("A");
        outputPort(11)->setPos(16,  0);   outputPort(11)->setName("B");
        break;
    }
    case 16: {
        outputPort( 0)->setPos(32,  0);   outputPort( 0)->setName("0");
        outputPort( 1)->setPos(48,  0);   outputPort( 1)->setName("1");
        outputPort( 2)->setPos(64,  0);   outputPort( 2)->setName("2");
        outputPort( 3)->setPos(64, 16);   outputPort( 3)->setName("3");
        outputPort( 4)->setPos(64, 32);   outputPort( 4)->setName("4");
        outputPort( 5)->setPos(64, 48);   outputPort( 5)->setName("5");
        outputPort( 6)->setPos(64, 64);   outputPort( 6)->setName("6");
        outputPort( 7)->setPos(48, 64);   outputPort( 7)->setName("7");
        outputPort( 8)->setPos(32, 64);   outputPort( 8)->setName("8");
        outputPort( 9)->setPos(16, 64);   outputPort( 9)->setName("9");
        outputPort(10)->setPos( 0, 64);   outputPort(10)->setName("A");
        outputPort(11)->setPos( 0, 48);   outputPort(11)->setName("B");
        outputPort(12)->setPos( 0, 32);   outputPort(12)->setName("C");
        outputPort(13)->setPos( 0, 16);   outputPort(13)->setName("D");
        outputPort(14)->setPos( 0,  0);   outputPort(14)->setName("E");
        outputPort(15)->setPos(16,  0);   outputPort(15)->setName("F");
        break;
    }

    default:
        // Handle unexpected output sizes gracefully
        // For unhandled sizes, use a simple default positioning
        for (int i = 0; i < outputSize(); ++i) {
            outputPort(i)->setPos(32, i * 16);
            outputPort(i)->setName(QString::number(i + 1));
        }
        break;
    }

    InputRotary::refresh();
}

void InputRotary::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicElement::paint(painter, option, widget);

    // Draw a small position mark for every output port, evenly spaced around the dial.
    // The active port also gets the arrow overlay drawn on top.
    for (int port = 0; port < outputSize(); ++port) {
        painter->save();

        // Distribute marks evenly: port 0 is at the top (12 o'clock), increasing clockwise
        const double angle = 360. / outputSize() * port;
        const QPointF center = pixmapCenter();
        // Mark rect coordinates are in the pre-rotation (top) position — the painter
        // transform rotates the entire coordinate system around the dial center.
        // 30.2, 8.727 = top-centre of the dial face (horizontally centred on the 64px body,
        //               placed near the rim rather than the hub so it's visible on the SVG).
        // 3.6 × 6.4 = small rectangular pip sized to match the tick marks on the SVG artwork.
        QRectF mark{30.2, 8.727, 3.6, 6.4};

        // Off-white / cream colour matches the dial face artwork, making the tick marks blend naturally
        painter->setBrush(QBrush{QColor{255, 246, 213}});
        painter->translate(center.x(), center.y());
        painter->rotate(angle);
        painter->translate(-center.x(), -center.y());
        painter->drawRect(mark);

        painter->restore();

        if (m_currentPort != port) {
            continue;
        }

        // Overlay the arrow pixmap at the same rotation as the active mark
        painter->save();

        painter->translate(center.x(), center.y());
        painter->rotate(angle);
        painter->translate(-center.x(), -center.y());
        painter->drawPixmap(0, 0, m_arrow);

        painter->restore();
    }
}

bool InputRotary::isOn(const int port) const
{
    return (m_currentPort == port);
}

void InputRotary::setOff()
{
}

void InputRotary::setOn()
{
    // Advance the dial one position forward, wrapping around to 0 after the last port
    const int port = (outputValue() + 1) % outputSize();
    InputRotary::setOn(true, port);
}

void InputRotary::setOn(const bool value, const int port)
{
    // value is ignored — the rotary is not a simple toggle; the selected port determines state
    Q_UNUSED(value)
    m_currentPort = port;

    if (m_currentPort >= outputSize()) {
        m_currentPort = 0;
    }

    update();

    // Exactly one output port is Active (the selected position); all others are Inactive
    for (int index = 0; index < outputSize(); ++index) {
        outputPort(index)->setStatus((m_currentPort == index) ? Status::Active : Status::Inactive);
    }
}

void InputRotary::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_locked && (event->button() == Qt::LeftButton)) {
        setOn(true, (m_currentPort + 1) % outputSize());
        event->accept();
    }

    QGraphicsItem::mousePressEvent(event);
}

void InputRotary::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("currentPort", m_currentPort);
    map.insert("locked", m_locked);

    stream << map;
}

void InputRotary::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElement::load(stream, context);

    if (context.version < Versions::V_4_1) {
        // v1.x–4.0 stored currentPort as a bare int; locked added in v3.1
        stream >> m_currentPort;

        if (context.version >= Versions::V_3_1) {
            stream >> m_locked;
        }
    }

    if (context.version >= Versions::V_4_1) {
        // v4.1+ uses a key-value map
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("currentPort")) {
            m_currentPort = map.value("currentPort").toInt();
        }

        if (map.contains("locked")) {
            m_locked = map.value("locked").toBool();
        }
    }

    // Apply loaded state to ports so the simulation reflects the saved position
    setOn(true, m_currentPort);
}

void InputRotary::setSkin(const bool defaultSkin, const QString &fileName)
{
    if (defaultSkin) {
        m_alternativeSkins = m_defaultSkins;
    } else {
        m_alternativeSkins[0] = fileName;
    }

    m_usingDefaultSkin = defaultSkin;
    setPixmap(0);
}

int InputRotary::outputSize() const
{
    return static_cast<int>(outputs().size());
}

int InputRotary::outputValue() const
{
    return m_currentPort;
}

