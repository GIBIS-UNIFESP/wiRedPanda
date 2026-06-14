// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElement.h"

#include <cmath>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPixmap>
#include <QRegularExpression>
#include <QStyleOptionGraphicsItem>
#include <QSvgRenderer>
#include <QThread>

#include "App/Core/Common.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementMetadata.h"
#include "App/IO/SerializationContext.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Scene.h"

/// Shared label font — constructed once to avoid repeated QFont creation and fontconfig lookups.
static const QFont &labelFont()
{
    static const QFont font = [] {
        QFont f("Sans Serif");
        f.setBold(true);
        return f;
    }();
    return font;
}

/// Cache decoded pixmaps by resolved path so each image is loaded from disk only once.
static QHash<QString, QPixmap> &pixmapCache()
{
    static QHash<QString, QPixmap> cache;
    return cache;
}

/// Cache of orientation-variant pixmaps keyed by "<resolvedPath>|<canonicalAngle>|<flipX><flipY>"
/// so the SVG text correction is rendered only once per appearance and rotation/flip state.
/// Both this and pixmapCache() are plain static QHashes with no locking: GraphicElement pixmaps are
/// only built and read on the GUI thread, so concurrent access does not occur.
static QHash<QString, QPixmap> &orientedPixmapCache()
{
    static QHash<QString, QPixmap> cache;
    return cache;
}

/// Rewrites \a svgBytes so every <text> element is counter-oriented about its own centre for the
/// element's current rotation \a angle and \a flipX / \a flipY. Pre-applying the inverse of the
/// item transform (Rotate(-angle) after the flip) means that after the element's own item-level
/// rotation + flip the glyphs end up upright and unmirrored, while still travelling to the
/// rotated/mirrored side. Transforming about each text's *own* centre composes cleanly with any
/// parent transform, so it is correct regardless of how the text is nested. Returns the input
/// unchanged on any parse failure.
static QByteArray orientSvgTextNodes(const QByteArray &svgBytes, const qreal angle, const bool flipX, const bool flipY)
{
    static const QRegularExpression textTag(QStringLiteral("<text\\b[^>]*>"));
    static const QRegularExpression idAttr(QStringLiteral("\\bid\\s*=\\s*\"([^\"]*)\""));
    static const QRegularExpression transformAttr(QStringLiteral("\\btransform\\s*="));

    QString svg = QString::fromUtf8(svgBytes);

    // Pass 1: ensure every <text> opening tag carries an id so boundsOnElement() can address it.
    // Inkscape already ids these, but be defensive for hand-written assets.
    {
        QString out;
        int last = 0;
        int generated = 0;
        auto matches = textTag.globalMatch(svg);
        while (matches.hasNext()) {
            const auto match = matches.next();
            out += svg.mid(last, match.capturedStart() - last);
            QString tag = match.captured(0);
            if (!idAttr.match(tag).hasMatch()) {
                // Insert after "<text" (5 chars) so the new attribute precedes the existing ones.
                tag.insert(5, QStringLiteral(" id=\"wpflip-text-%1\"").arg(generated++));
            }
            out += tag;
            last = match.capturedEnd();
        }
        out += svg.mid(last);
        svg = out;
    }

    QSvgRenderer probe(svg.toUtf8());
    if (!probe.isValid()) {
        return svgBytes;
    }

    const qreal sx = flipX ? -1.0 : 1.0;
    const qreal sy = flipY ? -1.0 : 1.0;

    // Pass 2: inject a counter-orientation transform on each <text>, pivoting about that text's
    // own centre. The item applies Flip ∘ Rotate(angle) to the pixmap content, so the inverse the
    // glyph must carry is Rotate(-angle) ∘ Flip (rotate outer, scale inner). The rotate term is
    // omitted at angle 0 and the scale term when unflipped, so the flip-only output is unchanged.
    QString out;
    int last = 0;
    auto matches = textTag.globalMatch(svg);
    while (matches.hasNext()) {
        const auto match = matches.next();
        out += svg.mid(last, match.capturedStart() - last);
        QString tag = match.captured(0);

        const auto idMatch = idAttr.match(tag);
        if (idMatch.hasMatch() && !transformAttr.match(tag).hasMatch()) {
            const QRectF bounds = probe.boundsOnElement(idMatch.captured(1));
            if (!bounds.isEmpty()) {
                const QPointF c = bounds.center();
                QString ops;
                if (angle != 0.0) {
                    ops += QStringLiteral("rotate(%1) ").arg(-angle);
                }
                if (flipX || flipY) {
                    ops += QStringLiteral("scale(%1,%2) ").arg(sx).arg(sy);
                }
                const QString transform =
                    QStringLiteral(" transform=\"translate(%1,%2) %3translate(%4,%5)\"")
                        .arg(c.x()).arg(c.y()).arg(ops).arg(-c.x()).arg(-c.y());
                tag.insert(5, transform);
            }
        }

        out += tag;
        last = match.capturedEnd();
    }
    out += svg.mid(last);

    return out.toUtf8();
}

