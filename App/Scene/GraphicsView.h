// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QGraphicsView>

/*!
 * @class GraphicsView
 * @brief Extended QGraphicsView with enhanced navigation capabilities
 *
 * GraphicsView extends the QGraphicsView class to provide additional
 * functionality for zooming, panning, and keyboard navigation. It includes
 * signals to notify when zoom operations occur and supports redirect zoom
 * mode for delegating zoom operations to parent components.
 */
class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit GraphicsView(QWidget *parent = nullptr);

    // --- Zoom ---

    bool canZoomIn() const;
    bool canZoomOut() const;
    void resetZoom();
    void zoomIn();
    void zoomOut();

    // --- Settings ---

    void setFastMode(const bool fastMode);
    void setRedirectZoom(const bool value);

signals:
    // --- Zoom signals ---

    void scaleIn();
    void scaleOut();
    void zoomChanged();

protected:
    // --- Event handlers ---

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    // --- Members ---

    bool m_redirectZoom = false;
    int m_zoomLevel = 0;

private:
    // --- Members ---

    bool m_pan = false;
    bool m_space = false;
    int m_panStartX = 0;
    int m_panStartY = 0;
};

