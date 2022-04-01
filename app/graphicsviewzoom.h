/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QObject>
#include <QPointF>

/*!
 * This class adds ability to zoom QGraphicsView using mouse wheel. The point under cursor
 * remains motionless while it's possible.
 *
 * Note that it becomes not possible when the scene's
 * size is not large enough comparing to the viewport size. QGraphicsView centers the picture
 * when it's smaller than the view. And QGraphicsView's scrolls boundaries don't allow to
 * put any picture point at any viewport position.
 *
 * When the user starts scrolling, this class remembers original scene position and
 * keeps it until scrolling is completed. It's better than getting original scene position at
 * each scrolling step because that approach leads to position errors due to before-mentioned
 * positioning restrictions.
 *
 * When zommed using scroll, this class emits zoomed() signal.
 *
 * Usage:
 *
 *   new Graphics_view_zoom(view);
 *
 * The object will be deleted automatically when the view is deleted.
 *
 * You can set keyboard modifiers used for zooming using set_modified(). Zooming will be
 * performed only on exact match of modifiers combination. The default modifier is Ctrl.
 *
 * You can change zoom velocity by calling set_zoom_factor_base().
 * Zoom coefficient is calculated as zoom_factor_base^angle_delta
 * (see QWheelEvent::angleDelta).
 * The default zoom factor base is 1.0015.
 */

class QGraphicsView;

class GraphicsViewZoom : public QObject
{
    Q_OBJECT

public:
    explicit GraphicsViewZoom(QGraphicsView *view);

    static constexpr double maxZoom = 1.5;
    static constexpr double minZoom = 0.2;

    bool canZoomIn();
    bool canZoomOut();
    void gentleZoom(double factor);
    void resetZoom();
    void setModifiers(Qt::KeyboardModifiers modifiers);
    void setZoomFactorBase(double value);
    void zoomIn();
    void zoomOut();

signals:
    void zoomed();

private:
    bool eventFilter(QObject *object, QEvent *event) override;
    double scaleFactor();
    void setScaleFactor(double factor);

    QGraphicsView *m_view;
    QPointF m_targetScenePos, m_targetViewportPos;
    Qt::KeyboardModifiers m_modifiers;
    double m_zoomFactorBase;
};