/// Renders an orientation-variant pixmap for the SVG at \a resolvedPath with its <text> labels
/// counter-oriented for the current rotation \a angle and flip, cached per path + angle + flip
/// state. Returns a null pixmap on failure so callers can fall back to the plain base pixmap
/// (which the item transform then rotates/flips).
static QPixmap orientedSvgPixmap(const QString &resolvedPath, const qreal angle, const bool flipX, const bool flipY)
{
    // Canonicalize to [0,360) so equivalent rotations (e.g. -90 and 270, which render identically)
    // share one cache entry instead of growing the key space without bound — MCP can request any
    // angle and rotate-left produces negatives.
    const qreal canonAngle = std::fmod(std::fmod(angle, 360.0) + 360.0, 360.0);
    const QString key = resolvedPath + QLatin1Char('|')
        + QString::number(canonAngle) + QLatin1Char('|')
        + (flipX ? QLatin1Char('1') : QLatin1Char('0'))
        + (flipY ? QLatin1Char('1') : QLatin1Char('0'));

    auto &cache = orientedPixmapCache();
    const auto it = cache.constFind(key);
    if (it != cache.constEnd()) {
        return *it;
    }

    QFile file(resolvedPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    const QByteArray raw = file.readAll();
    // No baked-in <text> → nothing to correct; return null so the caller keeps the plain
    // item-flipped base pixmap (identical to legacy behaviour, no redundant re-render).
    if (!raw.contains("<text")) {
        return {};
    }
    const QByteArray modified = orientSvgTextNodes(raw, canonAngle, flipX, flipY);

    QSvgRenderer renderer(modified);
    if (!renderer.isValid()) {
        return {};
    }

    QPixmap pixmap(renderer.defaultSize());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    renderer.render(&painter);
    painter.end();

    cache.insert(key, pixmap);
    return pixmap;
}

GraphicElement::GraphicElement(ElementType type, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_elementType(type)
{
    const auto &metadata = ElementMetadataRegistry::metadata(type);
    m_defaultAppearances = metadata.defaultAppearances;
    m_pixmapPath = metadata.pixmapPath();

    // For elements whose pixmapPath is theme-dependent (e.g. memory elements),
    // defaultAppearances may be left empty in the metadata to avoid evaluating the
    // theme path during static initialisation (before QApplication exists).
    // Populate it lazily here from the now-correct pixmapPath.
    if (m_defaultAppearances.isEmpty() && !m_pixmapPath.isEmpty()) {
        m_defaultAppearances = QStringList({m_pixmapPath});
    }

    m_alternativeAppearances = metadata.alternativeAppearances.isEmpty() ? m_defaultAppearances : metadata.alternativeAppearances;
    m_titleText = QCoreApplication::translate(metadata.trContext, metadata.titleText);
    m_translatedName = QCoreApplication::translate(metadata.trContext, metadata.translatedName);
    m_elementGroup = metadata.group;
    m_hasColors = metadata.hasColors;
    m_maxInputSize = metadata.maxInputSize;
    m_maxOutputSize = metadata.maxOutputSize;
    m_minInputSize = metadata.minInputSize;
    m_minOutputSize = metadata.minOutputSize;

    qCDebug(four) << "Setting flags of elements.";
    // ItemSendsGeometryChanges is required so itemChange() receives ItemPositionChange and
    // can snap movement to the grid
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);

    qCDebug(four) << "Setting attributes.";
    m_label->setVisible(metadata.hasLabel);
    // Font is applied lazily in updateLabel() to avoid expensive text layout
    // passes for elements that never display a label (e.g. IC sub-elements).
    // 64 px below origin keeps the label below the standard 64×64 element body
    m_label->setPos(0, 64);
    m_label->setParentItem(this);
    m_label->setBrush(Qt::black);

    setPortName(m_translatedName);
    setToolTip(m_translatedName);

    qCDebug(four) << "Including input and output ports.";
    GraphicElement::setInputSize(static_cast<int>(metadata.minInputSize));
    GraphicElement::setOutputSize(static_cast<int>(metadata.minOutputSize));

    GraphicElement::updatePortsProperties();
    GraphicElement::updateTheme();

    // DeviceCoordinateCache reuses the rendered pixmap when the device transform changes,
    // giving a large speedup for elements that don't redraw on every pan/zoom
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    if (!m_defaultAppearances.isEmpty()) {
        setPixmap(0);
    }
}

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
    return m_pixmap;
}

