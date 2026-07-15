// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QEnterEvent>
#include <QPixmap>
#include <QRect>
#include <QTimer>
#include <QWidget>

class Scene;
class GraphicsView;
class QPainter;

class MinimapWidget : public QWidget
{
    Q_OBJECT

    friend class TestMinimapWidget;

public:
    explicit MinimapWidget(Scene *scene, GraphicsView *view, QWidget *parent = nullptr);

    QSize sizeHint() const override;

public slots:
    void invalidateCache();
    void onThrottleTimeout();
    void updateOverlay();

signals:
    /// Emitted once a drag-move or drag-resize finishes (mouseReleaseEvent), carrying the
    /// widget's final geometry -- not on every mouseMoveEvent, so listeners persisting this
    /// (e.g. to Settings) aren't spammed mid-drag.
    void geometryChangeFinished(const QRect &geometry);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override; // swallow wheel events so main view doesn't zoom
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    enum class ResizeMode {
        None,
        Top,
        Bottom,
        Left,
        Right,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    Scene *m_scene = nullptr;
    GraphicsView *m_view = nullptr;
    QPixmap m_cache;           ///< cached rendering of the scene for fast repaints
    bool m_cacheDirty = true;  ///< whether cache needs to be regenerated
    QRectF m_cacheSrc;         ///< source rect the cache was rendered from; a change invalidates it
    bool m_dragging = false;
    bool m_resizing = false;
    bool m_moving = false;
    QPoint m_lastGlobalPos;
    ResizeMode m_resizeMode = ResizeMode::None;
    ResizeMode m_hoverResizeMode = ResizeMode::None; ///< handle under the cursor, when not dragging
    bool m_hoverMoveHandle = false;                  ///< move strip under the cursor, when not dragging
    QTimer m_throttle;        ///< throttle timer to limit redraws

    // Compute mapping parameters used by both paint and mouse handling.
    bool computeTransform(QRectF &srcOut, double &scaleOut, double &dxOut, double &dyOut, double &usedWOut, double &usedHOut) const;
    QPointF widgetToScene(const QPointF &p) const;

    // Edge/corner resize handles.
    ResizeMode resizeModeAt(const QPoint &pos) const;
    Qt::CursorShape cursorForResizeMode(ResizeMode mode) const;
    void applyResize(const QPoint &globalPos);

    // Move handle: a strip along the top of the widget, dragged to reposition it freely.
    QRect moveHandleRect() const;
    bool isMoveHandle(const QPoint &pos) const;
    void moveBy(const QPoint &delta);

    // Cursor + hover-highlight state for the handles at pos, shared by mouseMoveEvent()
    // and enterEvent() (and called directly by tests, since it takes a plain QPoint).
    void updateHoverState(const QPoint &pos);

    // Visual affordances painted so the handles are discoverable without hovering first.
    void drawMoveHandle(QPainter &painter) const;
    void drawResizeGrips(QPainter &painter) const;
};
