// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/InputRotary.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QSvgRenderer>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<InputRotary> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::InputRotary,
        .group = ElementGroup::Input,
        .minOutputSize = 2,
        .maxOutputSize = 16,
        .canChangeAppearance = true,
        .hasTrigger = true,
        .hasLabel = true,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = []{ return QStringLiteral(":/Components/Input/rotary_icon.svg"); };
        descriptor.titleText = QT_TRANSLATE_NOOP("InputRotary", "ROTARY SWITCH");
        descriptor.translatedName = QT_TRANSLATE_NOOP("InputRotary", "Rotary Switch");
        descriptor.trContext = "InputRotary";
        descriptor.defaultAppearances = QStringList({
            ":/Components/Input/rotary.svg",
            ":/Components/Input/rotary_arrow.svg",
        });
        // A selection out of the outputs' range is the first output, as a file's loads
        descriptor.parameters = {
            {
                .id = ParamId::Position,
                .defaultValue = 0,
                .key = "currentPort",
                .accept = [](const Element &element, const ParamValue &value) -> std::optional<ParamValue> {
                    const int position = std::get<int>(value);
                    return ((position >= 0) && (position < element.outputSize())) ? value : ParamValue(0);
                },
            },
            ElementInput::lockSpec(),
        };
        descriptor.readBeforeMap = [](Element &element, QDataStream &stream, const SerializationContext &context) {
            // Before 4.1 the selection was a bare int, then from 3.1 the lock a bare bool
            int position = 0;
            stream >> position;
            element.setParam(ParamId::Position, position);
            if (VersionInfo::hasLockState(context.version)) {
                bool locked = false;
                stream >> locked;
                element.setParam(ParamId::Locked, locked);
            }
        };
        // Every load drives the selection on the outputs, those the load added included
        descriptor.loaded = [](Element &element) {
            auto &rotary = static_cast<InputRotary &>(element);
            rotary.setOn(true, rotary.outputValue());
        };
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new InputRotary(); });
        return true;
    }();
};

InputRotary::InputRotary(QGraphicsItem *parent)
    : ElementInput(ElementType::InputRotary, parent)
{
    updatePortsProperties();

    // The dial starts at position 0, which drives HIGH while every other
    // position drives LOW: push those definite levels to the output ports,
    // which otherwise stay at the undriven Unknown default.
    InputRotary::setOn(true, 0);
}

void InputRotary::refresh()
{
    // Guard against a stale selection if output size was reduced (e.g. via undo)
    if (outputValue() >= InputRotary::outputSize()) {
        storeParam(ParamId::Position, 0);
    }

    pictureChanged();
}

void InputRotary::layoutPorts()
{
    Element::layoutPorts();
    InputRotary::refresh();
}

std::optional<Element::PortAttributes> InputRotary::outputAttributes(const int index) const
{
    // Ports are placed around the perimeter of the 64x64 bounding box,
    // evenly distributed to match the visual rotary positions on the SVG.
    // Only specific output counts (2, 3, 4, 6, 8, 10, 12, 16) have defined
    // layouts; these correspond to standard rotary switch configurations.
    // Ports >= 10 use hex labels (A, B, ...) to stay single-character.
    struct Position {
        int x;
        int y;
        const char *name;
    };
    static const QHash<int, QList<Position>> layouts{
        {2, {{32, 0, "0"}, {32, 64, "1"}}},
        {3, {{32, 0, "0"}, {64, 48, "1"}, {0, 48, "2"}}},
        {4, {{32, 0, "0"}, {64, 32, "1"}, {32, 64, "2"}, {0, 32, "3"}}},
        {6, {{32, 0, "0"}, {64, 16, "1"}, {64, 48, "2"}, {32, 64, "3"}, {0, 48, "4"}, {0, 16, "5"}}},
        {8, {{32, 0, "0"}, {64, 0, "1"}, {64, 32, "2"}, {64, 64, "3"}, {32, 64, "4"}, {0, 64, "5"}, {0, 32, "6"}, {0, 0, "7"}}},
        {10, {{32, 0, "0"}, {56, 0, "1"}, {64, 24, "2"}, {64, 40, "3"}, {56, 64, "4"}, {32, 64, "5"}, {8, 64, "6"}, {0, 40, "7"}, {0, 24, "8"}, {8, 0, "9"}}},
        {12, {{32, 0, "0"}, {48, 0, "1"}, {64, 16, "2"}, {64, 32, "3"}, {64, 48, "4"}, {48, 64, "5"}, {32, 64, "6"}, {16, 64, "7"}, {0, 48, "8"}, {0, 32, "9"}, {0, 16, "A"}, {16, 0, "B"}}},
        {16, {{32, 0, "0"}, {48, 0, "1"}, {64, 0, "2"}, {64, 16, "3"}, {64, 32, "4"}, {64, 48, "5"}, {64, 64, "6"}, {48, 64, "7"}, {32, 64, "8"}, {16, 64, "9"}, {0, 64, "A"}, {0, 48, "B"}, {0, 32, "C"}, {0, 16, "D"}, {0, 0, "E"}, {16, 0, "F"}}},
    };

    if (const auto layout = layouts.constFind(outputSize()); layout != layouts.constEnd()) {
        if ((index < 0) || (index >= layout->size())) {
            return std::nullopt;
        }
        const Position &position = layout->at(index);
        return PortAttributes{.name = QString::fromLatin1(position.name), .position = QPointF(position.x, position.y)};
    }

    // Handle unexpected output sizes gracefully: for unhandled sizes, use a simple default positioning
    return PortAttributes{.name = QString::number(index + 1), .position = QPointF(32, index * 16)};
}

