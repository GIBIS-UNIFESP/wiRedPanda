// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPixmap>
#include <QTimer>
#include <QWidget>

class Scene;
class GraphicsView;

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

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override; // swallow wheel events so main view doesn't zoom
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Scene *m_scene = nullptr;
    GraphicsView *m_view = nullptr;
    QPixmap m_cache;           ///< cached rendering of the scene for fast repaints
    bool m_cacheDirty = true;  ///< whether cache needs to be regenerated
    QRectF m_cacheSrc;         ///< source rect the cache was rendered from; a change invalidates it
    bool m_dragging = false;
    QTimer m_throttle;        ///< throttle timer to limit redraws

    // Compute mapping parameters used by both paint and mouse handling.
    bool computeTransform(QRectF &srcOut, double &scaleOut, double &dxOut, double &dyOut, double &usedWOut, double &usedHOut) const;
    QPointF widgetToScene(const QPointF &p) const;
};