QStringList GraphicElement::externalFiles() const
{
    QStringList result;
    for (int i = 0; i < m_alternativeAppearances.size() && i < m_defaultAppearances.size(); ++i) {
        const QString &appearance = m_alternativeAppearances.at(i);
        if (appearance != m_defaultAppearances.at(i) && !appearance.startsWith(":/")) {
            result.append(appearance);
        }
    }
    return result;
}

void GraphicElement::setPixmap(const int index)
{
    setPixmap(m_usingDefaultAppearance ? m_defaultAppearances.at(index) : m_alternativeAppearances.at(index));
}

void GraphicElement::setPixmap(const QString &pixmapPath)
{
    // Skip if unchanged to avoid redundant loads and cache invalidation
    if (pixmapPath.isEmpty() || (pixmapPath == m_currentPixmapPath)) {
        return;
    }

    QString path = pixmapPath;

    // Qt resource paths start with ":/"; anything else is a filesystem path
    // relative to the project's working directory (where the .panda file lives).
    // Try the path as-is against contextDir first; if not found, fall back to
    // just the filename — handles cross-platform absolute paths from old files.
    if (!path.startsWith(":/")) {
        const QString contextDir = Scene::resolveContextDir(this);
        if (contextDir.isEmpty()) {
            return;
        }
        const QDir dir(contextDir);
        const QString resolved = dir.filePath(path);

        if (!QFileInfo::exists(resolved)) {
            path = dir.filePath(QFileInfo(QString(path).replace('\\', '/')).fileName());
        } else {
            path = resolved;
        }
    }

    auto &cache = pixmapCache();
    auto it = cache.constFind(path);
    if (it != cache.constEnd()) {
        m_basePixmap = *it;
    } else if (m_basePixmap.load(path)) {
        cache.insert(path, m_basePixmap);
    } else {
        const QFileInfo info(path);
        const QString reason = !info.exists()
                                   ? tr("File does not exist")
                                   : !info.isReadable()
                                         ? tr("File is not readable")
                                         : tr("Unknown reason");

        // Load the default appearance so the element remains renderable before the exception unwinds
        m_basePixmap.load(m_defaultAppearances.constFirst());
        m_pixmap = m_basePixmap;
        qCDebug(zero) << "Problem loading pixmapPath: " << path;
        throw PANDACEPTION("Couldn't load pixmap: %1 (%2)", path, reason);
    }

    m_resolvedPixmapPath = path;
    // Derive the displayed pixmap from the base, swapping in a text-corrected variant when the
    // element is rotated or flipped.
    applyPixmapOrientation();

    // The transform origin must be updated whenever the pixmap changes so that
    // rotation and scale operations remain centred on the new image
    setTransformOriginPoint(pixmapCenter());
    update();

    m_currentPixmapPath = pixmapPath;
}

const QVector<QNEOutputPort *> &GraphicElement::outputs() const
{
    return m_outputPorts;
}

QNEInputPort *GraphicElement::inputPort(const int index)
{
    if (index < 0 || index >= m_inputPorts.size()) {
        return nullptr;
    }
    return m_inputPorts.at(index);
}

QNEOutputPort *GraphicElement::outputPort(const int index)
{
    if (index < 0 || index >= m_outputPorts.size()) {
        return nullptr;
    }
    return m_outputPorts.at(index);
}

WirelessMode GraphicElement::wirelessMode() const
{
    return WirelessMode::None;
}

bool GraphicElement::hasWirelessMode() const
{
    return false;
}

const QVector<QNEInputPort *> &GraphicElement::inputs() const
{
    return m_inputPorts;
}

