// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElement.h"

#include <cmath>
#include <memory>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QRegularExpression>
#include <QSvgRenderer>
#include <QThread>

#include "App/Core/Common.h"
#include "App/Core/Constants.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementMetadata.h"
#include "App/IO/SerializationContext.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

/// Shared label font — constructed once to avoid repeated QFont creation and fontconfig lookups.
/// Named after the bundled NotoSans-Regular.ttf (Application::registerBundledFonts(), called by
/// every host before any GraphicElement pixmap is built/cached) rather than the generic "Sans
/// Serif" alias: that alias resolves through platform fontconfig substitution, whose target font
/// -- and therefore its exact glyph metrics -- differs across machines and CI runners. Since
/// GraphicElementLabel::boundingRect() feeds real layout math (TestICFixtureLayout's overlap
/// checks, inline label editing), an unpinned font made those metrics, and therefore the tests,
/// environment-dependent. "Noto Sans" is guaranteed present (embedded as a Qt resource) and
/// therefore identical everywhere.
static const QFont &labelFont()
{
    static const QFont font = [] {
        QFont f("Noto Sans");
        f.setBold(true);
        return f;
    }();
    return font;
}

GraphicElement::GraphicElement(ElementType type, QObject *parent)
    : QObject(parent)
    , m_elementType(type)
    , m_metadata(ElementMetadataRegistry::metadata(type))
{
    const ElementMetadata &metadata = m_metadata;
    m_appearance.seedFromMetadata(metadata.defaultAppearances, metadata.alternativeAppearances, metadata.pixmapPath());
    m_titleText = QCoreApplication::translate(metadata.trContext, metadata.titleText);
    m_translatedName = QCoreApplication::translate(metadata.trContext, metadata.translatedName);
    m_elementGroup = metadata.group;
    m_hasColors = metadata.hasColors;
    m_maxInputSize = metadata.maxInputSize;
    m_maxOutputSize = metadata.maxOutputSize;
    m_minInputSize = metadata.minInputSize;
    m_minOutputSize = metadata.minOutputSize;

    qCDebug(four) << "Setting attributes.";
    m_label.setVisible(metadata.hasLabel);
    // Font is applied lazily in updateLabel() to avoid expensive text layout
    // passes for elements that never display a label (e.g. IC sub-elements).
    m_label.setBrush(Qt::black);
    // 64 px below origin keeps the label below the standard 64×64 element body
    setLabelAnchor(QPointF(0, 64));

    setPortName(m_translatedName);
    setToolTip(m_translatedName);

    qCDebug(four) << "Including input and output ports.";
    GraphicElement::setInputSize(static_cast<int>(metadata.minInputSize));
    GraphicElement::setOutputSize(static_cast<int>(metadata.minOutputSize));

    GraphicElement::updatePortsProperties();
    GraphicElement::updateTheme();

    if (m_appearance.hasDefaultAppearances()) {
        m_appearance.setPixmap(0);
    }
}

GraphicElement::~GraphicElement() = default; // LCOV_EXCL_LINE — the Itanium ABI emits separate deleting/complete-object destructor variants for this virtual dtor; gcov attributes them to the same source line, and not every variant is exercised by a given delete call site.

ElementType GraphicElement::elementType() const
{
    return m_elementType;
}

ElementGroup GraphicElement::elementGroup() const
{
    return m_elementGroup;
}

QPixmap GraphicElement::pixmap() const
{
    return m_appearance.pixmap();
}

QStringList GraphicElement::externalFiles() const
{
    return m_appearance.externalFiles();
}

void GraphicElement::setPixmap(const int index)
{
    m_appearance.setPixmap(index);
}

void GraphicElement::setPixmap(const QString &pixmapPath)
{
    m_appearance.setPixmap(pixmapPath);
}

const QVector<OutputPort *> &GraphicElement::outputs() const
{
    return m_ports.outputs();
}

InputPort *GraphicElement::inputPort(const int index) const
{
    return m_ports.inputPort(index);
}

