// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/MinimapWidget.h"

#include <QCursor>
#include <QEvent>
#include <QMouseEvent>
#include <QPaintDevice>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>

#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"

MinimapWidget::MinimapWidget(Scene *scene, GraphicsView *view, QWidget *parent)
    : QWidget(parent)
    , m_scene(scene)
    , m_view(view)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground, false);
    setFocusPolicy(Qt::NoFocus);
    // Resizable via drag handles on the edges/corners (see resizeModeAt()); the minimum
    // keeps the viewport-outline overlay and mascot content legible.
    setMinimumSize(160, 120);
    resize(220, 160);
    setMouseTracking(true); // hover cursor feedback over resize/move handles without a button held
    setToolTip(tr("Mini-map: click or drag to navigate"));
    setAccessibleName(tr("Circuit minimap"));
    setWhatsThis(tr("A miniature overview of the whole circuit. Click or drag inside it to "
                     "jump the main canvas to that location."));

    if (m_scene) {
        connect(m_scene, &Scene::circuitHasChanged, this, &MinimapWidget::invalidateCache);
    }

    if (m_view) {
        connect(m_view, &GraphicsView::zoomChanged, this, &MinimapWidget::updateOverlay);
        if (m_view->horizontalScrollBar())
            connect(m_view->horizontalScrollBar(), &QScrollBar::valueChanged, this, &MinimapWidget::updateOverlay);
        if (m_view->verticalScrollBar())
            connect(m_view->verticalScrollBar(), &QScrollBar::valueChanged, this, &MinimapWidget::updateOverlay);
        if (m_view->viewport())
            m_view->viewport()->installEventFilter(this);
    }

    m_throttle.setInterval(200);
    m_throttle.setSingleShot(true);
    connect(&m_throttle, &QTimer::timeout, this, &MinimapWidget::onThrottleTimeout);
}

QSize MinimapWidget::sizeHint() const
{
    return QSize(200, 150);
}

void MinimapWidget::paintEvent(QPaintEvent * /*event*/)
{
    if (!m_scene || !m_view) {
        return;
    }
    QPainter painter(this);
    painter.fillRect(rect(), palette().window().color());
    painter.setRenderHint(QPainter::Antialiasing, false);

    QRectF srcUsed;
    double scale = 1.0, dx = 0.0, dy = 0.0, usedW = 0.0, usedH = 0.0;
    if (!computeTransform(srcUsed, scale, dx, dy, usedW, usedH))
        return;

    const qreal dpr = this->devicePixelRatioF();
    const QSize pixSize = QSize(qRound(usedW * dpr), qRound(usedH * dpr));
    // Regenerate when the source region changes too: unlike the old item-box source, the source
    // now tracks sceneRect / the visible viewport, which change on zoom -- a stale cache would
    // otherwise show the wrong region.
    if (m_cacheDirty || m_cache.size() != pixSize || m_cacheSrc != srcUsed) {
        m_cache = QPixmap(pixSize);
        m_cache.setDevicePixelRatio(dpr);
        m_cache.fill(palette().window().color());
        QPainter pixmapPainter(&m_cache);
        pixmapPainter.setRenderHint(QPainter::Antialiasing, false);
        m_scene->render(&pixmapPainter, QRectF(0, 0, usedW, usedH), srcUsed, Qt::KeepAspectRatio);
        m_cacheDirty = false;
        m_cacheSrc = srcUsed;
    }

    // Draw the cached pixmap at the computed offset so the minimap content
    // aligns with the widget's mapping and avoids letterbox misalignment.
    painter.drawPixmap(QRectF(dx, dy, usedW, usedH), m_cache, QRectF(0, 0, usedW, usedH));

    // Draw viewport rectangle in minimap coordinates
    const QRectF visible = m_view->mapToScene(m_view->viewport()->rect()).boundingRect();

    QRectF vr(
        dx + (visible.left() - srcUsed.left()) * scale,
        dy + (visible.top() - srcUsed.top()) * scale,
        visible.width() * scale,
        visible.height() * scale);

    QPen pen(palette().highlight().color());
    pen.setWidthF(1.6);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(vr);

    painter.setRenderHint(QPainter::Antialiasing, true);
    drawMoveHandle(painter);
    drawResizeGrips(painter);
}

