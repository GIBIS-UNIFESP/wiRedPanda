// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPixmap>
#include <QPointF>

/**
 * \class DragPreview
 * \brief The picture of what an in-process drag will drop, which the scene draws where the drop will put it.
 *
 * \details The palette's drag and the selection's copy-drag do not hand their picture to QDrag: a Wayland compositor
 * that ignores a drag icon committed before the drag starts (Mutter before 50.2, Weston) shows none from Qt 6.10 on
 * (issue #367), and a picture under the cursor cannot show where the drop snaps. The source sets the picture for the
 * length of its drag through a Scope; the scene draws it while the drag is over it.
 */
class DragPreview
{
public:
    /// What a drag will drop, in scene units.
    struct Picture {
        /// The picture, its device pixel ratio making its size in device-independent pixels its size in the scene.
        QPixmap pixmap;
        /// The scene offset the drop subtracts from the cursor before snapping to the grid.
        QPointF grab;
        /// Where the picture's top-left corner sits from the snapped point.
        QPointF topLeft;
    };

    /// Holds \a picture as the current drag's for this scope's lifetime.
    class Scope
    {
    public:
        explicit Scope(const Picture &picture);
        ~Scope();
        Scope(const Scope &) = delete;
        Scope &operator=(const Scope &) = delete;
    };

    /// The current drag's picture; its pixmap is null when no in-process drag is running.
    static const Picture &current();

    /// Where \a picture's top-left corner goes with the cursor at \a scenePos: where the drop puts what it shows.
    static QPointF position(const Picture &picture, QPointF scenePos);

private:
    /// Created on first use, during a drag: a QPixmap cannot be constructed before the QGuiApplication.
    static Picture &storage();
};
