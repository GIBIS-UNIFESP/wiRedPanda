// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief WireRouter: A* pathfinder with rip-up & reroute for orthogonal wires.
 */

#pragma once

#include <QPoint>
#include <QPointF>
#include <QVector>

class Scene;

/**
 * \class WireRouter
 * \brief Computes non-overlapping orthogonal routes for all connections in a scene.
 *
 * Uses weighted A* (with bend penalty) for individual routes and rip-up & reroute
 * to resolve conflicts between wires, following the approach used by professional
 * EDA tools.
 */
class WireRouter
{
public:
    struct RouteResult {
        int connectionId;
        QVector<QPointF> waypoints;
    };

    explicit WireRouter(Scene *scene);

    /// Routes all orthogonal connections and returns new waypoints for each.
    QVector<RouteResult> routeAll();

private:
    // --- Grid management ---

    void buildGrid();
    void markRect(const QRectF &rect, bool blocked);
    void markSegment(const QPoint &a, const QPoint &b, bool blocked);
    void markPath(const QVector<QPoint> &path, bool blocked);

    [[nodiscard]] bool inBounds(int x, int y) const;
    [[nodiscard]] bool isBlocked(int x, int y) const;

    // --- Coordinate conversion ---

    [[nodiscard]] QPoint sceneToGrid(const QPointF &scenePos) const;
    [[nodiscard]] QPointF gridToScene(const QPoint &gridPos) const;

    // --- Pathfinding ---

    [[nodiscard]] QVector<QPoint> findPath(const QPoint &start, const QPoint &end) const;
    [[nodiscard]] static QVector<QPointF> gridPathToWaypoints(const QVector<QPoint> &gridPath, int cellSize);

    // --- Rip-up & reroute ---

    void ripUpAndReroute(QVector<RouteResult> &results, QVector<QVector<QPoint>> &gridPaths);

    // --- Members ---

    Scene *m_scene;
    QVector<QVector<bool>> m_grid;
    QPointF m_gridOrigin;
    int m_gridWidth = 0;
    int m_gridHeight = 0;

    static constexpr int CellSize = 8;
    static constexpr int BendPenalty = 3;
    static constexpr int MaxRipUpIterations = 10;
    static constexpr int GridPadding = 8;
};
