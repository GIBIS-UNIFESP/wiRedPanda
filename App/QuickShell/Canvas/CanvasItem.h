// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Phase 1 canvas prototype: batched QSGNode rendering + spatial-index hit-testing,
 * driven by the real Simulation engine.
 */

#pragma once

#include <memory>

#include <QQmlEngine>
#include <QQuickItem>
#include <QTimer>

#include "App/QuickShell/Canvas/SpatialIndex.h"

class Connection;
class GraphicElement;
class Simulation;
class SimulationHost;

/**
 * \class CanvasItem
 * \brief Qt Quick rewrite Phase 1 canvas prototype.
 *
 * \details Proves the plan's riskiest bet in one place: a handful of *real* GraphicElement
 * subclasses (InputSwitch/And/Led — no synthetic stand-ins) are constructed and wired
 * together with real Connection objects, exactly as production code does, but without ever
 * being added to a QGraphicsScene — GraphicElement/Port/Connection's geometry (pos(),
 * boundingRect(), scenePos()) all work from the QGraphicsItem parent-child chain alone, no
 * scene membership required. A real Simulation instance (bound through the narrow
 * SimulationHost interface, same seam Scene itself uses) drives them on its own 1ms timer.
 * Each simulation-driven state change is picked up by this item's own refresh timer and
 * rendered as two batched QSGGeometryNodes (one for gate bodies, one for wires) — colored
 * flat quads/lines for now, not real per-element SVG appearance (that's Phase 2's job; see
 * the Phase 0 audit finding on ElementAppearance/ElementOrientation in the plan's Context
 * section). Hit-testing goes through SpatialIndex, replacing what QGraphicsScene::itemAt()
 * would have done — clicking a switch toggles it through the same setOn(bool, int) API
 * production code uses, and the resulting Status change flows through the real Simulation
 * to the real Led, visibly proving the whole chain end to end.
 */
class CanvasItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit CanvasItem(QQuickItem *parent = nullptr);
    ~CanvasItem() override;

protected:
    /// \reimp Builds the two batched geometry nodes (gates, wires) from current element state.
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;
    /// \reimp Hit-tests via m_index; toggles the clicked element if it's an input.
    void mousePressEvent(QMouseEvent *event) override;
    /// \reimp Updates the hovered element for the next paint's highlight.
    void hoverMoveEvent(QHoverEvent *event) override;
    /// \reimp Clears hover highlight when the pointer leaves the canvas entirely.
    void hoverLeaveEvent(QHoverEvent *event) override;

private:
    void buildDemoCircuit();
    void rebuildSpatialIndex();
    void toggleIfInput(GraphicElement *element);

    std::unique_ptr<SimulationHost> m_host;
    std::unique_ptr<Simulation> m_simulation;
    QVector<GraphicElement *> m_elements; // owned; never added to a QGraphicsScene
    QVector<Connection *> m_connections;  // owned; never added to a QGraphicsScene
    QHash<quint64, GraphicElement *> m_elementsById;
    SpatialIndex m_index;
    quint64 m_hoveredId = 0;

    /// Real Simulation state changes on its own 1ms QTimer, independent of this item's
    /// render loop; this timer periodically calls QQuickItem::update() so the batched nodes
    /// stay in sync with live simulation output. Matches Simulation's own documented ~60fps
    /// visual-throttle cadence (see Simulation.h's m_visualTickInterval).
    QTimer m_refreshTimer;
};
