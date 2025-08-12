// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestScene : public QObject
{
    Q_OBJECT

private slots:
    void testSceneConstruction();
    void testElementsManagement();
    void testSelectionOperations();
    void testUndoRedoOperations();
    void testSceneGeometry();
    void testConnections();
    void testThemeUpdate();
    void testSceneUtilities();
    void testSceneActions();
    void testSceneTransformations();
    void testSceneKeyboardNavigation();
    void testSceneDisplayOptions();
    void testSceneItemQueries();
    void testSceneSignals();
    void testSceneMimeData();
    void testSceneView();
    void testSceneEventFiltering();
};