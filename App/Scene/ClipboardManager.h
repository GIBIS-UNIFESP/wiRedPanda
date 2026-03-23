// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ClipboardManager: handles copy, cut, paste and clone-drag operations.
 */

#pragma once

#include <QDataStream>
#include <QList>
#include <QPointF>
#include <QVersionNumber>

class QGraphicsItem;
class Scene;

/**
 * \class ClipboardManager
 * \brief Manages clipboard operations (copy/cut/paste) and clone-drag for the circuit scene.
 *
 * Extracted from Scene to decouple clipboard serialization and blob-registry
 * handling from the main circuit scene.  Delegates item creation and undo
 * commands back to the owning Scene.
 */
class ClipboardManager
{
public:
    explicit ClipboardManager(Scene *scene);

    /// Copies the currently selected items to the system clipboard.
    void copy();

    /// Cuts the currently selected items (copy + delete).
    void cut();

    /// Pastes items from the system clipboard into the scene.
    void paste();

    /// Initiates a clone-drag (Ctrl+drag) of the current selection.
    void cloneDrag(const QPointF &mousePos);

private:
    /// Serializes \a items into \a stream (centroid + element data).
    static void serializeItems(const QList<QGraphicsItem *> &items, QDataStream &stream);

    /// Serializes \a items and then deletes them from the scene.
    void serializeAndDelete(const QList<QGraphicsItem *> &items, QDataStream &stream);

    /// Deserializes from \a stream and adds to the scene with a positional offset.
    void deserializeAndAdd(QDataStream &stream, const QVersionNumber &version);

    Scene *m_scene = nullptr;
};

