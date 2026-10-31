// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/Element.h"

#include <cmath>
#include <memory>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QRegularExpression>
#include <QStyleOptionGraphicsItem>
#include <QSvgRenderer>
#include <QThread>

#include "App/Core/Common.h"
#include "App/Core/Constants.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/BodyPainter.h"
#include "App/Element/ElementTypeDescriptor.h"
#include "App/IO/SerializationContext.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

Element::Element(ElementType type, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_elementType(type)
    , m_descriptor(ElementTypeDescriptor::of(type))
    , m_parameters(m_descriptor.parameters)
{
    const ElementTypeDescriptor &descriptor = m_descriptor;
    m_appearance.seed(descriptor.defaultAppearances, descriptor.alternativeAppearances);
    m_elementGroup = descriptor.group;
    m_maxInputSize = descriptor.maxInputSize;
    m_maxOutputSize = descriptor.maxOutputSize;
    m_minInputSize = descriptor.minInputSize;
    m_minOutputSize = descriptor.minOutputSize;

    qCDebug(four) << "Setting flags of elements.";
    // ItemSendsGeometryChanges is required so itemChange() hears of moves and redraws the wires. Not ItemIsMovable:
    // SceneInteraction drags elements, snapping each to the grid, while a position set in code stays exact
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);

    qCDebug(four) << "Setting attributes.";
    m_label->setVisible(descriptor.hasLabel);
    // Font is applied lazily in updateLabel() to avoid expensive text layout
    // passes for elements that never display a label (e.g. IC sub-elements).
    m_label->setParentItem(this);
    m_label->setBrush(Qt::black);
    // Where the label goes is measured with the body (flushChanges())

    qCDebug(four) << "Including input and output ports.";
    Element::setInputSize(static_cast<int>(descriptor.minInputSize));
    Element::setOutputSize(static_cast<int>(descriptor.minOutputSize));

    updatePortsProperties();
    Element::updateTheme();

    // DeviceCoordinateCache reuses the rendered pixmap when the device transform changes,
    // giving a large speedup for elements that don't redraw on every pan/zoom
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    // The first pictures give the body and the pivot; a type with none (IC) is measured from its footprint alone
    if (m_appearance.hasDefaultAppearances()) {
        m_appearance.appearanceChanged();
    } else {
        pictureReplaced();
    }
}

Element::~Element() = default; // LCOV_EXCL_LINE — the Itanium ABI emits separate deleting/complete-object destructor variants for this virtual dtor; gcov attributes them to the same source line, and not every variant is exercised by a given delete call site.

ElementType Element::elementType() const
{
    return m_elementType;
}

ElementGroup Element::elementGroup() const
{
    return m_elementGroup;
}

QStringList Element::externalFiles() const
{
    return m_appearance.externalFiles();
}

void Element::reportFailedFiles() const
{
    m_appearance.reportFailedPicture();
}

const QVector<OutputPort *> &Element::outputs() const
{
    return m_ports.outputs();
}

InputPort *Element::inputPort(const int index) const
{
    return m_ports.inputPort(index);
}

OutputPort *Element::outputPort(const int index) const
{
    return m_ports.outputPort(index);
}

WirelessMode Element::wirelessMode() const
{
    return hasParam(ParamId::Wireless) ? static_cast<WirelessMode>(param<int>(ParamId::Wireless)) : WirelessMode::None;
}

void Element::setWirelessMode(const WirelessMode mode)
{
    setParam(ParamId::Wireless, static_cast<int>(mode));
}

bool Element::hasWirelessMode() const
{
    return offersParam(ParamId::Wireless);
}

bool Element::offersParam(const ParamId id) const
{
    const ParamSpec *spec = m_parameters.spec(id);
    return (spec != nullptr) && (!spec->offered || spec->offered(*this));
}

