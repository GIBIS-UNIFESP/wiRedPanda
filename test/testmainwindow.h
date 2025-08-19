// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryFile>

class QAction;

class MainWindow;
class WorkSpace;

class TestMainWindow : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    // Basic functionality tests
    void testMainWindowCreation();
    void testMainWindowWithFileName();
    void testMainWindowDestruction();

    // File operations tests
    void testCreateNewTab();
    void testSaveWithoutFileName(); // Fixed to avoid modal dialog
    void testSaveWithFileName();
    void testLoadPandaFile();
    void testSetCurrentFile();
    void testCurrentFileInfo();
    void testCurrentDir();

    // Tab management tests
    void testMultipleTabs();
    void testCloseTab();
    void testSwitchBetweenTabs();
    void testCloseFiles();
    void testCloseTabAnyway();

    // Export functionality tests (fixed to avoid modal dialogs)
    void testExportToArduino(); // Fixed to avoid file dialog
    void testExportToWaveFormFile(); // Fixed to avoid file dialog
    void testExportToWaveFormTerminal(); // Fixed to avoid file dialog
    void testExportToImage(); // Fixed to avoid file dialog
    void testExportToPdf(); // Fixed to avoid file dialog

    // Menu and action tests
    void testMenuActions();
    void testFileMenuActions();
    void testEditMenuActions();
    void testViewMenuActions();
    void testHelpMenuActions();

    // Theme and appearance tests
    void testDarkTheme();
    void testLightTheme();
    void testThemeSwitching();

    // Language and localization tests
    void testLanguageDetection();
    void testLanguageSwitching();
    void testRetranslation();
    void testAvailableLanguages();
    void testLanguageDisplayNames();
    void testLanguageFlagIcons();

    // Simulation mode tests
    void testFastMode();
    void testBackgroundSimulation();
    void testSimulationToggle();

    // Recent files tests
    void testRecentFileActions();
    void testAddRecentFile();
    void testRecentFileLimit();

    // Auto-save and recovery tests (fixed to avoid modal dialogs)
    void testAutosaveFiles();
    void testAutosaveRecovery();
    void testConfirmSave(); // Fixed to avoid save confirmation dialog

    // Window state and events tests
    void testWindowShow();
    void testWindowClose();
    void testCloseEvent();
    void testEventHandling();

    // Workspace integration tests
    void testCurrentTab();
    void testWorkspaceOperations();
    void testMultipleWorkspaces();

    // Error handling tests
    void testInvalidFileLoading();
    void testFilePermissionErrors();
    void testCorruptFileHandling();
    void testUnsupportedFileFormats();

    // UI interaction tests
    void testKeyboardShortcuts();
    void testToolbarInteractions();
    void testStatusBarUpdates();
    void testProgressIndicators();

    // Dolphin (waveform) integration tests
    void testDolphinFileName();
    void testSetDolphinFileName();
    void testDolphinFileOperations();

    // Element manipulation tests
    void testFlipHorizontally();
    void testFlipVertically();
    void testElementRotation();
    void testElementDeletion();

    // Complex workflow tests
    void testCreateSaveLoadWorkflow();
    void testMultiTabWorkflow();
    void testExportWorkflow();
    void testRecoveryWorkflow();

    // Circuit simulation logic tests
    void testCircuitSimulationLogic();

    // Menu action discovery and testing
    void testMenuActionDiscovery();

private:
    // Helper methods for creating test scenarios
    MainWindow* createTestMainWindow(const QString& fileName = QString());
    WorkSpace* getCurrentWorkspace();
    QString createTestPandaFile();
    void simulateFileModification();
    void simulateUserAction(const QString& actionName);

    // File system helpers
    QString createTempDirectory();
    QString createTestCircuitFile();
    void cleanupTestFiles();

    // UI interaction helpers
    void simulateMenuAction(const QString& menuName, const QString& actionName);
    void simulateKeyboardShortcut(const QString& shortcut);
    void simulateTabSwitch(int tabIndex);

    // Verification helpers
    void verifyFileOperationSuccess(const QString& operation);
    void verifyWindowState(const QString& expectedState);
    void verifyTabCount(int expectedCount);
    void verifyCurrentFile(const QString& expectedFileName);
    void verifyThemeApplication(const QString& themeName);
    void verifyLanguageApplication(const QString& languageCode);

    // Menu action helper
    QAction* findMenuAction(const QString& menuName, const QString& actionName);

    // Test data and utilities
    MainWindow* m_mainWindow = nullptr;
    QTemporaryFile* m_tempFile = nullptr;
    QStringList m_tempFiles;
    QString m_tempDir;
};
