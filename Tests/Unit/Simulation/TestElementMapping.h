// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QObject>
#include <QTest>
#include <QVector>

#include "App/Scene/Scene.h"

class ElementMapping;
class GraphicElement;

class TestElementMapping : public QObject {
    Q_OBJECT

public:
    TestElementMapping() = default;

private slots:
    // Test infrastructure

    // Element Validation Tests (4 tests)
    void testSimpleElementMapping();
    void testDisconnectedElementHandling();
    void testDefaultVCCGNDConnections();
    void testICInputOutputMapping();

    // IC Integration Tests (3 tests)
    void testGenerateMapWithIC();
    void testApplyConnectionsWithIC();
    void testICElementSorting();

    // Element Sorting Tests (2 tests)
    void testSortLogicElementsByPriority();
    void testMixedElementTypesSorting();

    // Connection Application (1 test)
    void testApplyConnectionsDefaultValues();

private:
    // Helper functions
    std::unique_ptr<Scene> createSimpleCircuit();
    std::unique_ptr<Scene> createDisconnectedCircuit();
    std::unique_ptr<Scene> createCircuitWithVCCGND();

    // Verification helpers
    void verifyMappingCreated(const QVector<GraphicElement *> &elements);
};
