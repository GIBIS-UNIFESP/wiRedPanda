// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class WorkSpace;

class TestWorkspace : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Basic workspace functionality
    void testWorkspaceConstruction();
    void testWorkspaceDestruction();
    void testWorkspaceSceneAccess();
    void testWorkspaceUndoStack();
    
    // File operations
    void testNewWorkspace();
    void testSaveWorkspace();
    void testLoadWorkspace();
    void testSaveAsWorkspace();
    void testRecentFiles();
    
    // Project management
    void testProjectCreation();
    void testProjectOpening();
    void testProjectSaving();
    void testProjectClosing();
    void testMultipleProjects();
    
    // Undo/Redo integration
    void testUndoRedoIntegration();
    void testCommandHistory();
    void testUndoRedoStates();
    void testUndoRedoLimits();
    
    // Scene management
    void testSceneCreation();
    void testSceneClearing();
    void testSceneSelection();
    void testSceneModification();
    
    // Error handling
    void testFileErrors();
    void testCorruptedFiles();
    void testPermissionErrors();
    void testRecovery();
    
private:
    WorkSpace* createTestWorkspace();
    void validateWorkspaceState(WorkSpace* workspace);
};