bool Element::setParam(const ParamId id, const ParamValue &value)
{
    const ParamSpec *spec = m_parameters.spec(id);
    if ((spec == nullptr) || (value.index() != spec->defaultValue.index())) {
        return false;
    }

    std::optional<ParamValue> taken = spec->accept ? spec->accept(*this, value) : std::optional<ParamValue>(value);
    if (!taken || (*taken == m_parameters.value(id))) {
        return false;
    }

    // A saved parameter is a change of the circuit, reported once with what the element derives from it; a transient
    // one (a button's press) changes only the picture, which the element reports itself.
    const ChangeBatch batch(*this, (spec->key != nullptr) ? ElementChanges(ElementChange::Parameter) : ElementChanges());
    const ParamValue previous = m_parameters.set(id, std::move(*taken));
    parameterChanged(id, previous);
    return true;
}

void Element::parameterChanged(const ParamId id, const ParamValue &previous)
{
    Q_UNUSED(id)
    Q_UNUSED(previous)
}

const QVector<InputPort *> &Element::inputs() const
{
    return m_ports.inputs();
}

QVector<Port *> Element::allPorts() const
{
    return m_ports.allPorts();
}

void Element::setInputs(const QVector<InputPort *> &inputs)
{
    m_ports.setInputs(inputs);
}

QRectF Element::boundingRect() const
{
    return m_boundsLatched ? m_bounds : measureBoundingRect();
}

QRectF Element::measureBoundingRect() const
{
    return outlineRect().united(m_body.decoration);
}

QRectF Element::outlineRect() const
{
    return portsBoundingRect().united(m_body.body);
}

QPainterPath Element::shape() const
{
    if (m_body.decoration.isNull()) {
        return QGraphicsObject::shape();
    }
    QPainterPath path;
    path.addRect(outlineRect());
    return path;
}

void Element::measureBody()
{
    m_body = BodyMeasure::installed().measure(*this);
}

void Element::pictureReplaced()
{
    const QSizeF previousSize = m_body.body.size();
    measureBody();

    // A size change grows/shrinks the bounds; DeviceCoordinateCache keeps the old device tile on a plain update(), so
    // reset the cache to force a full re-render and avoid a stale ghost of the previous picture.
    if (m_body.body.size() != previousSize) {
        invalidateRenderCache();
    }

    // The transform origin must follow the body's centre so that rotation stays centred on it; a flip, or a
    // non-rotatable element's oriented ports, are re-applied about it too.
    setTransformOriginPoint(m_body.centre);
    recentreOrientation();
    pictureChanged();
}

bool Element::syncBoundingRect()
{
    if (!m_boundsLatched) {
        return false;
    }

    const QRectF measured = measureBoundingRect();
    if (measured == m_bounds) {
        return false;
    }

    // Qt drops the old bounds from its index and repaints them while boundingRect() still returns them.
    prepareGeometryChange();
    m_bounds = measured;
    return true;
}

void Element::changed(const ElementChanges what)
{
    m_pendingChanges |= what;
    if (m_changeBatchDepth == 0) {
        flushChanges();
    }
}

void Element::pictureChanged()
{
    m_pictureChanged = true;
    if (m_changeBatchDepth == 0) {
        flushChanges();
    }
}

void Element::flushChanges()
{
    ElementChanges changes = std::exchange(m_pendingChanges, {});
    const bool repaint = std::exchange(m_pictureChanged, false);

    // The label first, which a body can hold (Text's): it follows the text, and the ports where it goes below them
    if (const LabelShape label = LabelMeasure::installed().measure(*this); label != m_labelShape) {
        m_labelShape = label;
        placeLabel();
        changes |= ElementChange::Label;
    }

    measureBody();
    if (syncBoundingRect()) {
        changes |= ElementChange::Body;
    }

    // A move alone leaves the picture as it is, and repainting it would discard the item's device cache.
    if (repaint || changes.testAnyFlags(~ElementChanges(ElementChange::Placement))) {
        update();
    }

    if (m_listener && changes) {
        m_listener->reportElementChange(id(), changes);
    }
}

Element::ChangeBatch::ChangeBatch(Element &element, const ElementChanges what)
    : m_element(element)
{
    ++m_element.m_changeBatchDepth;
    m_element.m_pendingChanges |= what;
}

