// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementAppearance.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QPen>
#include <QRegularExpression>
#include <QSvgRenderer>

#include "App/Core/Common.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"

namespace {

/// Cache decoded pixmaps by resolved path so each image is loaded from disk only once.
QHash<QString, QPixmap> &pixmapCache()
{
    static QHash<QString, QPixmap> cache;
    return cache;
}

/// Cache of orientation-variant pixmaps keyed by "<resolvedPath>|<canonicalAngle>|<flipX><flipY>"
/// so the SVG text correction is rendered only once per appearance and rotation/flip state.
/// Both this and pixmapCache() are plain static QHashes with no locking: GraphicElement pixmaps are
/// only built and read on the GUI thread, so concurrent access does not occur.
QHash<QString, QPixmap> &orientedPixmapCache()
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
QByteArray orientSvgTextNodes(const QByteArray &svgBytes, const qreal angle, const bool flipX, const bool flipY)
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
QPixmap orientedSvgPixmap(const QString &resolvedPath, const qreal angle, const bool flipX, const bool flipY)
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

} // namespace

ElementAppearance::ElementAppearance(GraphicElement *owner)
    : m_owner(owner)
{
}

ElementAppearance::~ElementAppearance() = default;

void ElementAppearance::seedFromMetadata(const QStringList &defaultAppearances,
                                         const QStringList &alternativeAppearances,
                                         const QString &pixmapPath)
{
    m_defaultAppearances = defaultAppearances;
    m_pixmapPath = pixmapPath;

    // For elements whose pixmapPath is theme-dependent (e.g. memory elements),
    // defaultAppearances may be left empty in the metadata to avoid evaluating the
    // theme path during static initialisation (before QApplication exists).
    // Populate it lazily here from the now-correct pixmapPath.
    if (m_defaultAppearances.isEmpty() && !m_pixmapPath.isEmpty()) {
        m_defaultAppearances = QStringList({m_pixmapPath});
    }

    m_alternativeAppearances = alternativeAppearances.isEmpty() ? m_defaultAppearances : alternativeAppearances;
}

QPointF ElementAppearance::pixmapCenter() const
{
    return QRectF(m_pixmap.rect()).center();
}

void ElementAppearance::setPixmap(const int index)
{
    setPixmap(m_usingDefaultAppearance ? m_defaultAppearances.at(index) : m_alternativeAppearances.at(index));
}

void ElementAppearance::setPixmap(const QString &pixmapPath)
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
        const QString contextDir = Scene::resolveContextDir(m_owner);
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
    applyOrientation();

    // The transform origin must be updated whenever the pixmap changes so that
    // rotation and scale operations remain centred on the new image
    m_owner->setTransformOriginPoint(pixmapCenter());
    m_owner->update();

    m_currentPixmapPath = pixmapPath;
}

void ElementAppearance::applyOrientation()
{
    // Only rotatable elements transform their graphic, so only they need the baked <text> labels
    // counter-oriented. A non-rotatable element keeps its icon upright (it moves only its ports),
    // so its text — if any — must render as authored and never be counter-oriented.
    const bool oriented = m_owner->rotatesGraphic()
        && (m_owner->isFlippedX() || m_owner->isFlippedY() || (m_owner->rotation() != 0.0));
    if (oriented && m_resolvedPixmapPath.endsWith(QLatin1String(".svg"), Qt::CaseInsensitive)) {
        const QPixmap variant = orientedSvgPixmap(m_resolvedPixmapPath, m_owner->rotation(), m_owner->isFlippedX(), m_owner->isFlippedY());
        m_pixmap = variant.isNull() ? m_basePixmap : variant;
    } else {
        m_pixmap = m_basePixmap;
    }
    rebuildSvgRenderer();
    m_owner->update();
}

void ElementAppearance::rebuildSvgRenderer()
{
    // render() draws this vector renderer so SVG elements stay crisp at any zoom; raster (PNG/JPG)
    // appearances leave it null and fall back to drawing the rasterised m_pixmap.
    if (!m_resolvedPixmapPath.endsWith(QLatin1String(".svg"), Qt::CaseInsensitive)) {
        m_svgRenderer.reset();
        return;
    }

    QFile file(m_resolvedPixmapPath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_svgRenderer.reset();
        return;
    }
    QByteArray svg = file.readAll();

    // Counter-orient each <text> while rotated/flipped so pin labels stay upright — the same
    // correction orientedSvgPixmap() bakes into the rasterised variant. Gated on rotatesGraphic()
    // because a non-rotatable element never transforms its graphic, so its text stays as authored.
    const bool oriented = m_owner->rotatesGraphic()
        && (m_owner->isFlippedX() || m_owner->isFlippedY() || (m_owner->rotation() != 0.0));
    if (oriented && svg.contains("<text")) {
        svg = orientSvgTextNodes(svg, m_owner->rotation(), m_owner->isFlippedX(), m_owner->isFlippedY());
    }

    auto renderer = std::make_unique<QSvgRenderer>(svg);
    m_svgRenderer = renderer->isValid() ? std::move(renderer) : nullptr;
}

void ElementAppearance::setAppearance(const bool defaultAppearance, const QString &fileName)
{
    if (defaultAppearance) {
        m_alternativeAppearances = m_defaultAppearances;
    } else {
        m_alternativeAppearances[0] = fileName;
    }

    m_usingDefaultAppearance = defaultAppearance;
    setPixmap(0);
}

void ElementAppearance::setAppearanceAt(const int index, const QString &fileName)
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

void ElementAppearance::setAlternativeAppearanceAt(const int index, const QString &path)
{
    if (index < 0 || index >= m_alternativeAppearances.size()) {
        return;
    }
    m_alternativeAppearances[index] = path;
}

void ElementAppearance::recomputeUsingDefault()
{
    m_usingDefaultAppearance = std::equal(
        m_defaultAppearances.begin(), m_defaultAppearances.end(),
        m_alternativeAppearances.begin(), m_alternativeAppearances.end()
        );
}

QStringList ElementAppearance::externalFiles() const
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

void ElementAppearance::render(QPainter *painter, const QRectF &boundingRect, const bool selected) const
{
    if (selected) {
        painter->save();
        painter->setBrush(m_selectionBrush);
        // 0.5 pen width keeps the selection outline thin regardless of zoom level
        painter->setPen(QPen(m_selectionPen, 0.5, Qt::SolidLine));
        // Corner radius of 5 matches the visual rounding used on element bodies
        painter->drawRoundedRect(boundingRect, 5, 5);
        painter->restore();
    }

    // Draw the body from vector data (crisp at any zoom) when the appearance is an SVG; fall back to
    // the rasterised pixmap for raster (PNG/JPG) appearances. DeviceCoordinateCache re-renders this
    // per zoom level, so the SVG stays sharp instead of scaling a fixed-size bitmap.
    if (m_svgRenderer && m_svgRenderer->isValid()) {
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                                QPainter::SmoothPixmapTransform, true);
        // Render into the SVG's native box at the origin — the same 0,0..defaultSize the raster occupied.
        m_svgRenderer->render(painter, QRectF(QPointF(0, 0), QSizeF(m_svgRenderer->defaultSize())));
        painter->restore();
    } else {
        // Pixmap origin is always (0,0) in item coordinates; the transform origin
        // (centre of the pixmap) is set separately via setTransformOriginPoint().
        painter->drawPixmap(QPoint(0, 0), m_pixmap);
    }
}

void ElementAppearance::applyTheme(const ThemeAttributes &theme)
{
    m_selectionBrush = theme.m_selectionBrush;
    m_selectionPen = theme.m_selectionPen;
}
