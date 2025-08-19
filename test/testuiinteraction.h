// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class Scene;
class Simulation;
class WorkSpace;
class GraphicsView;

/*!
 * @class TestUIInteraction
 * @brief Tests UI interactions including mouse clicks and drag operations to build circuits
 *
 * This test class verifies that circuits can be built through UI interactions,
 * simulating user clicks, drags, and element placement operations.
 */
class TestUIInteraction : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    // UI interaction tests
    void testBuildSimpleCircuitWithClicks();
    void testDragAndDropElements();
    void testConnectionCreation();

private:
    // Helper functions
    void setupWorkspace();
    void simulateMouseClick(const QPointF &pos);
    void simulateMouseDrag(const QPointF &start, const QPointF &end);
    void verifyCircuitBuilt();
    void addElementToScene(const QString &elementType, const QPointF &position);
    void connectElements(const QPointF &outputPos, const QPointF &inputPos);
    int countConnections() const;

    WorkSpace *m_workspace{nullptr};
    Scene *m_scene{nullptr};
    Simulation *m_simulation{nullptr};
    GraphicsView *m_view{nullptr};
};