Element::ChangeBatch::~ChangeBatch()
{
    if (--m_element.m_changeBatchDepth == 0) {
        m_element.flushChanges();
    }
}

QRectF Element::portsBoundingRect() const
{
    QRectF rectChildren;
    const auto children = childItems();

    for (auto *child : children) {
        if (auto *port = qgraphicsitem_cast<Port *>(child)) {
            rectChildren = rectChildren.united(mapRectFromItem(port, port->boundingRect()));
        }
    }

    return rectChildren;
}

QRectF Element::renderBodyBounds() const
{
    return portsBoundingRect().united(QRectF(0, 0, 64, 64));
}

void Element::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    BodyPainter::installed().paint(*painter, *this);
}

void Element::invalidateRenderCache()
{
    // Toggling the mode discards the cached device pixmap; re-enabling it forces the next
    // paint to re-render the whole item from scratch — the same full regen a pan/zoom
    // triggers. See the header for why a plain update() is not enough on a size change.
    setCacheMode(QGraphicsItem::NoCache);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

QPointF Element::snapToGrid(const QPointF &pos, const int step)
{
    // Rounded in floating point, halves away from zero as qRound() does: a file may hold any finite position, and one
    // past int's range has no int grid line. Adding zero turns the -0 that rounds a small negative value into the +0 an
    // int gives, which a saved file writes as other bytes.
    return {std::round(pos.x() / step) * step + 0.0, std::round(pos.y() / step) * step + 0.0};
}

Element::PortAttributes Element::describedPort(const char *letter, const char *description) const
{
    // One builder for the shown name and its source, so the two differ only by the description's translation
    const auto described = [letter](const QString &text) {
        return QString::fromUtf8(letter) + " (" + text + ')';
    };
    const QMetaObject *meta = metaObject();
    PortAttributes attributes;
    attributes.derivedName = PortAttributes::DerivedName{
        [described, meta, description] { return described(meta->tr(description, nullptr)); },
        described(QString::fromUtf8(description)),
    };
    return attributes;
}

void Element::setRotation(const qreal angle)
{
    // The ports a non-rotatable element turns, and the oriented picture of a rotatable one, are part of the turn.
    const ChangeBatch batch(*this, ElementChange::Placement);
    m_orientation.setRotation(angle);
}

void Element::rotatePorts()
{
    const ChangeBatch batch(*this, ElementChange::Placement);
    m_orientation.rotatePorts();
}

qreal Element::rotation() const
{
    return m_orientation.angle();
}

void Element::setFlippedX(const bool flipped)
{
    const ChangeBatch batch(*this, ElementChange::Placement);
    m_orientation.setFlippedX(flipped);
}

void Element::setFlippedY(const bool flipped)
{
    const ChangeBatch batch(*this, ElementChange::Placement);
    m_orientation.setFlippedY(flipped);
}

void Element::setAppearance(const bool defaultAppearance, const QString &fileName)
{
    const ChangeBatch batch(*this, ElementChange::Parameter);
    applyAppearance(defaultAppearance, fileName);
}

void Element::applyAppearance(const bool defaultAppearance, const QString &fileName)
{
    m_appearance.setAppearance(defaultAppearance, fileName);
}

void Element::setAppearanceAt(const int index, const QString &fileName)
{
    const ChangeBatch batch(*this, ElementChange::Parameter);
    m_appearance.setAppearanceAt(index, fileName);
}

QList<std::pair<int, QString>> Element::appearanceStates() const
{
    // Default: single appearance at index 0, translated in the "GraphicElement" context the catalogues hold
    return {{0, QCoreApplication::translate("GraphicElement", "Default")}};
}

namespace ElementPortLayout {

/// Applies the fields \a attributes sets to \a port, its place aside, leaving every unset field as the port has it.
void applyAttributes(Port *port, const Element::PortAttributes &attributes)
{
    Q_ASSERT(!(attributes.name && attributes.derivedName));
    if (attributes.derivedName) {
        port->setNameSource(attributes.derivedName->name, attributes.derivedName->source);
    } else if (attributes.name) {
        port->setName(*attributes.name);
    }
    if (attributes.required) {
        port->setRequired(*attributes.required);
    }
    if (attributes.defaultStatus) {
        port->setDefaultStatus(*attributes.defaultStatus);
    }
}

} // namespace ElementPortLayout

void Element::updatePortsProperties()
{
    const ChangeBatch batch(*this, ElementChange::Ports);
    layoutPorts();
}

void Element::load(QDataStream &stream, SerializationContext &context)
{
    const ChangeBatch batch(*this, ElementChange::Placement | ElementChange::Parameter | ElementChange::Label | ElementChange::Ports);
    loadState(stream, context);
}

void Element::layoutPorts()
{
    qCDebug(five) << "Updating port positions that belong to the IC.";

    // gridSize is 16 px; half that (8 px) is the port spacing unit so ports land
    // on sub-grid snap points that wires can reach when snapped to the same grid.
    const int step = Constants::gridSize / 2;

    // Inputs are pinned to the left edge (x=0) of the 64 px body and outputs to the right (x=64), unless a port's
    // attributes place it
    const auto layOut = [this, step](const auto &ports, const qreal x, const auto &attributesOf) {
        // Centre the port column vertically around y=32 (the mid-point of a 64 px body).
        // The first port starts at 32 - (count-1)*step so all ports are symmetrically distributed.
        int y = 32 - (static_cast<int>(ports.size()) * step) + step;

        for (int index = 0; index < static_cast<int>(ports.size()); ++index) {
            Port *port = ports.at(index);
            const std::optional<PortAttributes> attributes = attributesOf(index);
            port->setPos((attributes && attributes->position) ? *attributes->position : QPointF(x, y));
            if (attributes) {
                ElementPortLayout::applyAttributes(port, *attributes);
            }

            // Non-rotatable elements keep the pixmap fixed and orient their ports in place,
            // applying the current rotation and flip about the element centre.
            if (!rotatesGraphic()) {
                m_orientation.orientPort(port);
            }

            // Ports are spaced 2*step (16 px) apart so they align with the
            // full grid and remain easily connectable with straight wires.
            y += step * 2;
        }
    };

    layOut(m_ports.inputs(), 0, [this](const int index) { return inputAttributes(index); });
    layOut(m_ports.outputs(), 64, [this](const int index) { return outputAttributes(index); });
}

std::optional<Element::PortAttributes> Element::inputAttributes(const int index) const
{
    Q_UNUSED(index)
    return std::nullopt;
}

std::optional<Element::PortAttributes> Element::outputAttributes(const int index) const
{
    Q_UNUSED(index)
    return std::nullopt;
}

std::optional<Element::PortAttributes> Element::attributesAt(const QList<PortAttributes> &table, const int index)
{
    if ((index < 0) || (index >= table.size())) {
        return std::nullopt;
    }
    return table.at(index);
}

void Element::refresh()
{
}

QVariant Element::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneHasChanged) {
        m_listener = dynamic_cast<ElementChangeListener *>(scene());
        // Qt indexes the bounds of an item in a scene as boundingRect() first gives them, so from here they hold still
        // until a change tells Qt they move (syncBoundingRect()).
        m_boundsLatched = (scene() != nullptr);
        if (m_boundsLatched) {
            m_bounds = measureBoundingRect();
        }
    }

    // Guard against changes fired during construction before the item is added to a scene
    if (!scene()) {
        return QGraphicsItem::itemChange(change, value);
    }

    // Any geometric change (move, rotate, shear) requires redrawing all connected wires. An element has no parent
    // item, so its own move is its ports' only scene move.
    if ((change == ItemPositionHasChanged) || (change == ItemRotationHasChanged) || (change == ItemTransformHasChanged)) {
        qCDebug(four) << "Moves wires.";
        for (auto *port : allPorts()) {
            port->updateConnections();
        }
        changed(ElementChange::Placement);
    }

    if (change == ItemSelectedHasChanged) {
        m_selected = value.toBool();
        // Propagate selection highlight to all connected wires so users see which
        // signals are attached to the selected element
        highlight(m_selected);
    }

    return QGraphicsItem::itemChange(change, value);
}

