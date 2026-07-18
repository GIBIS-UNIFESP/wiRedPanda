// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Canvas/CanvasCommands.h"

#include <algorithm>

#include <QTransform>

#include "App/Element/GraphicElement.h"
#include "App/QuickShell/Canvas/CanvasItem.h"

CanvasElementsCommand::CanvasElementsCommand(const QList<GraphicElement *> &elements, CanvasItem *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_canvas(canvas)
{
    m_ids.reserve(elements.size());
    for (auto *elm : elements) {
        m_ids.append(elm->id());
    }
}

QList<GraphicElement *> CanvasElementsCommand::elements() const
{
    QList<GraphicElement *> result;
    result.reserve(m_ids.size());
    for (const int id : m_ids) {
        if (auto *elm = dynamic_cast<GraphicElement *>(m_canvas->itemById(id))) {
            result.append(elm);
        }
    }
    return result;
}

CanvasMoveCommand::CanvasMoveCommand(const QList<GraphicElement *> &elements, const QList<QPointF> &oldPositions,
                                     CanvasItem *canvas, QUndoCommand *parent)
    : CanvasElementsCommand(elements, canvas, parent)
    , m_oldPositions(oldPositions)
{
    m_newPositions.reserve(elements.size());
    for (auto *elm : elements) {
        m_newPositions.append(elm->pos());
    }
}

void CanvasMoveCommand::undo()
{
    const auto elems = elements();
    for (int i = 0; i < elems.size(); ++i) {
        elems.at(i)->setPos(m_oldPositions.at(i));
    }
}

void CanvasMoveCommand::redo()
{
    const auto elems = elements();
    for (int i = 0; i < elems.size(); ++i) {
        elems.at(i)->setPos(m_newPositions.at(i));
    }
}

CanvasRotateCommand::CanvasRotateCommand(const QList<GraphicElement *> &items, int angle, CanvasItem *canvas, QUndoCommand *parent)
    : CanvasElementsCommand(items, canvas, parent)
    , m_angle(angle)
{
    m_positions.reserve(items.size());
    for (auto *item : items) {
        m_positions.append(item->pos());
    }
}

void CanvasRotateCommand::undo()
{
    const auto elems = elements();
    for (int i = 0; i < elems.size(); ++i) {
        auto *elm = elems.at(i);
        elm->setRotation(elm->rotation() - m_angle);
        elm->setPos(m_positions.at(i));
        elm->update();
        elm->setSelected(true);
    }
}

void CanvasRotateCommand::redo()
{
    const auto elems = elements();
    double cx = 0;
    double cy = 0;
    int sz = 0;

    for (auto *elm : elems) {
        cx += elm->pos().x();
        cy += elm->pos().y();
        ++sz;
    }

    if (sz != 0) {
        cx /= sz;
        cy /= sz;
    }

    // Translate-rotate-translate-back maps each element position around the centroid.
    QTransform transform;
    transform.translate(cx, cy);
    transform.rotate(m_angle);
    transform.translate(-cx, -cy);

    for (auto *elm : elems) {
        elm->setPos(transform.map(elm->pos()));
        elm->setRotation(elm->rotation() + m_angle);
    }
}

CanvasFlipCommand::CanvasFlipCommand(const QList<GraphicElement *> &items, int axis, CanvasItem *canvas, QUndoCommand *parent)
    : CanvasElementsCommand(items, canvas, parent)
    , m_axis(axis)
{
    if (items.isEmpty()) {
        return;
    }

    m_positions.reserve(items.size());

    // Compute the bounding box of all selected elements so redo() can mirror each position
    // about the selection's own axis rather than the scene origin.
    double xmin = items.constFirst()->pos().x();
    double ymin = items.constFirst()->pos().y();
    double xmax = xmin;
    double ymax = ymin;

    for (auto *item : items) {
        m_positions.append(item->pos());
        xmin = (std::min)(xmin, item->pos().x());
        ymin = (std::min)(ymin, item->pos().y());
        xmax = (std::max)(xmax, item->pos().x());
        ymax = (std::max)(ymax, item->pos().y());
    }

    m_minPos = QPointF(xmin, ymin);
    m_maxPos = QPointF(xmax, ymax);
}

void CanvasFlipCommand::undo()
{
    // Flip is an involution: applying it twice returns to the original state.
    redo();
}

void CanvasFlipCommand::redo()
{
    for (auto *elm : elements()) {
        auto pos = elm->pos();

        // axis == 0: mirror across the vertical axis (flip horizontally)
        // axis == 1: mirror across the horizontal axis (flip vertically)
        (m_axis == 0) ? pos.setX(m_minPos.x() + (m_maxPos.x() - pos.x()))
                      : pos.setY(m_minPos.y() + (m_maxPos.y() - pos.y()));

        elm->setPos(pos);

        (m_axis == 0) ? elm->setFlippedX(!elm->isFlippedX())
                      : elm->setFlippedY(!elm->isFlippedY());
    }
}
