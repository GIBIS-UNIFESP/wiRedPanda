// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Presentation/ElementPictures.h"

#include <cmath>
#include <vector>

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QPainter>
#include <QRegularExpression>
#include <QSvgRenderer>

#include "App/Core/ThemeManager.h"
#include "App/Element/Element.h"
#include "App/Element/ElementTypeDescriptor.h"
#include "App/Presentation/ElementPresenter.h"

namespace ElementPicturesDetail {

/// An opening tag of a picture's label: a `<text>`, or any element marked `class="label"`, which is how a bundled picture
/// holds its text as outlines, needing no font. Captures the tag's name.
const QRegularExpression &labelTag()
{
    static const QRegularExpression tag(QStringLiteral("<(text|[A-Za-z]+(?=[^>]*\\bclass\\s*=\\s*\"label\"))\\b[^>]*>"));
    return tag;
}

/// Whether \a svgBytes holds a label to keep upright.
bool hasLabels(const QByteArray &svgBytes)
{
    return labelTag().match(QString::fromUtf8(svgBytes)).hasMatch();
}

/// Rewrites \a svgBytes so every label (see labelTag()) is counter-oriented about its own centre for the
/// element's current rotation \a angle and \a flipX / \a flipY. Pre-applying the inverse of the
/// item transform (Rotate(-angle) after the flip) means that after the element's own item-level
/// rotation + flip the glyphs end up upright and unmirrored, while still travelling to the
/// rotated/mirrored side. Transforming about each label's *own* centre composes cleanly with any
/// parent transform, so it is correct regardless of how the label is nested. Returns the input
/// unchanged on any parse failure.
QByteArray orientSvgLabels(const QByteArray &svgBytes, const qreal angle, const bool flipX, const bool flipY)
{
    static const QRegularExpression idAttr(QStringLiteral("\\bid\\s*=\\s*\"([^\"]*)\""));
    static const QRegularExpression transformAttr(QStringLiteral("\\btransform\\s*="));

    QString svg = QString::fromUtf8(svgBytes);

    // Pass 1: ensure every label's opening tag carries an id so boundsOnElement() can address it.
    // Inkscape already ids these, but be defensive for hand-written assets.
    {
        QString out;
        qsizetype last = 0;
        int generated = 0;
        auto matches = labelTag().globalMatch(svg);
        while (matches.hasNext()) {
            const auto match = matches.next();
            out += svg.mid(last, match.capturedStart() - last);
            QString tag = match.captured(0);
            if (!idAttr.match(tag).hasMatch()) {
                // Insert after the tag's name so the new attribute precedes the existing ones.
                tag.insert(match.capturedLength(1) + 1, QStringLiteral(" id=\"wpflip-text-%1\"").arg(generated++));
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

    // Pass 2: inject a counter-orientation transform on each label, pivoting about that label's
    // own centre. The item applies Flip ∘ Rotate(angle) to the pixmap content, so the inverse the
    // glyph must carry is Rotate(-angle) ∘ Flip (rotate outer, scale inner). The rotate term is
    // omitted at angle 0 and the scale term when unflipped, so the flip-only output is unchanged.
    QString out;
    qsizetype last = 0;
    auto matches = labelTag().globalMatch(svg);
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
                tag.insert(match.capturedLength(1) + 1, transform);
            }
        }

        out += tag;
        last = match.capturedEnd();
    }
    out += svg.mid(last);

    return out.toUtf8();
}

/// The orientation a picture's SVG labels are counter-oriented for: the turn and the flips, all zero for a picture
/// shown as authored.
struct Orientation {
    qreal angle = 0;
    bool flipX = false;
    bool flipY = false;