bool Element::sceneEvent(QEvent *event)
{
    // Swallow Ctrl+click events so the scene's rubber-band selection logic can handle them
    // without the element intercepting the press and starting a move instead
    if (event->type() == QEvent::GraphicsSceneMousePress || event->type() == QEvent::GraphicsSceneMouseRelease) {
        if (auto mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent *>(event)) {
            if (mouseEvent->modifiers().testFlag(Qt::ControlModifier)) {
                return true;
            }
        }
    }

    return QGraphicsItem::sceneEvent(event);
}

void Element::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (hasLabel()) {
        event->accept();
        emit inlineEditRequested(this);
        return;
    }

    QGraphicsItem::mouseDoubleClickEvent(event);
}

QKeySequence Element::trigger() const
{
    return m_trigger;
}

void Element::setTrigger(const QKeySequence &trigger)
{
    // The label shows the trigger.
    const ChangeBatch batch(*this, ElementChange::Parameter);
    m_trigger = trigger;
    updateLabel();
}

QString Element::genericProperties()
{
    return {};
}

void Element::updateLabel()
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
    if (!label.isEmpty() && m_label->font() != LabelMeasure::installed().font()) {
        m_label->setFont(LabelMeasure::installed().font());
    }

    m_label->setText(label);
    changed(ElementChange::Label);
}

