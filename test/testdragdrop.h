// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestDragDrop : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Basic tests
    void testBasicMouseEvents();
    void testMimeDataCreation();
    void testDropEventSimulation();
    
    // Drag simulation tests
    void testQTestMouseDragSimulation();
    void testProgrammaticDragDrop();
    void testCloneDragSimulation();
    void testConnectionCreation();
};