OutputPort *GraphicElement::outputPort(const int index) const
{
    return m_ports.outputPort(index);
}

WirelessMode GraphicElement::wirelessMode() const
{
    return WirelessMode::None;
}

bool GraphicElement::hasWirelessMode() const
{
    return false;
}

const QVector<InputPort *> &GraphicElement::inputs() const
{
    return m_ports.inputs();
}

void GraphicElement::setInputs(const QVector<InputPort *> &inputs)
{
    m_ports.setInputs(inputs);
}

QPointF GraphicElement::pixmapCenter() const
{
    return m_appearance.pixmapCenter();
}

QRectF GraphicElement::boundingRect() const
{
    return portsBoundingRect().united(pixmap().rect());
}

QRectF GraphicElement::portsBoundingRect() const
{
    QRectF rectChildren;

    const auto includePort = [&](Port *port) {
        const QRectF localRect = port->boundingRect();
        QPolygonF corners;
        corners << localRect.topLeft() << localRect.topRight()
                << localRect.bottomRight() << localRect.bottomLeft();

        QPolygonF mapped;
        for (const QPointF &corner : std::as_const(corners)) {
            mapped << port->mapToOwnerLocal(corner);
        }

        rectChildren = rectChildren.united(mapped.boundingRect());
    };
    for (auto *port : inputs())  { includePort(port); }
    for (auto *port : outputs()) { includePort(port); }

    return rectChildren;
}

QRectF GraphicElement::renderBodyBounds() const
{
    return portsBoundingRect().united(QRectF(0, 0, 64, 64));
}

void GraphicElement::setLabelAnchor(const QPointF &pos)
{
    m_labelAnchor = pos;
    m_label.setPos(pos);      // baseline: always correct for non-rotatable elements
    updateLabelOrientation(); // rotatable elements get position+orientation corrected on top
}

void GraphicElement::paint(QPainter *painter)
{
    m_appearance.render(painter, boundingRect(), isSelected());
}

void GraphicElement::setPortName(const QString &name)
{
    setObjectName(name);
}

void GraphicElement::setRotation(const qreal angle)
{
    m_orientation.setRotation(angle);
}

void GraphicElement::rotatePorts()
{
    m_orientation.rotatePorts();
}

qreal GraphicElement::rotation() const
{
    return m_orientation.angle();
}

void GraphicElement::setFlippedX(const bool flipped)
{
    m_orientation.setFlippedX(flipped);
}

void GraphicElement::setFlippedY(const bool flipped)
{
    m_orientation.setFlippedY(flipped);
}

void GraphicElement::setAppearance(const bool defaultAppearance, const QString &fileName)
{
    m_appearance.setAppearance(defaultAppearance, fileName);
}

void GraphicElement::setAppearanceAt(const int index, const QString &fileName)
{
    m_appearance.setAppearanceAt(index, fileName);
}

QList<std::pair<int, QString>> GraphicElement::appearanceStates() const
{
    // Default: single appearance at index 0
    return {{0, tr("Default")}};
}

QPixmap GraphicElement::appearancePreviewPixmap(const int index, const QSize &size) const
{
    return m_appearance.previewPixmapAt(index, size);
}