QVector<QNEPort *> GraphicElement::allPorts() const
{
    QVector<QNEPort *> result;
    result.reserve(m_inputPorts.size() + m_outputPorts.size());
    for (auto *p : m_inputPorts)  { result.append(p); }
    for (auto *p : m_outputPorts) { result.append(p); }
    return result;
}

void GraphicElement::setInputs(const QVector<QNEInputPort *> &inputs)
{
    m_inputPorts = inputs;
}

QPointF GraphicElement::pixmapCenter() const
{
    return QRectF(pixmap().rect()).center();
}

QRectF GraphicElement::boundingRect() const
{
    return portsBoundingRect().united(pixmap().rect());
}

QRectF GraphicElement::portsBoundingRect() const
{
    QRectF rectChildren;
    const auto children = childItems();

    for (auto *child : children) {
        if (auto *port = qgraphicsitem_cast<QNEPort *>(child)) {
            rectChildren = rectChildren.united(mapRectFromItem(port, port->boundingRect()));
        }
    }

    return rectChildren;
}

void GraphicElement::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    if (isSelected()) {
        painter->save();
        painter->setBrush(m_selectionBrush);
        // 0.5 pen width keeps the selection outline thin regardless of zoom level
        painter->setPen(QPen(m_selectionPen, 0.5, Qt::SolidLine));
        // Corner radius of 5 matches the visual rounding used on element bodies
        painter->drawRoundedRect(boundingRect(), 5, 5);
        painter->restore();
    }

    // Pixmap origin is always (0,0) in item coordinates; the transform origin
    // (centre of the pixmap) is set separately via setTransformOriginPoint().
    painter->drawPixmap(QPoint(0, 0), pixmap());
}

void GraphicElement::addPort(const QString &name, const bool isOutput)
{
    // No max-size guard here — setInputSize()/setOutputSize() already enforce
    // min/max constraints before calling this method.  The serializer also
    // calls addPort() and needs to create whatever ports the file contains.
    qCDebug(four) << "New port.";
    QNEPort *port = nullptr;

    if (isOutput) {
        m_outputPorts.push_back(new QNEOutputPort(this));
        port = m_outputPorts.constLast();
        port->setIndex(outputSize() - 1);
    } else {
        m_inputPorts.push_back(new QNEInputPort(this));
        port = m_inputPorts.constLast();
        port->setIndex(inputSize() - 1);
    }

    port->setGraphicElement(this);
    port->setName(name);
    port->show();
}

void GraphicElement::addInputPort(const QString &name)
{
    addPort(name, false);
}

void GraphicElement::addOutputPort(const QString &name)
{
    addPort(name, true);
}

void GraphicElement::setPortName(const QString &name)
{
    setObjectName(name);
}

void GraphicElement::setRotation(const qreal angle)
{
    // Keep angle in [0, 360) to avoid accumulated floating-point drift across many rotations
    m_angle = std::fmod(angle, 360);
    // Rotatable elements rotate the entire QGraphicsItem (pixmap + ports move together), then
    // swap in a pixmap whose baked-in SVG <text> is counter-rotated so the labels stay upright.
    // Non-rotatable elements (inputs/outputs) keep the pixmap fixed and only spin ports
    // around the element centre so connections track the correct positions.
    if (rotatesGraphic()) {
        QGraphicsItem::setRotation(m_angle);
        applyPixmapOrientation();
    } else {
        rotatePorts();
    }
}

void GraphicElement::rotatePorts()
{
    for (auto *port : allPorts()) {
        applyPortOrientation(port);
    }
}

void GraphicElement::applyPortOrientation(QNEPort *port)
{
    // Non-rotatable elements keep their pixmap fixed and instead transform each port about the
    // pixmap centre, so the port moves to the rotated/mirrored side while the graphic stays
    // upright. The single combined transform encodes both rotation and the flip flags, so all
    // three triggers (updatePortsProperties, setRotation, setFlippedX/Y) produce the same result.
    port->setRotation(0);
    port->setTransformOriginPoint(QPointF());

    // pixmapCenter() expressed in the port's local frame. port->pos() is the port's base
    // position — flip and rotation use the transform and never move pos — so this is stable
    // and independent of any transform already on the port (keeps the operation involutive).
    const QPointF origin = pixmapCenter() - port->pos();

    QTransform t;
    t.translate(origin.x(), origin.y());
    t.rotate(m_angle);
    t.scale(m_flippedX ? -1 : 1, m_flippedY ? -1 : 1);
    t.translate(-origin.x(), -origin.y());
    port->setTransform(t);

    port->updateConnections();
}

