// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickMinimap: QML-facing presenter driving the circuit-overview minimap.
 */

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QRect>
#include <QRectF>
#include <QString>
#include <QTimer>

#include "App/Core/Settings.h"

class CanvasItem;

/**
 * \class QuickMinimap
 * \brief Copy-and-adapted, Widgets-free port of App/UI/MinimapWidget's overview/navigate
 * behavior for the Quick chrome.
 *
 * \details Drives Minimap.qml's imageUrl/viewportRect bindings the way MinimapWidget's own
 * paintEvent() draws its cached thumbnail and viewport-rectangle overlay -- but resize/move
 * interaction and the thumbnail's own background/border chrome live directly in Minimap.qml
 * (MouseArea-driven, QML-idiomatic), not ported pixel-for-pixel from MinimapWidget's
 * ResizeMode/moveHandleRect() machinery; only the *content* (what to show, where the viewport
 * rect sits, aspect-locking data for the resize drag, click/drag-to-navigate) is this
 * presenter's job. setCanvas() connects to the bound CanvasItem's undoStack()->indexChanged()
 * (content changed -- mirrors Scene::circuitHasChanged(), itself wired to the same
 * QUndoStack::indexChanged signal) and zoomChanged() (pan/zoom changed -- mirrors
 * GraphicsView::zoomChanged()/its scrollbar valueChanged connections), coalescing both into one
 * throttled regen: unlike MinimapWidget's paintEvent(), which recomputes its source rect (and
 * therefore re-renders) on every single paint whenever the viewport differs from the last
 * cache, this class's renderMinimapImage() call is a real per-element CPU paint (the same
 * offscreen-QPainter-plus-real-paint() technique renderICPreviewImage()/renderExportImage()
 * use), not a cheap downscale of an already-rendered QGraphicsScene -- eager re-render on every
 * pan/zoom step would be a real cost regression, so both content and view changes share one
 * throttle here, a deliberate difference from MinimapWidget's split (content throttled, view
 * unthrottled) justified by the different rendering technique's cost shape.
 *
 * \details minimapWidth/minimapHeight are reported by Minimap.qml (mirroring
 * ElementPalette.qml's canvasWidth/canvasHeight-reported-by-Main.qml precedent) since this
 * presenter has no QQuickItem of its own to read a geometry from -- QML owns the minimap
 * item's actual on-screen size (including user resize), and tells this presenter what to
 * render at.
 */
class QuickMinimap : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("QuickMinimap is only ever exposed via AppController.minimap")

    // FINAL: QuickMinimap is never subclassed, matching every other presenter in this Quick
    // chrome since the AOT-compilation refactor (see project_qml_aot_compilation_fusion_style_pin
    // memory).
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged FINAL)
    Q_PROPERTY(QString imageUrl READ imageUrl NOTIFY refreshed FINAL)
    Q_PROPERTY(QRectF viewportRect READ viewportRect NOTIFY refreshed FINAL)
    Q_PROPERTY(qreal minimapWidth READ minimapWidth WRITE setMinimapWidth NOTIFY minimapSizeChanged FINAL)
    Q_PROPERTY(qreal minimapHeight READ minimapHeight WRITE setMinimapHeight NOTIFY minimapSizeChanged FINAL)