void GraphicElement::updatePortsProperties()
{
    qCDebug(five) << "Updating port positions that belong to the IC.";

    // gridSize is 16 px; half that (8 px) is the port spacing unit so ports land
    // on sub-grid snap points that wires can reach when snapped to the same grid.
    const int step = Constants::gridSize / 2;

    if (!m_ports.inputs().isEmpty()) {
        // Centre the port column vertically around y=32 (the mid-point of a 64 px body).
        // The first port starts at 32 - (count-1)*step so all ports are symmetrically distributed.
        int y = 32 - (static_cast<int>(m_ports.inputs().size()) * step) + step;

        for (auto *port : m_ports.inputs()) {
            qCDebug(five) << "Setting input at " << 0 << ", " << y;

            // Inputs are pinned to the left edge (x=0) of the 64 px body
            port->setPos(0, y);

            // Non-rotatable elements keep the pixmap fixed and orient their ports in place,
            // applying the current rotation and flip about the element centre.
            if (!rotatesGraphic()) {
                m_orientation.orientPort(port);
            }

            // Ports are spaced 2*step (16 px) apart so they align with the
            // full grid and remain easily connectable with straight wires.
            y += step * 2;
        }
    }

    if (!m_ports.outputs().isEmpty()) {
        int y = 32 - (static_cast<int>(m_ports.outputs().size()) * step) + step;

        for (auto *port : m_ports.outputs()) {
            qCDebug(five) << "Setting output at " << 64 << ", " << y;

            // Outputs are pinned to the right edge (x=64) of the 64 px body
            port->setPos(64, y);

            // Non-rotatable elements keep the pixmap fixed and orient their ports in place,
            // applying the current rotation and flip about the element centre.
            if (!rotatesGraphic()) {
                m_orientation.orientPort(port);
            }

            y += step * 2;
        }
    }
}

void GraphicElement::refresh()
{
    // Reload appearance index 0, which is the currently active appearance for the element's
    // present state (e.g. after a theme change or appearance file replacement).
    setPixmap(0);
}

void GraphicElement::setPos(const QPointF &pos)
{
    m_pos = pos;

    // Keep every attached wire in sync. No grid-snapping here -- see the header.
    for (auto *port : inputs())  { port->updateConnections(); }
    for (auto *port : outputs()) { port->updateConnections(); }
}

void GraphicElement::setSelected(bool selected)
{
    m_selected = selected;

    // Propagate selection highlight to all connected wires so users see which
    // signals are attached to the selected element.
    highlight(m_selected);
}

void GraphicElement::handleDoubleClick()
{
    if (hasLabel()) {
        emit inlineEditRequested(this);
    }
}

QTransform GraphicElement::rotateFlipTransform() const
{
    if (!rotatesGraphic()) {
        return QTransform();
    }

    // Rotate about pixmapCenter() first, then flip about pixmapCenter() -- this matches real
    // QGraphicsItem/QTransform composition order; it's easy to get backwards, and the two
    // orders only disagree once both a non-zero rotation and a flip are active.
    const QPointF center = pixmapCenter();
    QTransform t;
    t.translate(center.x(), center.y());
    t.scale(isFlippedX() ? -1 : 1, isFlippedY() ? -1 : 1);
    t.rotate(rotation());
    t.translate(-center.x(), -center.y());
    return t;
}

QPointF GraphicElement::pointToScene(const QPointF &localPoint) const
{
    if (!rotatesGraphic()) {
        // Non-rotatable elements never rotate/flip themselves -- their ports carry their own
        // rotation+flip about their own pivot instead (see Port::scenePos()).
        return m_pos + localPoint;
    }

    return m_pos + rotateFlipTransform().map(localPoint);
}

QRectF GraphicElement::sceneBoundingRect() const
{
    const QRectF local = boundingRect();
    QPolygonF corners;
    corners << local.topLeft() << local.topRight() << local.bottomRight() << local.bottomLeft();

    QPolygonF scenePoly;
    for (const QPointF &corner : std::as_const(corners)) {
        scenePoly << pointToScene(corner);
    }

    return scenePoly.boundingRect();
}

bool GraphicElement::hasAudio() const
{
    return m_metadata.hasAudio;
}

QKeySequence GraphicElement::trigger() const
{
    return m_trigger;
}

void GraphicElement::setTrigger(const QKeySequence &trigger)
{
    m_trigger = trigger;
    updateLabel();
}

QString GraphicElement::genericProperties()
{
    return {};
}

