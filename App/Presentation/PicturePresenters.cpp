// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Presentation/PicturePresenters.h"

#include <algorithm>
#include <memory>
#include <utility>

#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QPainter>
#include <QRegularExpression>
#include <QSvgRenderer>
#include <QThread>

#include "App/Element/Element.h"
#include "App/Element/ElementAppearance.h"
#include "App/Presentation/ElementPictures.h"
#include "App/Wiring/Port.h"

namespace PicturePresentersDetail {

/// Substitutes every solid fill colour in \a svgBytes with \a color, leaving each shape's own fill-opacity untouched:
/// the segment SVGs are solid-filled single shapes, so this gives the White/Red/Green/Blue/Purple segments.
QByteArray recoloredSegmentSvg(const QByteArray &svgBytes, const QColor &color)
{
    static const QRegularExpression fillHex(QStringLiteral("fill:#[0-9a-fA-F]{6}"));
    QString svg = QString::fromUtf8(svgBytes);
    svg.replace(fillHex, QStringLiteral("fill:%1").arg(color.name()));
    return svg.toUtf8();
}

/// Returns the White/Red/Green/Blue/Purple vector renderers of the segment SVG at \a resourcePath (index order matches
/// Element::colorNameToIndex()), built the first time, so lit segments stay crisp at any zoom. GUI-thread only.
const QVector<std::shared_ptr<QSvgRenderer>> &segmentRenderers(const QString &resourcePath)
{
    Q_ASSERT(QCoreApplication::instance()->thread() == QThread::currentThread());

    static QHash<QString, QVector<std::shared_ptr<QSvgRenderer>>> cache;

    if (const auto it = cache.constFind(resourcePath); it != cache.constEnd()) {
        return it.value();
    }

    QFile file(resourcePath);
    const QByteArray svgBytes = file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();

    static const QColor colors[] = {
        QColor(255, 255, 255), QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255), QColor(255, 0, 255)
    };

    QVector<std::shared_ptr<QSvgRenderer>> renderers;
    renderers.reserve(5);
    for (const QColor &color : colors) {
        renderers.append(std::make_shared<QSvgRenderer>(recoloredSegmentSvg(svgBytes, color)));
    }

    return cache.insert(resourcePath, renderers).value();
}

/// Returns the vector renderer of the rotary's arrow, built the first time. The arrow is a built-in picture, never a
/// custom appearance as the dial can be, so one renderer serves every rotary. GUI-thread only.
QSvgRenderer &rotaryArrowRenderer()
{
    static QSvgRenderer renderer(QStringLiteral(":/Components/Input/rotary_arrow.svg"));
    return renderer;
}

} // namespace PicturePresentersDetail

BodyShape PicturePresenter::measure(const Element &element) const
{
    const QRectF picture = pictureRect(element);
    return {picture, picture.center()};
}

void PicturePresenter::paintBody(QPainter &painter, const Element &element, const BodyContext &context) const
{
    Q_UNUSED(context)
    if (const PictureVariant *picture = ElementPictures::instance().shown(element)) {
        ElementPictures::draw(painter, *picture);
    }
    paintOverlay(painter, element);
}

QRectF PicturePresenter::pictureRect(const Element &element)
{
    const PictureVariant *picture = ElementPictures::instance().shown(element);
    return picture ? QRectF(picture->pixmap.rect()) : QRectF();
}

void PicturePresenter::paintOverlay(QPainter &painter, const Element &element) const
{
    Q_UNUSED(painter)
    Q_UNUSED(element)
}

int LevelPresenter::pictureSlot(const Element &element) const
{
    return element.param<bool>(ParamId::On) ? 1 : 0;
}

int AudioPresenter::pictureSlot(const Element &element) const
{
    return element.param<bool>(ParamId::Playing) ? 1 : 0;
}

int LedPresenter::pictureSlot(const Element &element) const
{
    return element.param<int>(ParamId::Lit);
}

void RotaryPresenter::paintOverlay(QPainter &painter, const Element &element) const
{
    // A small mark for every output port, evenly spaced around the dial; the selected one also gets the arrow over it
    const int positions = element.outputSize();
    const int selected = element.param<int>(ParamId::Position);
    for (int port = 0; port < positions; ++port) {
        painter.save();

        // Port 0 is at the top (12 o'clock), increasing clockwise
        const double angle = 360. / positions * port;
        const QPointF center = element.pixmapCenter();
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

        if (selected != port) {
            continue;
        }

        // Overlay the arrow at the same rotation as the active mark
        painter.save();

        painter.translate(center.x(), center.y());
        painter.rotate(angle);
        painter.translate(-center.x(), -center.y());
        PicturePresentersDetail::rotaryArrowRenderer().render(&painter, QRectF(0, 0, 64, 64));

        painter.restore();
    }
}

SegmentDisplayPresenter::SegmentDisplayPresenter(QVector<int> segments)
    : m_segments(std::move(segments))
{
}

BodyShape SegmentDisplayPresenter::measure(const Element &element) const
{
    const QRectF picture = pictureRect(element);
    return {picture.united(QRectF(0, 0, 64, 64)), picture.center()};
}

void SegmentDisplayPresenter::paintOverlay(QPainter &painter, const Element &element) const
{
    // Every segment's picture shares the unlit picture's 64x64 box, so the lit ones composite over it, as vectors,
    // crisp at any zoom. A segment is the built-in one whatever picture the display shows.
    Q_ASSERT_X(m_segments.size() == element.inputSize(), "SegmentDisplayPresenter::paintOverlay()",
               "a display's inputs are fixed, one per segment of its table");
    const QRectF body(0, 0, 64, 64);
    const int color = Element::colorNameToIndex(element.color());
    const QStringList &segments = element.appearance().defaultAppearances();
    const int inputs = std::min(static_cast<int>(m_segments.size()), element.inputSize());
    for (int input = 0; input < inputs; ++input) {
        if (element.inputPort(input)->status() == Status::Active) {
            PicturePresentersDetail::segmentRenderers(segments.at(m_segments.at(input))).at(color)->render(&painter, body);
        }
    }
}

BodyShape TextPresenter::measure(const Element &element) const
{
    const QRectF picture = pictureRect(element);
    return {picture.united(element.childrenBoundingRect()), picture.center()};
}
