// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/SceneInteraction.h"

#include <utility>

#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainterPath>
#include <QPen>

#include "App/Core/SentryHelpers.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Commands.h"
#include "App/Scene/ConnectionManager.h"
#include "App/Scene/Scene.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

SceneInteraction::SceneInteraction(Scene *scene)
    : m_scene(scene)
{
}

void SceneInteraction::attachToScene()
{
    // The rubber-band selection rectangle must not itself be selectable or it would
    // appear in selectedItems() and interfere with element operations
    m_selectionRect.setFlag(QGraphicsItem::ItemIsSelectable, false);
    m_scene->addItem(&m_selectionRect);

    // Used to throttle expensive operations during drag (e.g., ensureVisible)
    m_timer.start();
}

void SceneInteraction::applyTheme(const ThemeAttributes &theme)
{
    m_selectionRect.setBrush(QBrush(theme.m_selectionBrush));
    m_selectionRect.setPen(QPen(theme.m_selectionPen, 1, Qt::SolidLine));
}

void SceneInteraction::startSelectionRect()
{
    m_selectionStartPoint = m_mousePos;
    m_markingSelectionBox = true;
    m_selectionRect.setRect(QRectF(m_selectionStartPoint, m_selectionStartPoint));
    m_selectionRect.show();
    m_selectionRect.update();
}

bool SceneInteraction::mousePress(QGraphicsSceneMouseEvent *event)
{
    m_mousePos = event->scenePos();
    m_scene->connectionManager()->updateHover(m_mousePos);

    auto *item = m_scene->itemAt(m_mousePos);

    if (item) {
        // --- Element selection and drag preparation ---
        if (event->button() == Qt::LeftButton) {
            if (event->modifiers().testFlag(Qt::ControlModifier)) {
                // Ctrl+click toggles individual item membership in the selection
                item->setSelected(!item->isSelected());
            }

            auto selectedElements_ = m_scene->selectedElements();

            // Include the clicked element even if it isn't yet selected, so a
            // single-click drag of an unselected element works immediately
            if (auto *element = qgraphicsitem_cast<GraphicElement *>(item)) {
                selectedElements_ << element;
            }

            m_draggingElement = ((item->type() == GraphicElement::Type) && !selectedElements_.isEmpty());
            if (m_draggingElement) {
                sentryBreadcrumb("ui", QStringLiteral("Drag started: %1 element(s)").arg(selectedElements_.size()));
            }

            // Snapshot positions now; MoveCommand compares these against release-time positions.
            // QPointer in m_dragSnapshot lets entries auto-clear if the element is destroyed
            // before release (e.g. Delete shortcut while mid-drag — see WIREDPANDA-H9).
            m_dragSnapshot.clear();
            m_dragSnapshot.reserve(selectedElements_.size());

            for (auto *element : std::as_const(selectedElements_)) {
                m_dragSnapshot.append({element, element->pos()});
            }
        }

        // --- Wire connection handling ---
        if (item->type() == Port::Type) {
            /* When the mouse is pressed over an connected input port, the line
             * is disconnected and can be connected to another port. */
            if (m_scene->connectionManager()->hasEditedConnection()) {
                // An in-progress wire exists; try to complete it at this port
                m_scene->connectionManager()->tryComplete(m_mousePos);
                return true;
            }

            auto *pressedPort = qgraphicsitem_cast<Port *>(item);

            if (auto *startPort = dynamic_cast<OutputPort *>(pressedPort)) {
                m_scene->connectionManager()->startFromOutput(startPort);
                return true;
            }

            if (auto *endPort = dynamic_cast<InputPort *>(pressedPort)) {
                // Empty input port: begin a new wire; occupied port: detach the existing wire
                endPort->connections().isEmpty() ? m_scene->connectionManager()->startFromInput(endPort) : m_scene->connectionManager()->detach(endPort);
                return true;
            }
        }
    }

    // Clicking on empty space while a wire is being drawn cancels it
    m_scene->connectionManager()->cancel();

    if (!item && (event->button() == Qt::LeftButton)) {
        startSelectionRect();
    }

    if (event->button() == Qt::RightButton) {
        m_scene->contextMenu(event->screenPos());
        return true;
    }

    return false;
}

