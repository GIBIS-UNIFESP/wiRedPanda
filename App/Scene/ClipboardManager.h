// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ClipboardManager: handles copy, cut, paste and clone-drag operations.
 */

#pragma once

#include <QCoreApplication>
#include <QDataStream>
#include <QList>
#include <QPointF>
#include <QVersionNumber>

class QGraphicsItem;
class QImage;
class QMimeData;
class QRectF;
class QTransform;
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
    Q_DECLARE_TR_FUNCTIONS(ClipboardManager)

public:
    explicit ClipboardManager(Scene *scene);

    /// Copies the currently selected items to the system clipboard.
    void copy();

    /// Cuts the currently selected items (copy + delete).
    void cut();

    /// Pastes items from the system clipboard into the scene.
    void paste();

    /**
     * \brief Returns whether \a mimeData carries circuit data paste() accepts.
     * \details Single source of truth for paste gating (context menu, future
     * action enabling): paste() reads both the current and the legacy mime
     * format, so any gate that checks only one of them silently drifts.
     */
    static bool canPaste(const QMimeData *mimeData);

    /// Initiates a clone-drag (Ctrl+drag) of the current selection.
    void cloneDrag(const QPointF &mousePos);

    /// Maximum width/height (in device pixels) buildDragImage() will allocate. A drag ghost is
    /// a transient visual aid, not saved output, so a modest cap is enough — selection extent
    /// beyond this is scaled down to fit rather than sized proportionally.
    static constexpr int kMaxDragImageDimension = 1024;

    /**
     * \brief Renders \a sourceRect of \a scene, mapped through \a viewTransform, into a
     * transparent-filled QImage bounded by kMaxDragImageDimension.
     * \details \a sourceRect is a union of selected elements' scene bounding rects — unbounded
     * magnitude, since position validation on load only rejects non-finite coordinates, never
     * bounds them. Scaled down to fit rather than allocated proportionally. Exposed for testing.
     */
    static QImage buildDragImage(Scene *scene, const QTransform &viewTransform, const QRectF &sourceRect);

private:
    /// Serializes \a items into \a stream (centroid + element data).
    static void serializeItems(const QList<QGraphicsItem *> &items, QDataStream &stream);

    /// Serializes \a items and then deletes them from the scene.
    void serializeAndDelete(const QList<QGraphicsItem *> &items, QDataStream &stream);

    /// Deserializes from \a stream and adds to the scene with a positional offset.
    void deserializeAndAdd(QDataStream &stream, const QVersionNumber &version);

    Scene *m_scene = nullptr;
};
