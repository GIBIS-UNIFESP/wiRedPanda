// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Presentation/PicturePresenters.h"

#include <algorithm>
#include <memory>
#include <utility>

#include <QCoreApplication>
#include <QFile>
#include <QFont>
#include <QHash>
#include <QPainter>
#include <QPen>
#include <QRegularExpression>
#include <QSvgRenderer>
#include <QTextLayout>
#include <QThread>

#include "App/Core/ThemeManager.h"
#include "App/Element/Element.h"
#include "App/Element/ElementAppearance.h"
#include "App/Presentation/ElementPictures.h"
#include "App/Presentation/LabelMetrics.h"
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

/// Returns a wireless node's indicator in \a color, a 20x20 pixmap drawn the first time each colour is asked for: a dot
/// at its lower left and three arcs fanning to the upper right. GUI-thread only.
const QPixmap &wirelessIndicator(const QColor &color)
{
    Q_ASSERT(QCoreApplication::instance()->thread() == QThread::currentThread());

    static QHash<QRgb, QPixmap> cache;
    if (const auto it = cache.constFind(color.rgba()); it != cache.constEnd()) {
        return it.value();
    }

    const auto hex = color.name();
    const auto svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 16 16\">"
        "<circle cx=\"1.5\" cy=\"14.5\" r=\"1.5\" fill=\"%1\"/>"
        "<path d=\"M 1.5,11 A 3.5,3.5 0 0 1 5,14.5\" fill=\"none\" stroke=\"%1\" stroke-width=\"1.5\" stroke-linecap=\"round\"/>"
        "<path d=\"M 1.5,7 A 7.5,7.5 0 0 1 9,14.5\" fill=\"none\" stroke=\"%1\" stroke-width=\"1.5\" stroke-linecap=\"round\"/>"
        "<path d=\"M 1.5,3 A 11.5,11.5 0 0 1 13,14.5\" fill=\"none\" stroke=\"%1\" stroke-width=\"1.5\" stroke-linecap=\"round\"/>"
        "</svg>").arg(hex);

    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pixmap(20, 20);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();
    return cache.insert(color.rgba(), pixmap).value();
}

/// Where a wireless node's indicator goes: in Tx above the output port, its dot at (32, 12), in Rx above the input
/// port, mirrored so the arcs open to the upper left, its dot at (0, 12). Null in mode None.
QRectF wirelessIndicatorRect(const WirelessMode mode)
{
    switch (mode) {
    case WirelessMode::Tx: return {30, -6, 20, 20};
    case WirelessMode::Rx: return {-18, -6, 20, 20};
    case WirelessMode::None: break;
    }
    return {};
}

/// Where an empty Text's hint goes: where its label would be, at the default label anchor.
constexpr QPointF kHintPlace(0, 64);

/// Returns the hint an empty Text shows, in the current language, under Text's translation context.
QString textHint()
{
    QString hint = QCoreApplication::translate("Text", "Double-click to add text");
    hint.replace(u'\n', QChar::LineSeparator);
    return hint;
}

/// Returns the font of an empty Text's hint: the application's, in italics.
QFont textHintFont()
{
    QFont font;
    font.setItalic(true);
    return font;
}

} // namespace PicturePresentersDetail

BodyShape PicturePresenter::measure(const Element &element) const
{
    const QRectF picture = pictureRect(element);
    return {picture, picture.center()};
}

void PicturePresenter::paintBody(QPainter &painter, const Element &element, const BodyContext &context) const
{
    if (const PictureVariant *picture = ElementPictures::instance().shown(element, context.theme.theme())) {
        ElementPictures::draw(painter, *picture);
    }
    paintOverlay(painter, element, context);
}

QRectF PicturePresenter::pictureRect(const Element &element)
{
    // The picture the element names: a theme's sibling is the size of the Light one, so the body is every theme's
    const PictureVariant *picture = ElementPictures::instance().shown(element, Theme::Light);
    return picture ? QRectF(picture->pixmap.rect()) : QRectF();
}

void PicturePresenter::paintOverlay(QPainter &painter, const Element &element, const BodyContext &context) const
{
    Q_UNUSED(painter)
    Q_UNUSED(element)
    Q_UNUSED(context)
}

int LevelPresenter::pictureSlot(const Element &element) const
{
    return element.param<bool>(ParamId::On) ? 1 : 0;
}

int AudioPresenter::pictureSlot(const Element &element) const
{
    return element.param<bool>(ParamId::Playing) ? 1 : 0;
}

LabelPlacement AudioPresenter::label(const Element &element) const
{
    Q_UNUSED(element)
    // To the right of the 64x64 body, about its middle
    return {QPointF(64, 34)};
}

int LedPresenter::pictureSlot(const Element &element) const
{
    return element.param<int>(ParamId::Lit);
}

void RotaryPresenter::paintOverlay(QPainter &painter, const Element &element, const BodyContext &context) const
{
    Q_UNUSED(context)
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

void SegmentDisplayPresenter::paintOverlay(QPainter &painter, const Element &element, const BodyContext &context) const
{
    Q_UNUSED(context)
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

BodyShape NodePresenter::measure(const Element &element) const
{
    const QRectF picture = pictureRect(element);
    const auto mode = static_cast<WirelessMode>(element.param<int>(ParamId::Wireless));
    return {picture, picture.center(), PicturePresentersDetail::wirelessIndicatorRect(mode)};
}

LabelPlacement NodePresenter::label(const Element &element) const
{
    Q_UNUSED(element)
    // Below the 32x32 body
    return {QPointF(0, 32)};
}

void NodePresenter::paintOverlay(QPainter &painter, const Element &element, const BodyContext &context) const
{
    const auto mode = static_cast<WirelessMode>(element.param<int>(ParamId::Wireless));
    if (mode == WirelessMode::None) {
        return;
    }

    QColor color;
    switch (static_cast<Status>(element.param<int>(ParamId::Carried))) {
    case Status::Unknown:  color = context.theme.m_connectionUnknown;  break;
    case Status::Inactive: color = context.theme.m_connectionInactive; break;
    case Status::Active:   color = context.theme.m_connectionActive;   break;
    case Status::Error:    color = context.theme.m_connectionError;    break;
    }

    painter.save();
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    if (mode == WirelessMode::Tx) {
        painter.translate(30, -6);
    } else {
        // Mirrored about the pixmap's left edge, which lands at x = 2
        painter.translate(2, -6);
        painter.scale(-1, 1);
    }
    painter.drawPixmap(QPointF(0, 0), PicturePresentersDetail::wirelessIndicator(color));
    painter.restore();
}

BodyShape TextPresenter::measure(const Element &element) const
{
    using namespace PicturePresentersDetail;
    const QRectF picture = pictureRect(element);
    QTextLayout layout(textHint(), textHintFont());
    const QRectF hint = LabelMetrics::layOut(layout).translated(kHintPlace);
    return {picture.united(element.labelBoundingRect()).united(hint), picture.center()};
}

void TextPresenter::paintOverlay(QPainter &painter, const Element &element, const BodyContext &context) const
{
    using namespace PicturePresentersDetail;
    if (!element.label().isEmpty()) {
        return;
    }

    // A faded variant of the label's colour: distinguishable from real content at a glance, but not so loud it reads
    // as a label
    QColor color = context.theme.m_elementLabelColor;
    color.setAlpha(140);

    const QFont font = textHintFont();
    QTextLayout layout(textHint(), font);
    LabelMetrics::layOut(layout);

    painter.save();
    painter.setFont(font);
    QPen pen;
    pen.setBrush(color);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    layout.draw(&painter, kHintPlace);
    painter.restore();
}