    friend bool operator==(const Orientation &, const Orientation &) = default;
};

size_t qHash(const Orientation &orientation, const size_t seed = 0)
{
    return qHashMulti(seed, orientation.angle, orientation.flipX, orientation.flipY);
}

/// One version of one picture file, and the orientations it has been shown in.
struct Picture {
    QString path;      ///< The file it was loaded from.
    bool svg = false;
    QPixmap base;      ///< The picture as QPixmap::load() reads it.
    QByteArray source; ///< An SVG's source, which its counter-oriented variants are built from.
    QHash<Orientation, PictureVariant> variants;
    bool darkLooked = false;       ///< Whether its Dark theme sibling (ElementPictures::themed()) was looked for.
    std::optional<PictureId> dark; ///< Its Dark theme sibling, when it has one that loads.
};

/// The pictures loaded so far, indexed by their PictureId, and the id of each file version.
struct Library {
    std::vector<Picture> pictures;
    QHash<QString, PictureId> ids;
};

Library &library()
{
    static Library library;
    return library;
}

/// Renders the SVG \a source with its labels counter-oriented for \a orientation. Returns a null pixmap when there is
/// no label to correct or the SVG does not parse, so the caller keeps the picture as loaded (which the item transform
/// then rotates/flips).
QPixmap orientedSvgPixmap(const QByteArray &source, const Orientation &orientation)
{
    if (!hasLabels(source)) {
        return {};
    }
    const QByteArray modified = orientSvgLabels(source, orientation.angle, orientation.flipX, orientation.flipY);

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
    return pixmap;
}

/// Returns the vector renderer of the SVG \a source, its labels counter-oriented for \a orientation when it has any;
/// null if the SVG does not parse, so the caller falls back to the raster.
std::shared_ptr<QSvgRenderer> svgRenderer(const QByteArray &source, const Orientation &orientation)
{
    const bool oriented = (orientation.angle != 0.0) || orientation.flipX || orientation.flipY;
    const QByteArray svg = (oriented && hasLabels(source))
        ? orientSvgLabels(source, orientation.angle, orientation.flipX, orientation.flipY)
        : source;

    auto renderer = std::make_shared<QSvgRenderer>(svg);
    if (!renderer->isValid()) {
        renderer.reset();
    }
    return renderer;
}

/// Returns the orientation \a element shows its picture in. Only rotatable elements transform their graphic, so only
/// they need the baked labels counter-oriented; a non-rotatable element keeps its icon upright (it moves only its
/// ports), so its text renders as authored. Every unoriented element (the vast majority) shares one variant.
Orientation orientationOf(const Element &element)
{
    if (!element.rotatesGraphic() || !(element.isFlippedX() || element.isFlippedY() || (element.rotation() != 0.0))) {
        return {};
    }
    // Canonicalized to [0,360) so equivalent rotations (e.g. -90 and 270, which render identically) share one
    // variant -- MCP can request any angle and rotate-left produces negatives.
    return {std::fmod(std::fmod(element.rotation(), 360.0) + 360.0, 360.0), element.isFlippedX(), element.isFlippedY()};
}

/// Returns the picture \a id names as \a theme shows it: its Dark sibling in the Dark theme, when it has one.
PictureId themedId(const PictureId id, const Theme theme)
{
    if (theme != Theme::Dark) {
        return id;
    }
    // Loading the sibling can grow the library, so the picture is looked up again after it
    if (!library().pictures.at(static_cast<size_t>(id)).darkLooked) {
        const QString path = library().pictures.at(static_cast<size_t>(id)).path;
        const QString dark = ElementPictures::themed(path, Theme::Dark);
        const std::optional<PictureId> sibling = (dark != path) ? ElementPictures::instance().load(dark) : std::nullopt;
        Picture &picture = library().pictures.at(static_cast<size_t>(id));
        picture.darkLooked = true;
        picture.dark = sibling;
    }
    return library().pictures.at(static_cast<size_t>(id)).dark.value_or(id);
}

/// Returns \a picture shown in \a orientation, building it the first time.
const PictureVariant &variantOf(Picture &picture, const Orientation &orientation)
{
    if (const auto it = picture.variants.constFind(orientation); it != picture.variants.constEnd()) {
        return it.value();
    }

    PictureVariant variant;
    variant.pixmap = picture.base;
    if (picture.svg) {
        if (orientation != Orientation{}) {
            // A rotated/flipped SVG variant can differ in size from the picture as loaded; the body is measured from
            // the variant shown.
            if (QPixmap oriented = orientedSvgPixmap(picture.source, orientation); !oriented.isNull()) {
                variant.pixmap = oriented;
                variant.oriented = true;
            }
        }
        variant.renderer = svgRenderer(picture.source, orientation);
    }
    return picture.variants.insert(orientation, variant).value();
}

} // namespace ElementPicturesDetail

