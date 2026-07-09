// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief SceneDropHandler: decodes drag-and-drop payloads into scene elements.
 */

#pragma once

#include <optional>

#include <QPointF>

class QGraphicsSceneDragDropEvent;
class QMimeData;
class Scene;

/**
 * \class SceneDropHandler
 * \brief Decodes wiRedPanda drag-and-drop payloads (new element from the palette,
 * and clone-drag of an existing selection) and commits them to the owning Scene.
 *
 * \details Extracted from Scene, following the ConnectionManager pattern: it holds a
 * back-reference to the Scene and delegates element creation, undo commands, and
 * scene resizing back to it. Scene keeps the QGraphicsScene drag/drop event
 * overrides and forwards the payload handling here.
 */
class SceneDropHandler
{
public:
    explicit SceneDropHandler(Scene *scene);

    /// Returns \c true if \a mimeData carries any recognised wiRedPanda drop format.
    static bool isSupportedDropFormat(const QMimeData *mimeData);

    /// Builds a new element from a palette drag payload and adds it under the cursor.
    void handleNewElementDrop(QGraphicsSceneDragDropEvent *event);

    /// Re-instantiates a clone-dragged selection at the drop position.
    void handleCloneDrag(QGraphicsSceneDragDropEvent *event);

    /// Adds a new element from \a mimeData (palette add-without-drag path); takes ownership
    /// of \a mimeData. If \a scenePos is set, the element is placed there (snapped to the
    /// grid by the scene); otherwise it keeps its default position.
    void addFromMimeData(QMimeData *mimeData, std::optional<QPointF> scenePos = std::nullopt);

private:
    Scene *m_scene;
};