bool SceneInteraction::mouseMove(QGraphicsSceneMouseEvent *event)
{
    m_mousePos = event->scenePos();
    m_scene->connectionManager()->updateHover(m_mousePos);

    // Expand scene rect while dragging so elements can be moved beyond the current boundary,
    // and auto-scroll the viewport to follow the cursor.
    if (m_draggingElement) {
        m_scene->resizeScene();

        if (m_timer.elapsed() > 50) {
            const auto viewList = m_scene->views();
            if (!viewList.isEmpty()) {
                viewList.first()->ensureVisible(m_mousePos.x(), m_mousePos.y(), 1, 1, 50, 50);
            }
            m_timer.restart();
        }
    }

    // --- In-progress wire routing ---
    if (m_scene->connectionManager()->hasEditedConnection()) {
        m_scene->connectionManager()->updateEditedEnd(m_mousePos);
        return true;
    }

    // --- Rubber-band selection rectangle ---
    if (m_markingSelectionBox) {
        const QRectF rect = QRectF(m_selectionStartPoint, m_mousePos).normalized();
        m_selectionRect.setRect(rect);
        QPainterPath selectionBox;
        selectionBox.addRect(rect);
        m_scene->setSelectionArea(selectionBox);
    }

    return false;
}

bool SceneInteraction::mouseRelease(QGraphicsSceneMouseEvent *event)
{
    if (m_draggingElement && (event->button() == Qt::LeftButton)) {
        bool moved = false;

        if (!m_dragSnapshot.empty()) {
            // Filter out elements destroyed mid-drag (their QPointers are now null).
            // Without this, dereferencing a freed element below crashes the app.
            QList<GraphicElement *> liveElements;
            QList<QPointF> liveOldPositions;
            liveElements.reserve(m_dragSnapshot.size());
            liveOldPositions.reserve(m_dragSnapshot.size());

            for (const auto &[ptr, oldPos] : std::as_const(m_dragSnapshot)) {
                if (ptr) {
                    liveElements.append(ptr.data());
                    liveOldPositions.append(oldPos);
                }
            }

            // Only push a MoveCommand if at least one surviving element actually changed
            // position; avoids polluting the undo stack with no-op moves (click without drag).
            for (int i = 0; i < liveElements.size(); ++i) {
                if (liveElements.at(i)->pos() != liveOldPositions.at(i)) {
                    moved = true;
                    break;
                }
            }

            if (moved) {
                m_scene->receiveCommand(new MoveCommand(liveElements, liveOldPositions, m_scene));
            }
        }

        sentryBreadcrumb("ui", moved ? QStringLiteral("Drag ended: moved") : QStringLiteral("Drag ended: no move"));
        m_draggingElement = false;
        m_dragSnapshot.clear();

        // Only tighten scene rect after an actual drag; a click-without-move
        // should not trigger a rect change that could shift the viewport.
        if (moved) {
            m_scene->resizeScene();
        }
    }

    m_selectionRect.hide();
    m_markingSelectionBox = false;

    // Complete an in-progress wire on mouse release (when no button is held)
    // — this handles the drag-to-connect gesture (press output → drag → release on input)
    if (m_scene->connectionManager()->hasEditedConnection() && (event->buttons() == Qt::NoButton)) {
        m_scene->connectionManager()->tryComplete(m_mousePos);
        return true;
    }

    return false;
}

bool SceneInteraction::mouseDoubleClick(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        return true;
    }

    // Double-click on a fully connected wire inserts a routing node at the click point,
    // splitting the wire into two segments; guard ensures it's not a dangling wire
    if (auto *connection = qgraphicsitem_cast<Connection *>(m_scene->itemAt(m_mousePos)); connection && connection->startPort() && connection->endPort()) {
        m_scene->receiveCommand(new SplitCommand(connection, m_mousePos, m_scene));
    }

    return false;
}