void Element::placeLabel()
{
    // The label's own turn (an IC's name stands along the chip side) applies first, about its origin
    m_label->setRotation(m_labelShape.rotation);

    // Non-rotatable elements never transform their graphic (or its children), so their label is
    // already upright as authored.
    if (!rotatesGraphic()) {
        m_label->setPos(m_labelShape.anchor);
        return;
    }

    // The label is a child item and inherits the element's Flip∘Rotate. Left alone, its anchor
    // point (the shape's anchor, in the element's un-rotated local frame) would orbit around the
    // rotation pivot along with everything else -- fine for ports/pins, but it leaves the label
    // sweeping past wherever the pivot's "below"/"beside" edge currently points to, which for a
    // wide/tall body (or once ports are packed along an edge) can land it squarely on a pin.
    // Counter-rotate the anchor's *position* about the same pivot the element itself rotates
    // about (pixmapCenter()), so it stays fixed in world space -- built by replaying the exact
    // transform the element applies to its children (its own extra transform(), i.e. the flip
    // matrix already set by ElementOrientation::applyFlipTransform(), composed with a rotation
    // about that pivot matching rotation()) and inverting it, rather than hand-deriving the
    // reversed composition.
    const QPointF pivot = pixmapCenter();

    QTransform pivotRotate;
    pivotRotate.translate(pivot.x(), pivot.y());
    pivotRotate.rotate(rotation());
    pivotRotate.translate(-pivot.x(), -pivot.y());

    const QTransform elementRotateFlip = pivotRotate * transform();
    m_label->setPos(elementRotateFlip.inverted().map(m_labelShape.anchor));

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
    m_label->setTransform(t);
}

void Element::setLabel(const QString &label)
{
    m_labelText = label;
    updateLabel();
}

QString Element::label() const
{
    return m_labelText;
}

QRectF Element::labelSceneBoundingRect() const
{
    return m_label->sceneBoundingRect();
}

QString Element::labelShown() const
{
    return m_label->text();
}

QRectF Element::labelBoundingRect() const
{
    if (m_labelShape.box.isEmpty()) {
        return {};
    }
    return m_label->mapRectToParent(QRectF(QPointF(0, 0), m_labelShape.box));
}

// Color cycle (forward): White → Red → Green → Blue → Purple → White
// These two functions implement previous/next steps in that cycle so the
// property editor can wrap around without knowing the full list
QString Element::previousColor() const
{
    if (color() == "White") return "Purple";
    if (color() == "Red") return "White";
    if (color() == "Green") return "Red";
    if (color() == "Blue") return "Green";
    if (color() == "Purple") return "Blue";
    return "White"; // Standard
}