void MinimapWidget::mousePressEvent(QMouseEvent *event)
{
    if (!m_scene || !m_view) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        const ResizeMode mode = resizeModeAt(event->pos());
        if (mode != ResizeMode::None) {
            m_resizing = true;
            m_resizeMode = mode;
            m_lastGlobalPos = event->globalPosition().toPoint();
            grabMouse();
            event->accept();
            return;
        }

        if (isMoveHandle(event->pos())) {
            m_moving = true;
            m_lastGlobalPos = event->globalPosition().toPoint();
            grabMouse();
            event->accept();
            return;
        }
    }

    m_view->centerOn(widgetToScene(event->pos()));
    updateOverlay();
    m_dragging = true;
    event->accept();
}

void MinimapWidget::wheelEvent(QWheelEvent *event)
{
    // Swallow wheel events to prevent zooming the main view when the cursor
    // is over the minimap. Do not call base implementation.
    event->accept();
}

void MinimapWidget::invalidateCache()
{
    // Start or restart the throttle timer. The actual cache invalidation
    // and repaint will happen when the timer fires (max ~5 fps).
    m_throttle.start();
}

void MinimapWidget::onThrottleTimeout()
{
    m_cacheDirty = true;
    update();
}

void MinimapWidget::updateOverlay()
{
    update();
}

bool MinimapWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == (m_view ? m_view->viewport() : nullptr) && event->type() == QEvent::Resize) {
        updateOverlay();
        return QWidget::eventFilter(watched, event);
    }
    return QWidget::eventFilter(watched, event);
}

bool MinimapWidget::computeTransform(QRectF &srcOut, double &scaleOut, double &dxOut, double &dyOut, double &usedWOut, double &usedHOut) const
{
    if (!m_scene)
        return false;

    // The minimap maps the whole navigable canvas, not the tight item box: fitting
    // itemsBoundingRect() alone would zoom a single small element to fill the widget.
    //   - sceneRect() is the navigable area once the user has interacted (stable under scroll).
    //   - itemsBoundingRect() is a defensive superset so an item momentarily outside the scene
    //     rect stays visible.
    //   - the visible viewport region is the decisive term: right after a file loads, sceneRect()
    //     is set to the tight item box (see WorkSpace load), so without this the minimap would
    //     stay zoomed. Including it guarantees the minimap is never more zoomed-in than the view.
    // Scene always contains a permanent (near-zero) rubber-band selection-rect item, so
    // itemsBoundingRect() never reports invalid/empty here in practice -- the final early return
    // exists only as a defensive guard for a Scene that isn't fully constructed the usual way.
    QRectF src = m_scene->sceneRect().united(m_scene->itemsBoundingRect());
    if (m_view && m_view->viewport())
        src = src.united(m_view->mapToScene(m_view->viewport()->rect()).boundingRect());
    if (!src.isValid() || src.isEmpty())
        return false;

    const double w = static_cast<double>(width());
    const double h = static_cast<double>(height());

    // Expand the source to the widget's aspect ratio (growing the deficient axis symmetrically
    // about its centre) so the scene fills the whole widget instead of letterboxing with bars.
    // This stays aspect-correct -- it just shows a little extra empty canvas on the shorter axis,
    // and unlike cropping it never clips the overview. src is valid/non-empty (guarded above), so
    // its width/height are > 0.
    const double widgetAspect = w / h;
    const double srcAspect = src.width() / src.height();
    if (srcAspect < widgetAspect) {
        const double grow = (src.height() * widgetAspect - src.width()) / 2.0;
        src.adjust(-grow, 0.0, grow, 0.0);
    } else if (srcAspect > widgetAspect) {
        const double grow = (src.width() / widgetAspect - src.height()) / 2.0;
        src.adjust(0.0, -grow, 0.0, grow);
    }

    const double sx = src.width() > 0 ? w / src.width() : 1.0;
    const double sy = src.height() > 0 ? h / src.height() : 1.0;
    const double scale = qMin(sx, sy);
    const double usedW = src.width() * scale;
    const double usedH = src.height() * scale;
    const double dx = (w - usedW) * 0.5;
    const double dy = (h - usedH) * 0.5;

    srcOut = src;
    scaleOut = scale;
    dxOut = dx;
    dyOut = dy;
    usedWOut = usedW;
    usedHOut = usedH;
    return true;
}

