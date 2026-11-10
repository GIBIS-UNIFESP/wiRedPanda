// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief InlineLabelEditor: on-canvas inline editing of an element's label.
 */

#pragma once

#include <QMetaObject>
#include <QPointer>

class GraphicElement;
class QGraphicsProxyWidget;
class Scene;

/**
 * \class InlineLabelEditor
 * \brief Hosts a temporary QLineEdit directly on the canvas to rename an element's label
 * in place, in response to GraphicElement::inlineEditRequested().
 *
 * \details Follows the SceneDropHandler/ConnectionManager pattern: holds a back-reference to
 * the owning Scene and delegates the undo command back to it. Only one inline edit is ever
 * active at a time; starting a new one commits whatever was already in progress. Deleting
 * the element under edit closes its editor without committing.
 */
class InlineLabelEditor
{
public:
    explicit InlineLabelEditor(Scene *scene);
    ~InlineLabelEditor();

    /// Starts inline-editing \a element's label, committing any already-in-progress edit first.
    void start(GraphicElement *element);

    /// Commits the current edit (if any): pushes an UpdateCommand only if the text actually
    /// changed, then closes the editor. Safe to call when nothing is being edited (no-op).
    void commit();

    /// Discards the current edit (if any) and closes the editor without changing anything.
    void cancel();

private:
    void close();

    Scene *m_scene;
    /// Elements are deleted outright (Delete, Cut, undoing their Add...), possibly while
    /// their editor is still open, so the target must never be held as a raw pointer.
    QPointer<GraphicElement> m_target;
    QGraphicsProxyWidget *m_proxy = nullptr;
    /// Closes the editor when m_target is destroyed. Severed by close() and by the destructor,
    /// which runs before the owning Scene deletes its items.
    QMetaObject::Connection m_targetDestroyed;
};