void GraphicElement::updateLabel()
{
    QString label = m_labelText;

    // If a keyboard trigger is assigned, append it in parentheses so users can see
    // the shortcut directly on the canvas, e.g. "Clock (Space)"
    if (hasTrigger() && !trigger().toString().isEmpty()) {
        if (!label.isEmpty()) {
            label += " ";
        }
        label += QString("(%1)").arg(trigger().toString());
    }

    // Apply the shared font on first use — deferred from the constructor to
    // avoid an expensive QTextDocumentLayout pass for elements that never show a label.
    if (!label.isEmpty() && m_label.font() != labelFont()) {
        m_label.setFont(labelFont());
    }

    m_label.setText(label);
    // The counter-orientation pivots on the label's centre, which the new text just moved.
    updateLabelOrientation();
    labelContentChanged();
}

void GraphicElement::updateLabelOrientation()
{
    // Non-rotatable elements never transform their graphic (or its children), so their label is
    // already upright as authored.
    if (!rotatesGraphic()) {
        return;
    }

    // The label is drawn through this same rotate-then-flip composition (see paint()'s caller
    // contract). Left alone, its anchor point (m_labelAnchor, in the element's un-rotated local
    // frame) would orbit around the rotation pivot along with everything else -- fine for
    // ports/pins, but it leaves the label sweeping past wherever the pivot's "below"/"beside"
    // edge currently points to, which for a wide/tall body (or once ports are packed along an
    // edge) can land it squarely on a pin. Counter-rotate the anchor's *position* by inverting
    // rotateFlipTransform() -- the exact same composition applied to the label's own placement --
    // so it lands back at a fixed, upright position once that composition is applied again.
    const QTransform elementRotateFlip = rotateFlipTransform();
    m_label.setPos(elementRotateFlip.inverted().map(m_labelAnchor));

    // Independently counter-orient the glyphs themselves so the text reads upright and
    // unmirrored too. Pivots about the label's own local origin (0,0) -- the same point setPos()
    // above just anchored -- rather than its bounding-rect centre: pivoting about any *other*
    // point here would translate the label's own origin away from that anchor as rotation/flip
    // change (rotating/scaling about a point off-origin necessarily moves the origin), quietly
    // reintroducing the same kind of drift setPos() above was just added to eliminate. A pivot
    // choice only ever affects translation, never the rotation/scale factors, so this is exactly
    // as effective at keeping the glyphs upright as pivoting about the centre was.
    QTransform t;
    t.rotate(-rotation());
    t.scale(isFlippedX() ? -1 : 1, isFlippedY() ? -1 : 1);
    m_label.setTransform(t);
}

void GraphicElement::setLabel(const QString &label)
{
    m_labelText = label;
    updateLabel();
}

QString GraphicElement::label() const
{
    return m_labelText;
}

QRectF GraphicElement::labelSceneBoundingRect() const
{
    return m_label.sceneBoundingRect();
}

QRectF GraphicElement::labelLocalBoundingRect() const
{
    return m_label.boundingRectInOwnerLocal();
}

// Color cycle (forward): White → Red → Green → Blue → Purple → White
// These two functions implement previous/next steps in that cycle so the
// property editor can wrap around without knowing the full list
QString GraphicElement::previousColor() const
{
    if (color() == "White") return "Purple";
    if (color() == "Red") return "White";
    if (color() == "Green") return "Red";
    if (color() == "Blue") return "Green";
    if (color() == "Purple") return "Blue";
    return "White"; // Standard
}

QString GraphicElement::nextColor() const
{
    if (color() == "White") return "Red";
    if (color() == "Red") return "Green";
    if (color() == "Green") return "Blue";
    if (color() == "Blue") return "Purple";
    if (color() == "Purple") return "White";
    return "White"; // Standard
}

void GraphicElement::updateTheme()
{
    const ThemeAttributes theme = ThemeManager::attributes();

    m_label.setBrush(theme.m_graphicElementLabelColor);
    m_appearance.applyTheme(theme);

    for (auto *input : m_ports.inputs()) {
        input->updateTheme();
    }

    for (auto *output : m_ports.outputs()) {
        output->updateTheme();
    }

    update();
}