QPointF MinimapWidget::widgetToScene(const QPointF &p) const
{
    QRectF src;
    double scale = 1.0, dx = 0.0, dy = 0.0, usedW = 0.0, usedH = 0.0;
    if (!computeTransform(src, scale, dx, dy, usedW, usedH))
        return QPointF();

    double x = (p.x() - dx) / scale + src.left();
    double y = (p.y() - dy) / scale + src.top();

    // Clamp to source rect
    x = qBound(src.left(), x, src.right());
    y = qBound(src.top(), y, src.bottom());
    return QPointF(x, y);
}

MinimapWidget::ResizeMode MinimapWidget::resizeModeAt(const QPoint &pos) const
{
    const int handleSize = 8;
    const QRect r = rect();
    const bool nearLeft = pos.x() <= handleSize;
    const bool nearRight = pos.x() >= r.width() - handleSize;
    const bool nearTop = pos.y() <= handleSize;
    const bool nearBottom = pos.y() >= r.height() - handleSize;

    if (nearTop && nearLeft) {
        return ResizeMode::TopLeft;
    }
    if (nearTop && nearRight) {
        return ResizeMode::TopRight;
    }
    if (nearBottom && nearLeft) {
        return ResizeMode::BottomLeft;
    }
    if (nearBottom && nearRight) {
        return ResizeMode::BottomRight;
    }
    if (nearLeft) {
        return ResizeMode::Left;
    }
    if (nearRight) {
        return ResizeMode::Right;
    }
    if (nearTop) {
        return ResizeMode::Top;
    }
    if (nearBottom) {
        return ResizeMode::Bottom;
    }
    return ResizeMode::None;
}

Qt::CursorShape MinimapWidget::cursorForResizeMode(ResizeMode mode) const
{
    switch (mode) {
    case ResizeMode::Top:
    case ResizeMode::Bottom:
        return Qt::SizeVerCursor;
    case ResizeMode::Left:
    case ResizeMode::Right:
        return Qt::SizeHorCursor;
    case ResizeMode::TopLeft:
    case ResizeMode::BottomRight:
        return Qt::SizeFDiagCursor;
    case ResizeMode::TopRight:
    case ResizeMode::BottomLeft:
        return Qt::SizeBDiagCursor;
    default:
        return Qt::ArrowCursor;
    }
}

