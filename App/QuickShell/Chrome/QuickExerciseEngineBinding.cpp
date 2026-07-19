// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ExerciseEngine::setCanvas() -- the one method of the shared ExerciseEngine class that
 * needs CanvasItem's complete type. Kept in its own translation unit, compiled only into
 * wiredpanda_quick, so ExerciseEngine.cpp itself (whole-archived Layer 1 code, compiled once
 * into wiredpanda_lib) never needs to include a real QQuickItem header -- see setCanvas()'s own
 * doc comment on App/Exercise/ExerciseEngine.h for the full reasoning.
 */

#include "App/Exercise/ExerciseEngine.h"

#include <QPointer>

#include "App/QuickShell/Canvas/CanvasItem.h"

void ExerciseEngine::setCanvas(CanvasItem *canvas)
{
    if (m_disconnectFn) {
        m_disconnectFn();
    }
    if (canvas) {
        // A local QPointer, captured by value into each closure below -- not a class member --
        // mirroring setScene()'s identical technique (see its own comment for why this class
        // never stores CanvasItem* as a member).
        const QPointer<CanvasItem> safeCanvas(canvas);
        m_connectFn = [this, safeCanvas] {
            if (safeCanvas) {
                connect(safeCanvas->undoStack(), &QUndoStack::indexChanged, this, &ExerciseEngine::onCircuitChanged);
            }
        };
        m_disconnectFn = [this, safeCanvas] {
            if (safeCanvas) {
                disconnect(safeCanvas->undoStack(), &QUndoStack::indexChanged, this, &ExerciseEngine::onCircuitChanged);
            }
        };
        m_elementsFn = [safeCanvas] {
            return safeCanvas ? safeCanvas->elements() : QVector<GraphicElement *>{};
        };
    } else {
        m_connectFn = nullptr;
        m_disconnectFn = nullptr;
        m_elementsFn = nullptr;
    }
    if (canvas && isActive()) {
        m_connectFn();
    }
}