/// Shared, lazily-constructed vector renderer for the active-position arrow -- drawn directly in
/// paint() so it stays crisp at any zoom instead of blitting a fixed-resolution pixmap. The arrow
/// asset is a fixed built-in (never user-customizable, unlike the dial face), so one renderer
/// suffices for every InputRotary instance. GUI-thread only, like pixmapCache().
static QSvgRenderer &rotaryArrowRenderer()
{
    static QSvgRenderer renderer(QStringLiteral(":/Components/Input/rotary_arrow.svg"));
    return renderer;
}

void InputRotary::drawBody(QPainter &painter) const
{
    // Draw a small position mark for every output port, evenly spaced around the dial.
    // The active port also gets the arrow overlay drawn on top.
    for (int port = 0; port < outputSize(); ++port) {
        painter.save();

        // Distribute marks evenly: port 0 is at the top (12 o'clock), increasing clockwise
        const double angle = 360. / outputSize() * port;
        const QPointF center = pixmapCenter();
        // Mark rect coordinates are in the pre-rotation (top) position — the painter
        // transform rotates the entire coordinate system around the dial center.
        // 30.2, 8.727 = top-centre of the dial face (horizontally centred on the 64px body,
        //               placed near the rim rather than the hub so it's visible on the SVG).
        // 3.6 × 6.4 = small rectangular pip sized to match the tick marks on the SVG artwork.
        const QRectF mark = QRectF{30.2, 8.727, 3.6, 6.4};

        // Off-white / cream colour matches the dial face artwork, making the tick marks blend naturally
        painter.setBrush(QBrush{QColor{255, 246, 213}});
        painter.translate(center.x(), center.y());
        painter.rotate(angle);
        painter.translate(-center.x(), -center.y());
        painter.drawRect(mark);

        painter.restore();

        if (outputValue() != port) {
            continue;
        }

        // Overlay the arrow at the same rotation as the active mark
        painter.save();

        painter.translate(center.x(), center.y());
        painter.rotate(angle);
        painter.translate(-center.x(), -center.y());
        rotaryArrowRenderer().render(&painter, QRectF(0, 0, 64, 64));

        painter.restore();
    }
}

bool InputRotary::isOn(const int port) const
{
    return (outputValue() == port);
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
    // value is ignored — the rotary is not a simple toggle; the selected port determines state.
    // The spec takes a port out of range, in either direction, as the first (load() feeds this
    // raw values from the file, and a negative position would otherwise leave no port selected).
    // The selection is shown also when it is the one the dial has, so a new dial drives its
    // outputs with definite levels.
    Q_UNUSED(value)
    if (!setParam(ParamId::Position, port)) {
        showSelection();
    }
}

void InputRotary::parameterChanged(const ParamId id, const ParamValue &previous)
{
    ElementInput::parameterChanged(id, previous);
    if (id == ParamId::Position) {
        showSelection();
    }
}

void InputRotary::showSelection()
{
    pictureChanged();

    // Exactly one output port is Active (the selected position); all others are Inactive
    for (int index = 0; index < outputSize(); ++index) {
        outputPort(index)->setStatus((outputValue() == index) ? Status::Active : Status::Inactive);
    }
}

void InputRotary::resetSimState()
{
    Element::resetSimState();
    storeParam(ParamId::Position, 0);
}

void InputRotary::restoreSimState(const QVector<Status> &in, int &cursor)
{
    Element::restoreSimState(in, cursor);

    // The base restores the one-hot output values, but isOn() answers from the selection and
    // ElementInput::updateOutputs() pushes that onto the outputs on the very next tick --
    // so without this the restore is undone before anything reads it. Derive the selection back
    // out of the outputs rather than smuggling an int through a QVector<Status>, whose enum has
    // no fixed underlying type and no room for a port index.
    for (int i = 0; i < outputSize(); ++i) {
        if (Element::outputValue(i) == Status::Active) {
            storeParam(ParamId::Position, i);
            break;
        }
    }
}

void InputRotary::setWaveformValue(const bool value, const int port)
{
    // Only a high cell selects this port; a low cell is implicit (writing it would wrongly
    // re-select that port, since setOn ignores the value and selects by port index).
    if (value) {
        setOn(true, port);
    }
}

void InputRotary::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!isLocked() && (event->button() == Qt::LeftButton)) {
        setOn(true, (outputValue() + 1) % outputSize());
        event->accept();
    }

    QGraphicsItem::mousePressEvent(event);
}

void InputRotary::applyAppearance(const bool defaultAppearance, const QString &fileName)
{
    if (defaultAppearance) {
        m_appearance.resetAlternativeToDefault();
    } else {
        m_appearance.setAlternativeAppearanceAt(0, fileName);
    }

    m_appearance.setUsingDefaultAppearance(defaultAppearance);
    m_appearance.appearanceChanged();
}

int InputRotary::outputSize() const
{
    return static_cast<int>(outputs().size());
}

int InputRotary::outputValue() const
{
    return param<int>(ParamId::Position);
}