void MinimapWidget::applyResize(const QPoint &globalPos)
{
    const QPoint delta = globalPos - m_lastGlobalPos;
    const QRect oldGeom = geometry();
    QRect geom = oldGeom;

    // Keep the widget's own aspect ratio stable while resizing -- based on the scene's
    // content, same fallback chain as computeTransform() (itemsBoundingRect, then
    // sceneRect), so a resize while empty/degenerate still has a sane 1:1 ratio.
    const auto sceneAspectRatio = [this]() -> double {
        QRectF src = m_scene ? m_scene->itemsBoundingRect() : QRectF();
        if (!src.isValid() || src.isEmpty())
            src = m_scene ? m_scene->sceneRect() : QRectF();
        if (!src.isValid() || src.width() <= 0.0 || src.height() <= 0.0)
            return 1.0;
        return src.width() / src.height();
    };

    const double aspect = sceneAspectRatio();
    int newWidth = geom.width();
    int newHeight = geom.height();

    const bool top = m_resizeMode == ResizeMode::Top || m_resizeMode == ResizeMode::TopLeft || m_resizeMode == ResizeMode::TopRight;
    const bool bottom = m_resizeMode == ResizeMode::Bottom || m_resizeMode == ResizeMode::BottomLeft || m_resizeMode == ResizeMode::BottomRight;
    const bool left = m_resizeMode == ResizeMode::Left || m_resizeMode == ResizeMode::TopLeft || m_resizeMode == ResizeMode::BottomLeft;
    const bool right = m_resizeMode == ResizeMode::Right || m_resizeMode == ResizeMode::TopRight || m_resizeMode == ResizeMode::BottomRight;

    if ((top && right) || (top && left) || (bottom && right) || (bottom && left)) {
        // Diagonal handle: follow whichever axis the cursor is moving faster along, derive
        // the other from the locked aspect ratio.
        const bool horizontalDominant = qAbs(delta.x()) >= qAbs(delta.y());
        if (horizontalDominant) {
            newWidth = left ? geom.width() - delta.x() : geom.width() + delta.x();
            newHeight = qRound(newWidth / aspect);
        } else {
            newHeight = top ? geom.height() - delta.y() : geom.height() + delta.y();
            newWidth = qRound(newHeight * aspect);
        }
    } else if (top || bottom) {
        newHeight = top ? geom.height() - delta.y() : geom.height() + delta.y();
        newWidth = qRound(newHeight * aspect);
    } else if (left || right) {
        newWidth = left ? geom.width() - delta.x() : geom.width() + delta.x();
        newHeight = qRound(newWidth / aspect);
    }

    newWidth = qMax(minimumWidth(), newWidth);
    newHeight = qMax(minimumHeight(), newHeight);
    if (newWidth < minimumWidth()) {
        newWidth = minimumWidth();
        newHeight = qMax(minimumHeight(), qRound(newWidth / aspect));
    }
    if (newHeight < minimumHeight()) {
        newHeight = minimumHeight();
        newWidth = qMax(minimumWidth(), qRound(newHeight * aspect));
    }

    geom.setSize(QSize(newWidth, newHeight));
    if (top) {
        geom.setTop(oldGeom.top() + (oldGeom.height() - newHeight));
    }
    if (left) {
        geom.setLeft(oldGeom.left() + (oldGeom.width() - newWidth));
    }

    if (geom != oldGeom) {
        setGeometry(geom);
    }
    m_lastGlobalPos = globalPos;
}

QRect MinimapWidget::moveHandleRect() const
{
    return QRect(0, 0, width(), 24);
}

bool MinimapWidget::isMoveHandle(const QPoint &pos) const
{
    return moveHandleRect().contains(pos);
}

void MinimapWidget::moveBy(const QPoint &delta)
{
    if (!parentWidget())
        return;

    const QPoint newPos = pos() + delta;
    const QRect parentRect = parentWidget()->rect();
    const int margin = 12;
    const int x = qBound(margin, newPos.x(), parentRect.width() - width() - margin);
    const int y = qBound(margin, newPos.y(), parentRect.height() - height() - margin);
    move(x, y);
}

void MinimapWidget::drawMoveHandle(QPainter &painter) const
{
    const QRect handle = moveHandleRect();
    const bool highlighted = m_moving || m_hoverMoveHandle;

    QColor bg = highlighted ? palette().highlight().color() : palette().windowText().color();
    bg.setAlpha(highlighted ? 90 : 35);
    painter.fillRect(handle, bg);

    // Grip dots (2 rows x 3 columns), the same affordance used for draggable strips/handles
    // elsewhere (e.g. splitter/dock handles) -- signals "grab here" at a glance.
    QColor dotColor = palette().windowText().color();
    dotColor.setAlpha(highlighted ? 230 : 140);
    painter.setPen(Qt::NoPen);
    painter.setBrush(dotColor);

    const QPointF center = handle.center();
    constexpr double dotRadius = 1.2;
    constexpr double spacingX = 6.0;
    constexpr double spacingY = 5.0;
    for (const double rowOffset : {-spacingY / 2.0, spacingY / 2.0}) {
        for (const double colOffset : {-spacingX, 0.0, spacingX}) {
            painter.drawEllipse(center + QPointF(colOffset, rowOffset), dotRadius, dotRadius);
        }
    }
}

