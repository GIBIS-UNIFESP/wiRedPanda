// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief SceneInteraction: owns the scene's mouse-driven editing state and gestures.
 */

#pragma once

#include <utility>

#include <QElapsedTimer>
#include <QGraphicsRectItem>
#include <QList>
#include <QPointer>
#include <QPointF>

class GraphicElement;
class QGraphicsSceneMouseEvent;
class Scene;
class ThemeAttributes;

/**
 * \class SceneInteraction
 * \brief Owns the interactive editing state of a Scene — element dragging, the
 * rubber-band selection rectangle, and the last cursor position — and implements the
 * press/move/release/double-click gestures.
 *
 * \details Extracted from Scene via ownership inversion, mirroring ConnectionManager:
 * it holds the drag snapshot, selection-box state and the selection-rect item, and
 * calls back to the Scene only for genuine scene operations (itemAt, selectedElements,
 * receiveCommand, resizeScene, contextMenu, the connection manager). The gesture
 * methods return \c true when the event is fully handled so Scene skips the base-class
 * call. (The mouse-move re-entrancy guard stays on Scene because it must wrap Scene's
 * own base-event call.)
 */
class SceneInteraction
{
public:
    explicit SceneInteraction(Scene *scene);

    /// Adds the selection rectangle to the scene and starts the drag throttle timer.
    /// Called from the Scene constructor body to preserve the original init order.
    void attachToScene();

    /// \returns \c true if the press was fully handled (caller skips the base call).
    bool mousePress(QGraphicsSceneMouseEvent *event);
    /// \returns \c true if the move was fully handled (caller skips the base call).
    bool mouseMove(QGraphicsSceneMouseEvent *event);
    /// \returns \c true if the release was fully handled (caller skips the base call).
    bool mouseRelease(QGraphicsSceneMouseEvent *event);
    /// \returns \c true if the double-click was fully handled (caller skips the base call).
    bool mouseDoubleClick(QGraphicsSceneMouseEvent *event);

    /// Last known cursor position in scene coordinates.
    [[nodiscard]] QPointF lastMousePos() const { return m_mousePos; }

    /// \c true while an element drag is in progress (drives Scene::resizeScene's expand-only mode).
    [[nodiscard]] bool isDraggingElement() const { return m_draggingElement; }

    /// Applies the theme's selection-rectangle brush and pen.
    void applyTheme(const ThemeAttributes &theme);

private:
    /// Begins a rubber-band selection anchored at the current cursor position.
    void startSelectionRect();

    Scene *m_scene;

    QGraphicsRectItem m_selectionRect;
    QElapsedTimer m_timer;
    QPointF m_mousePos;
    QPointF m_selectionStartPoint;

    /// Per-drag snapshot of (element, original position); QPointer auto-clears if an
    /// element is destroyed mid-drag (e.g. Delete shortcut while dragging).
    QList<std::pair<QPointer<GraphicElement>, QPointF>> m_dragSnapshot;
    bool m_draggingElement = false;
    bool m_markingSelectionBox = false;
};