qreal GraphicElement::rotation() const
{
    return m_angle;
}

void GraphicElement::setFlippedX(const bool flipped)
{
    m_flippedX = flipped;
    applyFlipTransform();
}

void GraphicElement::setFlippedY(const bool flipped)
{
    m_flippedY = flipped;
    applyFlipTransform();
}

void GraphicElement::applyFlipTransform()
{
    // Non-rotatable elements (inputs/outputs) keep their pixmap fixed and mirror only their
    // ports — exactly as rotation does for them — so a flipped pushbutton/display never renders
    // its graphic reversed. rotatePorts() re-applies the combined rotation+flip orientation.
    if (!rotatesGraphic()) {
        rotatePorts();
        return;
    }

    if (!m_flippedX && !m_flippedY) {
        setTransform(QTransform());
    } else {
        const auto center = pixmapCenter();
        QTransform t;
        t.translate(center.x(), center.y());
        t.scale(m_flippedX ? -1 : 1, m_flippedY ? -1 : 1);
        t.translate(-center.x(), -center.y());
        setTransform(t);
    }

    // The item transform above mirrors the whole pixmap (and moves the ports). Swap in a pixmap
    // whose baked-in SVG <text> is pre-counter-oriented so the glyphs read upright after the
    // rotation + flip while still moving to the opposite side. No-op for an upright, unflipped or
    // non-SVG pixmap.
    applyPixmapOrientation();
    update();
}

void GraphicElement::applyPixmapOrientation()
{
    // Only rotatable elements transform their graphic, so only they need the baked <text> labels
    // counter-oriented. A non-rotatable element keeps its icon upright (it moves only its ports),
    // so its text — if any — must render as authored and never be counter-oriented.
    const bool oriented = rotatesGraphic() && (m_flippedX || m_flippedY || (m_angle != 0.0));
    if (oriented && m_resolvedPixmapPath.endsWith(QLatin1String(".svg"), Qt::CaseInsensitive)) {
        const QPixmap variant = orientedSvgPixmap(m_resolvedPixmapPath, m_angle, m_flippedX, m_flippedY);
        m_pixmap = variant.isNull() ? m_basePixmap : variant;
    } else {
        m_pixmap = m_basePixmap;
    }
    update();
}

void GraphicElement::setAppearance(const bool defaultAppearance, const QString &fileName)
{
    if (defaultAppearance) {
        m_alternativeAppearances = m_defaultAppearances;
    } else {
        m_alternativeAppearances[0] = fileName;
    }

    m_usingDefaultAppearance = defaultAppearance;
    setPixmap(0);
}

void GraphicElement::setAppearanceAt(const int index, const QString &fileName)
{
    if (index < 0 || index >= m_alternativeAppearances.size()) {
        return;
    }

    if (fileName.isEmpty()) {
        m_alternativeAppearances[index] = m_defaultAppearances.at(index);
    } else {
        m_alternativeAppearances[index] = fileName;
    }

    m_usingDefaultAppearance = (m_alternativeAppearances == m_defaultAppearances);
    setPixmap(index);
}

QList<QPair<int, QString>> GraphicElement::appearanceStates() const
{
    // Default: single appearance at index 0
    return {{0, tr("Default")}};
}