void MinimapWidget::drawResizeGrips(QPainter &painter) const
{
    struct Corner {
        ResizeMode mode;
        QPoint origin;
        int armX; // arm direction along x: +1 rightwards, -1 leftwards
        int armY; // arm direction along y: +1 downwards, -1 upwards
    };

    constexpr int inset = 3;
    constexpr int armLength = 9;
    const QRect r = rect();
    const Corner corners[] = {
        {ResizeMode::TopLeft, QPoint(inset, inset), 1, 1},
        {ResizeMode::TopRight, QPoint(r.width() - 1 - inset, inset), -1, 1},
        {ResizeMode::BottomLeft, QPoint(inset, r.height() - 1 - inset), 1, -1},
        {ResizeMode::BottomRight, QPoint(r.width() - 1 - inset, r.height() - 1 - inset), -1, -1},
    };

    for (const Corner &corner : corners) {
        const bool highlighted = m_resizing ? m_resizeMode == corner.mode : m_hoverResizeMode == corner.mode;
        QColor color = highlighted ? palette().highlight().color() : palette().windowText().color();
        color.setAlpha(highlighted ? 230 : 130);
        QPen pen(color);
        pen.setWidthF(highlighted ? 2.0 : 1.4);
        painter.setPen(pen);
        painter.drawLine(corner.origin, corner.origin + QPoint(corner.armX * armLength, 0));
        painter.drawLine(corner.origin, corner.origin + QPoint(0, corner.armY * armLength));
    }
}

void MinimapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_resizing) {
        applyResize(event->globalPosition().toPoint());
        invalidateCache();
        event->accept();
        return;
    }

    if (m_moving) {
        moveBy(event->globalPosition().toPoint() - m_lastGlobalPos);
        m_lastGlobalPos = event->globalPosition().toPoint();
        event->accept();
        return;
    }

    if (!m_dragging) {
        updateHoverState(event->pos());
    }

    if (!m_dragging || !m_scene || !m_view) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    m_view->centerOn(widgetToScene(event->pos()));
    updateOverlay();
    event->accept();
}

void MinimapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_resizing) {
        m_resizing = false;
        m_resizeMode = ResizeMode::None;
        releaseMouse();
        event->accept();
        emit geometryChangeFinished(geometry());
        return;
    }
    if (m_moving) {
        m_moving = false;
        releaseMouse();
        event->accept();
        emit geometryChangeFinished(geometry());
        return;
    }
    if (m_dragging) {
        m_dragging = false;
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void MinimapWidget::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    updateHoverState(mapFromGlobal(QCursor::pos()));
}

void MinimapWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    unsetCursor();
    if (m_hoverResizeMode != ResizeMode::None || m_hoverMoveHandle) {
        m_hoverResizeMode = ResizeMode::None;
        m_hoverMoveHandle = false;
        update();
    }
}

void MinimapWidget::updateHoverState(const QPoint &pos)
{
    const ResizeMode mode = resizeModeAt(pos);
    const bool onMoveHandle = mode == ResizeMode::None && isMoveHandle(pos);

    if (mode != ResizeMode::None) {
        setCursor(cursorForResizeMode(mode));
    } else if (onMoveHandle) {
        setCursor(Qt::OpenHandCursor);
    } else {
        unsetCursor();
    }

    if (mode != m_hoverResizeMode || onMoveHandle != m_hoverMoveHandle) {
        m_hoverResizeMode = mode;
        m_hoverMoveHandle = onMoveHandle;
        update(); // repaint so the corner/move-strip highlight tracks the hover state
    }
}