QString Element::nextColor() const
{
    if (color() == "White") return "Red";
    if (color() == "Red") return "Green";
    if (color() == "Green") return "Blue";
    if (color() == "Blue") return "Purple";
    if (color() == "Purple") return "White";
    return "White"; // Standard
}

void Element::updateTheme()
{
    const ThemeAttributes theme = ThemeManager::attributes();

    m_label->setBrush(theme.m_elementLabelColor);

    for (auto *input : m_ports.inputs()) {
        input->updateTheme();
    }

    for (auto *output : m_ports.outputs()) {
        output->updateTheme();
    }

    pictureChanged();
}

bool Element::isValid()
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

bool Element::hasColors() const
{
    return offersParam(ParamId::Colour);
}

bool Element::hasTrigger() const
{
    return m_descriptor.hasTrigger;
}

void Element::setColor(const QString &color)
{
    setParam(ParamId::Colour, color);
}

QString Element::color() const
{
    return hasParam(ParamId::Colour) ? param<QString>(ParamId::Colour) : QString();
}

int Element::colorNameToIndex(const QString &color)
{
    if (color == "White")  { return 0; }
    if (color == "Red")    { return 1; }
    if (color == "Green")  { return 2; }
    if (color == "Blue")   { return 3; }
    if (color == "Purple") { return 4; }
    return 0;
}

void Element::setAudio(const QString &audio)
{
    setParam(ParamId::AudioFile, audio);
}

QString Element::audio() const
{
    return hasParam(ParamId::AudioFile) ? param<QString>(ParamId::AudioFile) : QString();
}

bool Element::hasFrequency() const
{
    return offersParam(ParamId::Frequency);
}

bool Element::hasDelay() const
{
    return offersParam(ParamId::Delay);
}

bool Element::hasLabel() const
{
    return m_descriptor.hasLabel;
}

bool Element::hasTruthTable() const
{
    return offersParam(ParamId::Table);
}

bool Element::hasAudioBox() const
{
    return offersParam(ParamId::AudioFile);
}

bool Element::hasVolume() const
{
    return offersParam(ParamId::Volume);
}

float Element::volume() const
{
    // The spec keeps a float's precision, so narrowing it back is exact
    return hasParam(ParamId::Volume) ? static_cast<float>(param<double>(ParamId::Volume)) : 0.0f;
}

void Element::setVolume(const float vol)
{
    setParam(ParamId::Volume, static_cast<double>(vol));
}

QList<PropertyDescriptor> Element::editableProperties() const
{
    QList<PropertyDescriptor> props;
    if (hasLabel())      props.append({PropertyDescriptor::Type::Label});
    if (hasColors())     props.append({PropertyDescriptor::Type::Color});
    if (hasFrequency())  props.append({PropertyDescriptor::Type::Frequency});
    if (hasDelay())      props.append({PropertyDescriptor::Type::Delay});
    if (hasAudioBox())   props.append({PropertyDescriptor::Type::AudioBox});
    if (hasVolume())     props.append({PropertyDescriptor::Type::Volume});
    if (hasTrigger())    props.append({PropertyDescriptor::Type::Trigger});
    if (hasTruthTable()) props.append({PropertyDescriptor::Type::TruthTable});
    if (canChangeAppearance())    props.append({PropertyDescriptor::Type::Appearance});
    if (hasWirelessMode())  props.append({PropertyDescriptor::Type::WirelessModeSelector});
    return props;
} // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned QList<PropertyDescriptor>, never reached after the return above.

bool Element::canChangeAppearance() const
{
    return m_descriptor.canChangeAppearance;
}

bool Element::rotatesGraphic() const
{
    return m_descriptor.rotatesGraphic;
}

int Element::inputSize() const
{
    return m_ports.inputSize();
}

