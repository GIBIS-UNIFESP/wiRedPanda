// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestElementMapping : public QObject
{
    Q_OBJECT

private slots:
    void testConstructorAndBasicMapping();
    void testInputOutputMapping();
    void testVccGndMapping();
    void testConnectionMapping();
    void testDefaultValueConnections();
    void testComplexCircuitMapping();
    void testSortLogicElements();
    void testValidateElements();
    void testSequentialElementMapping();
    void testDestructorCleanup();
    void testMultipleInputConnections();
    void testEdgeCaseElements();
    void testICElementMapping();
};