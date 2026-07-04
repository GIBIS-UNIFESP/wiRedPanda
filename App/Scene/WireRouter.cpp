// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/WireRouter.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

WireRouter::WireRouter(Scene *scene)
    : m_scene(scene)
{
}

void WireRouter::buildGrid()
{
    QRectF bounds;
    for (auto *elm : m_scene->elements()) {
        bounds = bounds.united(elm->sceneBoundingRect());
    }
    for (auto *conn : m_scene->connections()) {
        if (conn->startPort()) {
            bounds = bounds.united(QRectF(conn->startPort()->scenePos(), QSizeF(1, 1)));
        }
        if (conn->endPort()) {
            bounds = bounds.united(QRectF(conn->endPort()->scenePos(), QSizeF(1, 1)));
        }
    }

    bounds.adjust(-GridPadding * CellSize, -GridPadding * CellSize,
                   GridPadding * CellSize,  GridPadding * CellSize);

    m_gridOrigin = bounds.topLeft();
    m_gridWidth = static_cast<int>(std::ceil(bounds.width() / CellSize)) + 1;
    m_gridHeight = static_cast<int>(std::ceil(bounds.height() / CellSize)) + 1;

    m_grid.assign(m_gridHeight, QVector<bool>(m_gridWidth, false));

    for (auto *elm : m_scene->elements()) {
        markRect(elm->sceneBoundingRect(), true);
    }
}

void WireRouter::markRect(const QRectF &rect, bool blocked)
{
    const QPoint topLeft = sceneToGrid(rect.topLeft());
    const QPoint bottomRight = sceneToGrid(rect.bottomRight());

    for (int y = topLeft.y(); y <= bottomRight.y(); ++y) {
        for (int x = topLeft.x(); x <= bottomRight.x(); ++x) {
            if (inBounds(x, y)) {
                m_grid[y][x] = blocked;
            }
        }
    }
}

void WireRouter::markSegment(const QPoint &a, const QPoint &b, bool blocked)
{
    if (a.x() == b.x()) {
        const int x = a.x();
        const int yMin = (std::min)(a.y(), b.y());
        const int yMax = (std::max)(a.y(), b.y());
        for (int y = yMin; y <= yMax; ++y) {
            if (inBounds(x, y)) {
                m_grid[y][x] = blocked;
            }
        }
    } else if (a.y() == b.y()) {
        const int y = a.y();
        const int xMin = (std::min)(a.x(), b.x());
        const int xMax = (std::max)(a.x(), b.x());
        for (int x = xMin; x <= xMax; ++x) {
            if (inBounds(x, y)) {
                m_grid[y][x] = blocked;
            }
        }
    }
}

void WireRouter::markPath(const QVector<QPoint> &path, bool blocked)
{
    for (int i = 0; i + 1 < path.size(); ++i) {
        markSegment(path[i], path[i + 1], blocked);
    }
}

bool WireRouter::inBounds(int x, int y) const
{
    return x >= 0 && x < m_gridWidth && y >= 0 && y < m_gridHeight;
}

bool WireRouter::isBlocked(int x, int y) const
{
    if (!inBounds(x, y)) {
        return true;
    }
    return m_grid[y][x];
}

QPoint WireRouter::sceneToGrid(const QPointF &scenePos) const
{
    return {
        static_cast<int>(std::round((scenePos.x() - m_gridOrigin.x()) / CellSize)),
        static_cast<int>(std::round((scenePos.y() - m_gridOrigin.y()) / CellSize))
    };
}

QPointF WireRouter::gridToScene(const QPoint &gridPos) const
{
    return {
        m_gridOrigin.x() + gridPos.x() * CellSize,
        m_gridOrigin.y() + gridPos.y() * CellSize
    };
}

