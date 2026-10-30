// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief InlineLabelEditor: on-canvas inline editing of an element's label.
 */

#pragma once

#include <QCoreApplication>

class Element;
class QGraphicsProxyWidget;
class Scene;

/**
 * \class InlineLabelEditor
 * \brief Hosts a temporary QLineEdit directly on the canvas to rename an element's label
 * in place, in response to Element::inlineEditRequested().
 *
 * \details Follows the SceneDropHandler/ConnectionManager pattern: holds a back-reference to
 * the owning Scene and delegates the undo command back to it. Only one inline edit is ever
 * active at a time; starting a new one commits whatever was already in progress.
 */
class InlineLabelEditor
{
    Q_DECLARE_TR_FUNCTIONS(InlineLabelEditor)

public:
    explicit InlineLabelEditor(Scene *scene);
    ~InlineLabelEditor();

    /// Starts inline-editing \a element's label, committing any already-in-progress edit first.
    void start(Element *element);

    /// Commits the current edit (if any): pushes an UpdateCommand only if the text actually
    /// changed, then closes the editor. Safe to call when nothing is being edited (no-op).
    /// A label that would give two Tx nodes one channel is refused (Scene::duplicateTxLabel()),
    /// with a message in the status bar.
    void commit();

    /// Discards the current edit (if any) and closes the editor without changing anything.
    void cancel();

private:
    void close();

    Scene *m_scene;
    Element *m_target = nullptr;
    QGraphicsProxyWidget *m_proxy = nullptr;
};
