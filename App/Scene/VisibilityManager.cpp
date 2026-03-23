// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/VisibilityManager.h"

#include "App/Element/GraphicElement.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Scene.h"

VisibilityManager::VisibilityManager(Scene *scene)
    : m_scene(scene)
{
}

void VisibilityManager::showGates(const bool visible)
{
    m_showGates = visible;
    const auto items = m_scene->items();

    for (auto *item : items) {
        if (item->type() == GraphicElement::Type) {
            auto *element = qgraphicsitem_cast<GraphicElement *>(item);
            if (!element) {
                continue;
            }
            const auto group = element->elementGroup();

            // Only hide/show internal logic gates; Input, Output and Other elements
            // (e.g., labels, ICs) are always kept visible regardless of this toggle
            if ((group != ElementGroup::Input) && (group != ElementGroup::Output) && (group != ElementGroup::Other)) {
                item->setVisible(visible);
            }
        }
    }
}

void VisibilityManager::showWires(const bool visible)
{
    m_showWires = visible;
    const auto items = m_scene->items();

    for (auto *item : items) {
        if (item->type() == QNEConnection::Type) {
            item->setVisible(visible);
            continue;
        }

        if (item->type() == GraphicElement::Type) {
            auto *element = qgraphicsitem_cast<GraphicElement *>(item);

            // Node elements are purely wire-routing helpers with no logical function;
            // hiding wires should hide nodes too since they're meaningless without wires
            if (element->elementType() == ElementType::Node) {
                element->setVisible(visible);
            } else {
                // For other elements, hide only their port handles (the connectable dots),
                // not the element body itself, so the gate symbols remain visible
                for (auto *inputPort : element->inputs()) {
                    inputPort->setVisible(visible);
                }

                for (auto *outputPort : element->outputs()) {
                    outputPort->setVisible(visible);
                }
            }
        }
    }
}

void VisibilityManager::reapply()
{
    showWires(m_showWires);
    showGates(m_showGates);
}

