// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Extended QGraphicsView with zoom, pan, and fast-rendering modes.
 */

#pragma once

#include <QGraphicsView>

/**
 * \class GraphicsView
 * \brief Extended QGraphicsView with enhanced navigation capabilities.
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

    /// Constructs the view with optional \a parent.
    explicit GraphicsView(QWidget *parent = nullptr);

    // --- Zoom ---

    /// Returns \c true if zooming in further is possible.
    bool canZoomIn() const;

    /// Returns \c true if zooming out further is possible.
    bool canZoomOut() const;

    /// Resets the view scale to 1:1.
    void resetZoom();

    /// Increases the view magnification by one zoom step.
    void zoomIn();

    /// Decreases the view magnification by one zoom step.
    void zoomOut();

    // --- Settings ---

    /**
     * \brief Enables or disables fast rendering mode (disables antialiasing).
     * \param fastMode \c true to use fast (non-antialiased) rendering.
     */
    void setFastMode(const bool fastMode);

    /**
     * \brief When enabled, wheel zoom events emit scaleIn/scaleOut instead of zooming directly.
     * \param value \c true to redirect zoom events.
     */
    void setRedirectZoom(const bool value);

signals:
    /// Emitted when redirectZoom is true and the user scrolls in.
    void scaleIn();

    /// Emitted when redirectZoom is true and the user scrolls out.
    void scaleOut();

    /// Emitted whenever the zoom level changes.
    void zoomChanged();

protected:
    // --- Event handlers ---

    /// \reimp
    void keyPressEvent(QKeyEvent *event) override;
    /// \reimp
    void keyReleaseEvent(QKeyEvent *event) override;
    /// \reimp
    void mouseMoveEvent(QMouseEvent *event) override;
    /// \reimp
    void mousePressEvent(QMouseEvent *event) override;
    /// \reimp
    void mouseReleaseEvent(QMouseEvent *event) override;
    /// \reimp
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