public:
    explicit QuickMinimap(QObject *parent = nullptr);

    /// Binds this presenter to \a canvas's content/view-change signals; passing nullptr unbinds
    /// and clears the current thumbnail. Mirrors QuickElementEditor::setCanvas()'s shape.
    void setCanvas(CanvasItem *canvas);

    [[nodiscard]] bool isVisible() const { return m_visible; }
    /// Also persists to Settings::setMinimapVisible() -- mirrors
    /// MainWindow::on_actionShowMinimap_triggered()'s pairing of the checked-state change with
    /// the persisted preference.
    void setVisible(bool visible);

    /// A "data:image/png;base64,..." URL directly usable as a QML Image.source, or empty if
    /// there's nothing to show yet (hidden, unbound, or an empty circuit).
    [[nodiscard]] QString imageUrl() const { return m_imageUrl; }
    /// The current viewport, in minimap-local pixel coordinates -- Minimap.qml draws this as a
    /// plain bordered Rectangle overlay. Empty while hidden/unbound/empty.
    [[nodiscard]] QRectF viewportRect() const { return m_viewportRect; }

    [[nodiscard]] qreal minimapWidth() const { return m_minimapWidth; }
    void setMinimapWidth(qreal width);
    [[nodiscard]] qreal minimapHeight() const { return m_minimapHeight; }
    void setMinimapHeight(qreal height);

    /// Aspect ratio (width/height) of the bound canvas's current circuit content, or 1.0 if
    /// there is no canvas or its content is empty. Mirrors
    /// MinimapWidget::applyResize()'s sceneAspectRatio() lambda -- Minimap.qml's resize-handle
    /// drag math calls this to keep the widget's own aspect locked to what it displays, the
    /// same way MinimapWidget's C++ resize logic does.
    Q_INVOKABLE qreal contentAspectRatio() const;

    /// Pans the bound canvas so the world point under minimap-local (\a localX, \a localY)
    /// becomes the new viewport center, preserving zoom. Mirrors
    /// MinimapWidget::mousePressEvent()/mouseMoveEvent()'s "centerOn(widgetToScene(pos))"
    /// click/drag-to-navigate gesture -- clamping to content bounds happens inside
    /// CanvasItem::minimapContentRect()/centerOn() naturally (a point outside the rendered
    /// thumbnail still resolves to a valid, if extreme, world point). No-op if there is no
    /// bound canvas or no content to navigate.
    Q_INVOKABLE void navigateTo(qreal localX, qreal localY) const;

    /// The persisted minimap position/size (Settings::minimapGeometry()), or an invalid QRect
    /// if never set -- Minimap.qml reads this once (Component.onCompleted) to restore its own
    /// x/y/width/height, then owns that geometry itself for the rest of the session (mirroring
    /// WorkSpace::applyMinimapGeometry()'s one-time restore, m_minimapPositioned). Kept on this
    /// presenter, not exposed via a QML Settings singleton, so Settings access stays
    /// centralized in C++ -- the same shape as recentFiles()/examplesList()/languages() bridge
    /// Settings-backed data to QML elsewhere in this class.
    Q_INVOKABLE QRect initialGeometry() const { return Settings::minimapGeometry(); }
    /// Persists \a x/\a y/\a width/\a height as the user's minimap placement/size. Mirrors
    /// WorkSpace::onMinimapGeometryChangeFinished() -- call only when a drag/resize gesture
    /// finishes, not on every intermediate frame, so Settings isn't spammed mid-drag.
    Q_INVOKABLE void commitGeometry(int x, int y, int width, int height) const
    {
        Settings::setMinimapGeometry(QRect(x, y, width, height));
    }

signals:
    void visibleChanged();
    void refreshed();
    void minimapSizeChanged();

private:
    /// (Re)starts the coalescing regen timer -- mirrors MinimapWidget::invalidateCache()'s
    /// throttle-restart shape, extended here to also cover view (pan/zoom) changes; see this
    /// class's own doc comment for why both share one throttle.
    void requestRegen();
    /// Mirrors MinimapWidget::onThrottleTimeout() + its paintEvent()'s cache-regen branch,
    /// combined: regenerates the thumbnail image and the viewport-rect overlay together from
    /// the same content rect, so they can never disagree about what region is being shown.
    void regenerate();

    CanvasItem *m_canvas = nullptr;
    QTimer m_regenTimer;

    bool m_visible = true;
    QString m_imageUrl;
    QRectF m_viewportRect;
    qreal m_minimapWidth = 0.0;
    qreal m_minimapHeight = 0.0;
};