bool GraphicElement::isValid()
{
    qCDebug(four) << "Checking if the element has the required signals to compute its value.";
    // An element is valid only when every input port is satisfied (connected or optional)
    const bool valid = std::all_of(m_ports.inputs().cbegin(), m_ports.inputs().cend(),
                                   [](auto *input) { return input->isValid(); });

    if (!valid) {
        // Propagate invalid status downstream so the visual chain shows where validity breaks
        for (auto *output : m_ports.outputs()) {
            for (auto *conn : output->connections()) {
                conn->setStatus(Status::Error);

                if (auto *port = conn->otherPort(output)) {
                    port->setStatus(Status::Error);
                }
            }
        }
    }

    return valid;
}

bool GraphicElement::hasColors() const
{
    return m_hasColors;
}

bool GraphicElement::hasTrigger() const
{
    return m_metadata.hasTrigger;
}

void GraphicElement::setColor(const QString &color)
{
    Q_UNUSED(color)
}

void GraphicElement::setHasColors(const bool hasColors)
{
    m_hasColors = hasColors;
}

QString GraphicElement::color() const
{
    return {};
}

int GraphicElement::colorNameToIndex(const QString &color)
{
    if (color == "White")  { return 0; }
    if (color == "Red")    { return 1; }
    if (color == "Green")  { return 2; }
    if (color == "Blue")   { return 3; }
    if (color == "Purple") { return 4; }
    return 0;
}

void GraphicElement::setAudio(const QString &audio)
{
    Q_UNUSED(audio)
}

QString GraphicElement::audio() const
{
    return {};
}

bool GraphicElement::hasFrequency() const
{
    return m_metadata.hasFrequency;
}

bool GraphicElement::hasDelay() const
{
    return m_metadata.hasDelay;
}

bool GraphicElement::hasLabel() const
{
    return m_metadata.hasLabel;
}

bool GraphicElement::hasTruthTable() const
{
    return m_metadata.hasTruthTable;
}

bool GraphicElement::hasAudioBox() const
{
    return m_metadata.hasAudioBox;
}

bool GraphicElement::hasVolume() const
{
    return m_metadata.hasVolume;
}

float GraphicElement::volume() const
{
    return 0.0f;
}

void GraphicElement::setVolume(float)
{
}

QList<PropertyDescriptor> GraphicElement::editableProperties() const
{
    QList<PropertyDescriptor> props;
    if (hasLabel())      props.append({PropertyDescriptor::Type::Label});
    if (hasColors())     props.append({PropertyDescriptor::Type::Color});
    if (hasFrequency())  props.append({PropertyDescriptor::Type::Frequency});
    if (hasDelay())      props.append({PropertyDescriptor::Type::Delay});
    if (hasAudio())      props.append({PropertyDescriptor::Type::Audio});
    if (hasAudioBox())   props.append({PropertyDescriptor::Type::AudioBox});
    if (hasVolume())     props.append({PropertyDescriptor::Type::Volume});
    if (hasTrigger())    props.append({PropertyDescriptor::Type::Trigger});
    if (hasTruthTable()) props.append({PropertyDescriptor::Type::TruthTable});
    if (canChangeAppearance())    props.append({PropertyDescriptor::Type::Appearance});
    if (hasWirelessMode())  props.append({PropertyDescriptor::Type::WirelessModeSelector});
    return props;
} // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned QList<PropertyDescriptor>, never reached after the return above.

bool GraphicElement::canChangeAppearance() const
{
    return m_metadata.canChangeAppearance;
}

bool GraphicElement::rotatesGraphic() const
{
    return m_metadata.rotatesGraphic;
}

int GraphicElement::inputSize() const
{
    return m_ports.inputSize();
}

