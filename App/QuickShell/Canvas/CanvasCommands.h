// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief CanvasItem-side ports of App/Scene/Commands.h's QUndoCommand subclasses.
 */

#pragma once

#include <QList>
#include <QPointF>
#include <QUndoCommand>

class CanvasItem;
class GraphicElement;

/**
 * \class CanvasElementsCommand
 * \brief CanvasItem-side port of Commands.h's ElementsCommand.
 *
 * \details Stores the stable element ids captured at construction time (via
 * GraphicElement::id(), the real ItemWithId id CanvasItem::addItem() assigns -- see
 * CanvasItem.h's "Real id/registry layer" doc comment) and resolves them back to live
 * pointers through CanvasItem::itemById(), exactly mirroring ElementsCommand's id-based
 * resolution against Scene::itemById(). Unlike CommandUtils::findElements(), a missing id
 * is silently skipped rather than thrown (Scene's PANDACEPTION/Sentry error-reporting
 * infrastructure isn't wired into this Quick-only tree) -- harmless while nothing deletes
 * elements yet; revisit once the Add/Delete command family lands.
 */
class CanvasElementsCommand : public QUndoCommand
{
protected:
    explicit CanvasElementsCommand(const QList<GraphicElement *> &elements, CanvasItem *canvas, QUndoCommand *parent = nullptr);

    /// Returns the live element pointers for this command's targets, looked up by stored id.
    [[nodiscard]] QList<GraphicElement *> elements() const;

    CanvasItem *m_canvas;
    QList<int> m_ids;
};

/**
 * \class CanvasMoveCommand
 * \brief Port of Commands.h's MoveCommand: records a drag-move (or nudge) of a set of elements.
 */
class CanvasMoveCommand : public CanvasElementsCommand
{
public:
    explicit CanvasMoveCommand(const QList<GraphicElement *> &elements, const QList<QPointF> &oldPositions,
                               CanvasItem *canvas, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QList<QPointF> m_newPositions;
    QList<QPointF> m_oldPositions;
};

/**
 * \class CanvasRotateCommand
 * \brief Port of Commands.h's RotateCommand: rotates a list of elements by a fixed angle
 * around their shared centroid.
 */
class CanvasRotateCommand : public CanvasElementsCommand
{
public:
    explicit CanvasRotateCommand(const QList<GraphicElement *> &items, int angle, CanvasItem *canvas, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    int m_angle;
    QList<QPointF> m_positions;
};

/**
 * \class CanvasFlipCommand
 * \brief Port of Commands.h's FlipCommand: flips a selection of elements along a horizontal
 * or vertical axis. Involution -- undo() == redo().
 */
class CanvasFlipCommand : public CanvasElementsCommand
{
public:
    /// \a axis: 0 = horizontal flip (mirror left-right), 1 = vertical flip (mirror top-bottom).
    explicit CanvasFlipCommand(const QList<GraphicElement *> &items, int axis, CanvasItem *canvas, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QList<QPointF> m_positions;
    QPointF m_maxPos;
    QPointF m_minPos;
    int m_axis;
};