QVector<QPoint> WireRouter::findPath(const QPoint &start, const QPoint &end) const
{
    if (start == end) {
        return {start};
    }

    struct PathNode {
        int x, y;
        int dir;
        int gCost;
        int fCost;

        bool operator>(const PathNode &other) const { return fCost > other.fCost; }
    };

    auto heuristic = [&](int x, int y) {
        return std::abs(x - end.x()) + std::abs(y - end.y());
    };

    auto packKey = [&](int x, int y, int dir) -> qint64 {
        return (static_cast<qint64>(y) * m_gridWidth + x) * 5 + dir;
    };

    static constexpr int dx[] = {1, -1, 0, 0};
    static constexpr int dy[] = {0, 0, 1, -1};

    std::priority_queue<PathNode, std::vector<PathNode>, std::greater<PathNode>> openSet;
    std::unordered_map<qint64, qint64> cameFrom;
    std::unordered_set<qint64> visited;

    const int h0 = heuristic(start.x(), start.y());
    const qint64 startKey = packKey(start.x(), start.y(), 0);
    openSet.push({start.x(), start.y(), 0, 0, h0});
    cameFrom[startKey] = -1;

    while (!openSet.empty()) {
        const PathNode current = openSet.top();
        openSet.pop();

        if (current.x == end.x() && current.y == end.y()) {
            QVector<QPoint> path;
            qint64 key = packKey(current.x, current.y, current.dir);
            while (key != -1) {
                const int idx = static_cast<int>((key / 5));
                const int py = idx / m_gridWidth;
                const int px = idx % m_gridWidth;
                path.prepend(QPoint(px, py));
                key = cameFrom[key];
            }
            return path;
        }

        const qint64 currentKey = packKey(current.x, current.y, current.dir);
        if (visited.contains(currentKey)) {
            continue;
        }
        visited.insert(currentKey);

        for (int i = 0; i < 4; ++i) {
            const int nx = current.x + dx[i];
            const int ny = current.y + dy[i];
            const int ndir = i + 1;

            if (nx == end.x() && ny == end.y()) {
                // always allow reaching the goal
            } else if (isBlocked(nx, ny)) {
                continue;
            }

            const qint64 nKey = packKey(nx, ny, ndir);
            if (visited.contains(nKey)) {
                continue;
            }

            int stepCost = 1;
            if (current.dir != 0 && current.dir != ndir) {
                stepCost += BendPenalty;
            }

            const int gCost = current.gCost + stepCost;
            const int fCost = gCost + heuristic(nx, ny);

            if (!cameFrom.contains(nKey) || gCost < (fCost - heuristic(nx, ny))) {
                cameFrom[nKey] = currentKey;
                openSet.push({nx, ny, ndir, gCost, fCost});
            }
        }
    }

    return {};
}

QVector<QPointF> WireRouter::gridPathToWaypoints(const QVector<QPoint> &gridPath, int cellSize)
{
    if (gridPath.size() < 2) {
        return {};
    }

    QVector<QPointF> waypoints;
    int prevDx = gridPath[1].x() - gridPath[0].x();
    int prevDy = gridPath[1].y() - gridPath[0].y();

    for (int i = 2; i < gridPath.size(); ++i) {
        const int curDx = gridPath[i].x() - gridPath[i - 1].x();
        const int curDy = gridPath[i].y() - gridPath[i - 1].y();

        if (curDx != prevDx || curDy != prevDy) {
            waypoints.append(QPointF(gridPath[i - 1].x() * cellSize,
                                     gridPath[i - 1].y() * cellSize));
        }

        prevDx = curDx;
        prevDy = curDy;
    }

    return waypoints;
}