void GraphicElement::setPortSize(const int size, const bool isInput)
{
    const int minSize = isInput ? minInputSize() : minOutputSize();
    const int maxSize = isInput ? maxInputSize() : maxOutputSize();

    if ((size < minSize) || (size > maxSize)) {
        return;
    }

    // Adding/removing ports changes portsBoundingRect(), and thus boundingRect(): notify the
    // scene's spatial index before mutating so its stale bounds can't crash a later paint.
    prepareGeometryChange();

    if (isInput) {
        m_ports.resizeInputs(size);
    } else {
        m_ports.resizeOutputs(size);
    }

    updatePortsProperties();
}

void GraphicElement::setInputSize(const int size)
{
    setPortSize(size, true);
}

int GraphicElement::outputSize() const
{
    return m_ports.outputSize();
}

void GraphicElement::setOutputSize(const int size)
{
    setPortSize(size, false);
}

double GraphicElement::frequency() const
{
    return 0.0;
}

void GraphicElement::setFrequency(const double freq)
{
    Q_UNUSED(freq)
}

double GraphicElement::delay() const
{
    return 0.0;
}

void GraphicElement::setDelay(const double delay)
{
    Q_UNUSED(delay)
}

void GraphicElement::updateLogic()
{
    // Default no-op — decorative elements (Line, Text) have no simulation behaviour.
}

void GraphicElement::resetSimState()
{
    m_sim.reset(m_ports.outputs());
}

void GraphicElement::saveSimState(QVector<Status> &out) const
{
    for (int i = 0; i < static_cast<int>(m_sim.outputSize()); ++i) {
        out.append(m_sim.outputValue(i));
    }
}

void GraphicElement::restoreSimState(const QVector<Status> &in, int &cursor)
{
    for (int i = 0; i < static_cast<int>(m_sim.outputSize()); ++i) {
        if (cursor >= in.size()) {
            return; // LCOV_EXCL_LINE -- save/restore are always paired over the same element set
        }
        m_sim.setOutputValue(i, in.at(cursor++));
    }
}

void GraphicElement::connectPredecessor(const int inputIndex, GraphicElement *source, const int outputPort)
{
    m_sim.connectPredecessor(inputIndex, source, outputPort);
}

void GraphicElement::initSimulationVectors(const int inputCount, const int outputCount)
{
    m_sim.initVectors(inputCount, outputCount, m_ports.outputs());
}

int GraphicElement::minOutputSize() const
{
    return static_cast<int>(m_minOutputSize);
}

int GraphicElement::minInputSize() const
{
    return static_cast<int>(m_minInputSize);
}

int GraphicElement::maxOutputSize() const
{
    return static_cast<int>(m_maxOutputSize);
}

int GraphicElement::maxInputSize() const
{
    return static_cast<int>(m_maxInputSize);
}

void GraphicElement::setMaxInputSize(const int maxInputSize)
{
    m_maxInputSize = static_cast<quint64>(maxInputSize);
}

void GraphicElement::setMaxOutputSize(const int maxOutputSize)
{
    m_maxOutputSize = static_cast<quint64>(maxOutputSize);
}

void GraphicElement::setMinInputSize(const int minInputSize)
{
    m_minInputSize = static_cast<quint64>(minInputSize);
}

void GraphicElement::setMinOutputSize(const int minOutputSize)
{
    m_minOutputSize = static_cast<quint64>(minOutputSize);
}

void GraphicElement::highlight(const bool isSelected)
{
    const auto highlightPort = [isSelected](Port *port) {
        for (auto *connection : port->connections()) {
            // Skip connections already in the desired highlight state to avoid
            // triggering unnecessary repaints
            if (connection->highLight() == isSelected) {
                continue;
            }

            connection->setHighLight(isSelected);
        }
    };
    for (auto *port : inputs())  { highlightPort(port); }
    for (auto *port : outputs()) { highlightPort(port); }
}

void GraphicElement::retranslate()
{
    m_translatedName = ElementFactory::translatedName(m_elementType);

    setPortName(m_translatedName);
    setToolTip(m_translatedName);
}

void GraphicElement::loadFromDrop(const QString &fileName, const QString &contextDir)
{
    Q_UNUSED(fileName)
    Q_UNUSED(contextDir)
}