void GraphicElement::updatePortsProperties()
{
    qCDebug(five) << "Updating port positions that belong to the IC.";

    // gridSize is 16 px; half that (8 px) is the port spacing unit so ports land
    // on sub-grid snap points that wires can reach when snapped to the same grid.
    const int step = Scene::gridSize / 2;

    if (!m_inputPorts.isEmpty()) {
        // Centre the port column vertically around y=32 (the mid-point of a 64 px body).
        // The first port starts at 32 - (count-1)*step so all ports are symmetrically distributed.
        int y = 32 - (static_cast<int>(m_inputPorts.size()) * step) + step;

        for (auto *port : std::as_const(m_inputPorts)) {
            qCDebug(five) << "Setting input at " << 0 << ", " << y;

            // Inputs are pinned to the left edge (x=0) of the 64 px body
            port->setPos(0, y);

            // Non-rotatable elements keep the pixmap fixed and orient their ports in place,
            // applying the current rotation and flip about the element centre.
            if (!rotatesGraphic()) {
                applyPortOrientation(port);
            }

            // Ports are spaced 2*step (16 px) apart so they align with the
            // full grid and remain easily connectable with straight wires.
            y += step * 2;
        }
    }

    if (!m_outputPorts.isEmpty()) {
        int y = 32 - (static_cast<int>(m_outputPorts.size()) * step) + step;

        for (auto *port : std::as_const(m_outputPorts)) {
            qCDebug(five) << "Setting output at " << 64 << ", " << y;

            // Outputs are pinned to the right edge (x=64) of the 64 px body
            port->setPos(64, y);

            // Non-rotatable elements keep the pixmap fixed and orient their ports in place,
            // applying the current rotation and flip about the element centre.
            if (!rotatesGraphic()) {
                applyPortOrientation(port);
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

QVariant GraphicElement::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    // Guard against changes fired during construction before the item is added to a scene
    if (!scene()) {
        return QGraphicsItem::itemChange(change, value);
    }

    if (change == ItemPositionChange) {
        qCDebug(four) << "Align to grid.";
        QPointF newPos = value.toPointF();
        // Snap to half-grid (8 px steps) so elements always align with each other
        // and with the port positions computed in updatePortsProperties().
        const int gridSize = Scene::gridSize / 2;
        const int xV = qRound(newPos.x() / gridSize) * gridSize;
        const int yV = qRound(newPos.y() / gridSize) * gridSize;
        return QPoint(xV, yV);
    }

    // Any geometric change (move, rotate, shear) requires redrawing all connected wires
    if ((change == ItemScenePositionHasChanged) || (change == ItemRotationHasChanged) || (change == ItemTransformHasChanged)) {
        qCDebug(four) << "Moves wires.";
        for (auto *port : allPorts()) {
            port->updateConnections();
        }
    }

    if (change == ItemSelectedHasChanged) {
        m_selected = value.toBool();
        // Propagate selection highlight to all connected wires so users see which
        // signals are attached to the selected element
        highlight(m_selected);
    }

    return QGraphicsItem::itemChange(change, value);
}

bool GraphicElement::sceneEvent(QEvent *event)
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

bool GraphicElement::hasAudio() const
{
    return ElementMetadataRegistry::metadata(m_elementType).hasAudio;
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
    if (!label.isEmpty() && m_label->font() != labelFont()) {
        m_label->setFont(labelFont());
    }

    m_label->setText(label);
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

    m_label->setBrush(theme.m_graphicElementLabelColor);
    m_selectionBrush = theme.m_selectionBrush;
    m_selectionPen = theme.m_selectionPen;

    for (auto *input : std::as_const(m_inputPorts)) {
        input->updateTheme();
    }

    for (auto *output : std::as_const(m_outputPorts)) {
        output->updateTheme();
    }

    update();
}

bool GraphicElement::isValid()
{
    qCDebug(four) << "Checking if the element has the required signals to compute its value.";
    // An element is valid only when every input port is satisfied (connected or optional)
    const bool valid = std::all_of(m_inputPorts.cbegin(), m_inputPorts.cend(),
                                   [](auto *input) { return input->isValid(); });

    if (!valid) {
        // Propagate invalid status downstream so the visual chain shows where validity breaks
        for (auto *output : std::as_const(m_outputPorts)) {
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
    return ElementMetadataRegistry::metadata(m_elementType).hasTrigger;
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
    return ElementMetadataRegistry::metadata(m_elementType).hasFrequency;
}

bool GraphicElement::hasDelay() const
{
    return ElementMetadataRegistry::metadata(m_elementType).hasDelay;
}

bool GraphicElement::hasLabel() const
{
    return ElementMetadataRegistry::metadata(m_elementType).hasLabel;
}

bool GraphicElement::hasTruthTable() const
{
    return ElementMetadataRegistry::metadata(m_elementType).hasTruthTable;
}

bool GraphicElement::hasAudioBox() const
{
    return ElementMetadataRegistry::metadata(m_elementType).hasAudioBox;
}

bool GraphicElement::hasVolume() const
{
    return ElementMetadataRegistry::metadata(m_elementType).hasVolume;
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
}

bool GraphicElement::canChangeAppearance() const
{
    return ElementMetadataRegistry::metadata(m_elementType).canChangeAppearance;
}

bool GraphicElement::rotatesGraphic() const
{
    return ElementMetadataRegistry::metadata(m_elementType).rotatesGraphic;
}

int GraphicElement::inputSize() const
{
    return static_cast<int>(m_inputPorts.size());
}

void GraphicElement::setPortSize(const int size, const bool isInput)
{
    const int minSize = isInput ? minInputSize() : minOutputSize();
    const int maxSize = isInput ? maxInputSize() : maxOutputSize();

    if ((size < minSize) || (size > maxSize)) {
        return;
    }

    if (isInput) {
        if (size > inputSize()) {
            while (size > inputSize()) { addInputPort(); }
        } else {
            qDeleteAll(m_inputPorts.begin() + size, m_inputPorts.end());
            m_inputPorts.resize(size);
        }
    } else {
        if (size > outputSize()) {
            while (size > outputSize()) { addOutputPort(); }
        } else {
            qDeleteAll(m_outputPorts.begin() + size, m_outputPorts.end());
            m_outputPorts.resize(size);
        }
    }

    updatePortsProperties();
}

void GraphicElement::setInputSize(const int size)
{
    setPortSize(size, true);
}

int GraphicElement::outputSize() const
{
    return static_cast<int>(m_outputPorts.size());
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

qsizetype GraphicElement::simOutputSize() const
{
    return m_simOutputValues.size();
}

void GraphicElement::connectPredecessor(const int inputIndex, GraphicElement *source, const int outputPort)
{
    if (inputIndex >= m_simInputConnections.size()) {
        return;
    }
    m_simInputConnections[inputIndex] = {source, outputPort};
}

void GraphicElement::initSimulationVectors(const int inputCount, const int outputCount)
{
    m_simInputConnections.fill({}, inputCount);
    m_simInputValues.fill(Status::Inactive, inputCount);
    m_simOutputValues.resize(outputCount);
    // Initialize outputs from port default statuses when they're explicitly set
    // (e.g., flip-flop Q'=Active), otherwise default to Inactive.
    // Using Inactive (not Unknown) ensures gate-level feedback loops can settle.
    for (int i = 0; i < outputCount; ++i) {
        if (i < m_outputPorts.size()) {
            const Status def = m_outputPorts.at(i)->defaultValue();
            m_simOutputValues[i] = (def == Status::Unknown) ? Status::Inactive : def;
        } else {
            m_simOutputValues[i] = Status::Inactive;
        }
    }
    m_simOutputChanged = false;
}

bool GraphicElement::simUpdateInputsImpl(const bool allowUnknown)
{
    for (int index = 0; index < m_simInputConnections.size(); ++index) {
        auto *pred = m_simInputConnections.at(index).sourceElement;
        Status val;
        if (pred) {
            val = pred->outputValue(m_simInputConnections.at(index).sourceOutputIndex);
        } else {
            if (index < m_inputPorts.size() && m_inputPorts.at(index)->connections().size() > 1) {
                val = Status::Error;  // multi-driver bus conflict
            } else {
                // Unconnected input: use port's default status (replaces global GND/VCC).
                val = (index < m_inputPorts.size()) ? m_inputPorts.at(index)->defaultValue() : Status::Unknown;
            }
        }

        // allowUnknown: only fail for truly unconnected inputs that return Unknown.
        // !allowUnknown: fail for any Unknown or Error value.
        const bool shouldFail = allowUnknown ? (val == Status::Unknown && !pred)
                                             : (val == Status::Unknown || val == Status::Error);
        if (shouldFail) {
            for (auto &out : m_simOutputValues) {
                if (out != Status::Unknown) {
                    m_simOutputChanged = true;
                }
                out = Status::Unknown;
            }
            return false;
        }
        m_simInputValues[index] = val;
    }
    return true;
}

bool GraphicElement::simUpdateInputs()
{
    return simUpdateInputsImpl(false);
}

bool GraphicElement::simUpdateInputsAllowUnknown()
{
    return simUpdateInputsImpl(true);
}

int GraphicElement::decodeSelectValue(int offset, int count) const
{
    int selectValue = 0;
    for (int i = 0; i < count; i++) {
        if (m_simInputValues.at(offset + i) == Status::Active) {
            selectValue |= (1 << i);
        }
    }
    return selectValue;
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