void Element::setPortSize(const int size, const bool isInput)
{
    const int minSize = isInput ? minInputSize() : minOutputSize();
    const int maxSize = isInput ? maxInputSize() : maxOutputSize();

    if ((size < minSize) || (size > maxSize)) {
        return;
    }

    // The ports added or removed and the layout of the whole set are one change of the ports.
    const ChangeBatch batch(*this, ElementChange::Ports);

    if (isInput) {
        m_ports.resizeInputs(size);
    } else {
        m_ports.resizeOutputs(size);
    }

    updatePortsProperties();
}

void Element::setInputSize(const int size)
{
    setPortSize(size, true);
}

int Element::outputSize() const
{
    return m_ports.outputSize();
}

void Element::setOutputSize(const int size)
{
    setPortSize(size, false);
}

double Element::frequency() const
{
    return hasParam(ParamId::Frequency) ? param<double>(ParamId::Frequency) : 0.0;
}

void Element::setFrequency(const double freq)
{
    setParam(ParamId::Frequency, freq);
}

bool Element::acceptsFrequency(const double freq) const
{
    const ParamSpec *spec = m_parameters.spec(ParamId::Frequency);
    return (spec != nullptr) && (!spec->accept || spec->accept(*this, freq).has_value());
}

double Element::delay() const
{
    return hasParam(ParamId::Delay) ? param<double>(ParamId::Delay) : 0.0;
}

void Element::setDelay(const double delay)
{
    setParam(ParamId::Delay, delay);
}

void Element::updateLogic()
{
    // Default no-op — decorative elements (Line, Text) have no simulation behaviour.
}

void Element::resetSimState()
{
    m_sim.reset(m_ports.outputs());
}

void Element::saveSimState(QVector<Status> &out) const
{
    for (int i = 0; i < static_cast<int>(m_sim.outputSize()); ++i) {
        out.append(m_sim.outputValue(i));
    }
}

void Element::restoreSimState(const QVector<Status> &in, int &cursor)
{
    for (int i = 0; i < static_cast<int>(m_sim.outputSize()); ++i) {
        if (cursor >= in.size()) {
            return; // LCOV_EXCL_LINE -- save/restore are always paired over the same element set
        }
        m_sim.setOutputValue(i, in.at(cursor++));
    }
}

void Element::connectPredecessor(const int inputIndex, Element *source, const int outputPort)
{
    m_sim.connectPredecessor(inputIndex, source, outputPort);
}

void Element::initSimulationVectors(const int inputCount, const int outputCount, const bool keepOutputs)
{
    m_sim.initVectors(inputCount, outputCount, m_ports.outputs(), keepOutputs);
}

int Element::minOutputSize() const
{
    return static_cast<int>(m_minOutputSize);
}

int Element::minInputSize() const
{
    return static_cast<int>(m_minInputSize);
}

int Element::maxOutputSize() const
{
    return static_cast<int>(m_maxOutputSize);
}

int Element::maxInputSize() const
{
    return static_cast<int>(m_maxInputSize);
}

void Element::setMaxInputSize(const int maxInputSize)
{
    m_maxInputSize = static_cast<quint64>(maxInputSize);
}

void Element::setMaxOutputSize(const int maxOutputSize)
{
    m_maxOutputSize = static_cast<quint64>(maxOutputSize);
}

void Element::setMinInputSize(const int minInputSize)
{
    m_minInputSize = static_cast<quint64>(minInputSize);
}

void Element::setMinOutputSize(const int minOutputSize)
{
    m_minOutputSize = static_cast<quint64>(minOutputSize);
}

void Element::highlight(const bool isSelected)
{
    for (auto *port : allPorts()) {
        for (auto *connection : port->connections()) {
            // Skip connections already in the desired highlight state to avoid
            // triggering unnecessary repaints
            if (connection->highLight() == isSelected) {
                continue;
            }

            connection->setHighLight(isSelected);
        }
    }
}

QString Element::translatedName() const
{
    return QCoreApplication::translate(m_descriptor.trContext, m_descriptor.translatedName);
}

QString Element::helpText() const
{
    return translatedName();
}

void Element::languageChanged()
{
}

void Element::loadFromDrop(const QString &fileName, const QString &contextDir)
{
    Q_UNUSED(fileName)
    Q_UNUSED(contextDir)
}