const ElementPictures &ElementPictures::instance()
{
    static const ElementPictures pictures;
    return pictures;
}

std::optional<PictureId> ElementPictures::load(const QString &path) const
{
    using namespace ElementPicturesDetail;

    // A version is the file's path, modification time and size, so an element that loads a file after it changed on
    // disk shows the new one, while one that loaded it before keeps the picture it showed. A resource is built into
    // the program and never changes: its path is its version.
    QString version = path;
    if (!path.startsWith(QLatin1String(":/"))) {
        const QFileInfo info(path);
        version += QLatin1Char('|') + QString::number(info.lastModified().toMSecsSinceEpoch()) + QLatin1Char('|')
            + QString::number(info.size());
    }

    Library &pictures = library();
    if (const auto it = pictures.ids.constFind(version); it != pictures.ids.constEnd()) {
        return it.value();
    }

    Picture picture;
    picture.path = path;
    if (!picture.base.load(path)) {
        return std::nullopt;
    }
    picture.svg = path.endsWith(QLatin1String(".svg"), Qt::CaseInsensitive);
    if (picture.svg) {
        if (QFile file(path); file.open(QIODevice::ReadOnly)) {
            picture.source = file.readAll();
        }
    }

    if (pictures.pictures.empty()) {
        // The pictures hold pixmaps, which must go before the application does.
        qAddPostRoutine([] {
            library().pictures.clear();
            library().ids.clear();
        });
    }

    const auto id = static_cast<PictureId>(pictures.pictures.size());
    pictures.pictures.push_back(std::move(picture));
    pictures.ids.insert(version, id);
    return id;
}

const PictureVariant *ElementPictures::shown(const Element &element, const Theme theme) const
{
    using namespace ElementPicturesDetail;

    const int slot = ElementPresenter::of(element.elementType()).pictureSlot(element);
    const std::optional<PictureId> id = element.appearance().picture(slot);
    if (!id) {
        return nullptr;
    }
    return &variantOf(library().pictures.at(static_cast<size_t>(themedId(*id, theme))), orientationOf(element));
}

void ElementPictures::draw(QPainter &painter, const PictureVariant &variant)
{
    // Draw the body from vector data (crisp at any zoom) when the appearance is an SVG; fall back to
    // the rasterised pixmap for raster (PNG/JPG) appearances. DeviceCoordinateCache re-renders this
    // per zoom level, so the SVG stays sharp instead of scaling a fixed-size bitmap.
    if (variant.renderer && variant.renderer->isValid()) {
        painter.save();
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                               QPainter::SmoothPixmapTransform, true);
        // Render into the SVG's native box at the origin — the same 0,0..defaultSize the raster occupied.
        variant.renderer->render(&painter, QRectF(QPointF(0, 0), QSizeF(variant.renderer->defaultSize())));
        painter.restore();
    } else {
        // Pixmap origin is always (0,0) in item coordinates; the rotation pivot (the body's centre) is set
        // separately, as the picture changes (Element::pictureReplaced()).
        painter.drawPixmap(QPoint(0, 0), variant.pixmap);
    }
}

QPixmap ElementPictures::preview(const QString &path, const QSize &size)
{
    QPixmap pixmap;
    pixmap.load(path);

    return pixmap.isNull() ? pixmap : pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QString ElementPictures::themed(const QString &path, const Theme theme)
{
    static const QString light = QStringLiteral("/Light/");
    if ((theme != Theme::Dark) || !path.startsWith(QLatin1String(":/")) || !path.contains(light)) {
        return path;
    }

    QString dark = path;
    dark.replace(dark.lastIndexOf(light), light.size(), QStringLiteral("/Dark/"));
    return QFile::exists(dark) ? dark : path;
}

QPixmap ElementPictures::typePicture(const ElementType type)
{
    if (type == ElementType::Unknown) {
        return {};
    }

    return QPixmap{themed(ElementTypeDescriptor::of(type).pixmapPath, ThemeManager::attributes().theme())};
}