void WireRouter::ripUpAndReroute(QVector<RouteResult> &results, QVector<QVector<QPoint>> &gridPaths)
{
    for (int iteration = 0; iteration < MaxRipUpIterations; ++iteration) {
        std::unordered_map<qint64, QVector<int>> segmentUsers;

        auto packSeg = [](int x1, int y1, int x2, int y2) -> qint64 {
            if (x1 > x2 || (x1 == x2 && y1 > y2)) {
                std::swap(x1, x2);
                std::swap(y1, y2);
            }
            return (static_cast<qint64>(x1) << 48) | (static_cast<qint64>(y1) << 32)
                 | (static_cast<qint64>(x2) << 16) | static_cast<qint64>(y2);
        };

        for (int i = 0; i < gridPaths.size(); ++i) {
            const auto &path = gridPaths[i];
            for (int j = 0; j + 1 < path.size(); ++j) {
                const QPoint &a = path[j];
                const QPoint &b = path[j + 1];
                if (a.x() == b.x()) {
                    for (int y = (std::min)(a.y(), b.y()); y < (std::max)(a.y(), b.y()); ++y) {
                        segmentUsers[packSeg(a.x(), y, a.x(), y + 1)].append(i);
                    }
                } else {
                    for (int x = (std::min)(a.x(), b.x()); x < (std::max)(a.x(), b.x()); ++x) {
                        segmentUsers[packSeg(x, a.y(), x + 1, a.y())].append(i);
                    }
                }
            }
        }

        std::unordered_set<int> conflicting;
        for (const auto &[seg, users] : segmentUsers) {
            if (users.size() > 1) {
                int longest = users[0];
                for (int idx : users) {
                    if (gridPaths[idx].size() > gridPaths[longest].size()) {
                        longest = idx;
                    }
                }
                conflicting.insert(longest);
            }
        }

        if (conflicting.empty()) {
            break;
        }

        for (int i = 0; i < gridPaths.size(); ++i) {
            if (!conflicting.contains(i)) {
                markPath(gridPaths[i], true);
            }
        }

        for (int idx : conflicting) {
            const auto &result = results[idx];
            auto *conn = dynamic_cast<Connection *>(m_scene->itemById(result.connectionId));
            if (!conn || !conn->startPort() || !conn->endPort()) {
                continue;
            }

            const QPoint startGrid = sceneToGrid(conn->startPort()->scenePos());
            const QPoint endGrid = sceneToGrid(conn->endPort()->scenePos());
            QVector<QPoint> newPath = findPath(startGrid, endGrid);

            if (!newPath.isEmpty()) {
                gridPaths[idx] = newPath;
                results[idx].waypoints = gridPathToWaypoints(newPath, CellSize);
                for (auto &wp : results[idx].waypoints) {
                    wp = QPointF(wp.x() + m_gridOrigin.x(), wp.y() + m_gridOrigin.y());
                }
            }

            markPath(gridPaths[idx], true);
        }

        for (int i = 0; i < gridPaths.size(); ++i) {
            markPath(gridPaths[i], false);
        }
    }
}

QVector<WireRouter::RouteResult> WireRouter::routeAll()
{
    buildGrid();

    struct ConnInfo {
        Connection *conn;
        int manhattanDist;
    };

    QVector<ConnInfo> conns;
    for (auto *conn : m_scene->connections()) {
        if (!conn->startPort() || !conn->endPort()) {
            continue;
        }
        const QPointF s = conn->startPort()->scenePos();
        const QPointF e = conn->endPort()->scenePos();
        const int dist = static_cast<int>(std::abs(s.x() - e.x()) + std::abs(s.y() - e.y()));
        conns.append({conn, dist});
    }

    std::sort(conns.begin(), conns.end(), [](const ConnInfo &a, const ConnInfo &b) {
        return a.manhattanDist < b.manhattanDist;
    });

    constexpr int corridorLen = 4;
    for (const auto &ci : conns) {
        auto clearCorridor = [&](Port *port) {
            const QPoint pg = sceneToGrid(port->scenePos());

            static constexpr int dirs[][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            for (const auto &dir : dirs) {
                for (int i = 0; i <= corridorLen; ++i) {
                    const int cx = pg.x() + dir[0] * i;
                    const int cy = pg.y() + dir[1] * i;
                    if (inBounds(cx, cy)) {
                        m_grid[cy][cx] = false;
                    }
                }
            }
        };
        clearCorridor(ci.conn->startPort());
        clearCorridor(ci.conn->endPort());
    }

    QVector<RouteResult> results;
    QVector<QVector<QPoint>> gridPaths;
    results.reserve(conns.size());
    gridPaths.reserve(conns.size());

    for (const auto &ci : conns) {
        const QPoint startGrid = sceneToGrid(ci.conn->startPort()->scenePos());
        const QPoint endGrid = sceneToGrid(ci.conn->endPort()->scenePos());

        QVector<QPoint> path = findPath(startGrid, endGrid);

        RouteResult result;
        result.connectionId = ci.conn->id();

        if (path.isEmpty()) {
            const QPointF startScene = ci.conn->startPort()->scenePos();
            const QPointF endScene = ci.conn->endPort()->scenePos();
            QPointF corner(endScene.x(), startScene.y());
            if (corner != startScene && corner != endScene) {
                result.waypoints = {corner};
            }
        } else {
            result.waypoints = gridPathToWaypoints(path, CellSize);
            for (auto &wp : result.waypoints) {
                wp = QPointF(wp.x() + m_gridOrigin.x(), wp.y() + m_gridOrigin.y());
            }
        }

        results.append(result);
        gridPaths.append(path);
    }

    ripUpAndReroute(results, gridPaths);

    return results;
}
