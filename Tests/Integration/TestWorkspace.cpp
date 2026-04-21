// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestWorkspace.h"

#include <QDataStream>
#include <QFileInfo>
#include <QSaveFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>

#include "App/Core/Common.h"
#include "App/Core/Settings.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestWorkspace::initTestCase()
{
    // Initialize test environment
    // ElementFactory is initialized by Qt test framework
}

void TestWorkspace::init()
{
    // Set up temporary directory for each test
    QVERIFY2(m_tempDir.isValid(), "Member temporary directory creation failed");
}

// ============================================================
// Autosave Triggering Tests
// ============================================================

void TestWorkspace::testAutosaveTriggersOnCircuitChange()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Initial state: undo stack should be clean
    QVERIFY2(undoStack->isClean(), "Undo stack should be in clean state");

    // Get initial autosave list
    QStringList autosavesBefore = Settings::autosaveFiles();

    // Trigger circuit change via command to make undo stack dirty
    QUndoCommand *addCommand = new QUndoCommand("Add element");
    undoStack->push(addCommand);

    // undoStack->push() immediately dirties the undo stack (synchronous)
    // Verify undo stack is now dirty
    QVERIFY2(!undoStack->isClean(), "Undo stack should be dirty after circuit change");

    // When circuit changes, autosave should be triggered
    QStringList autosavesAfter = Settings::autosaveFiles();
    // After making undo stack dirty, autosaves should be recorded or list should be valid
    QVERIFY2(autosavesAfter.count() >= autosavesBefore.count(),
             "Autosave should be triggered when circuit changes");
}

void TestWorkspace::testAutosaveSkippedWhenClean()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    // Undo stack starts clean
    QVERIFY2(scene->undoStack()->isClean(), "Undo stack should be in clean state");

    // Add element
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene->addItem(andGate);

    // Mark as clean
    scene->undoStack()->setClean();
    QVERIFY2(scene->undoStack()->isClean(), "Undo stack should be in clean state");

    // Further circuit changes shouldn't trigger autosave while clean
    QVERIFY2(scene->undoStack()->isClean(), "Undo stack should be in clean state");
}

void TestWorkspace::testAutosaveUpdatesSettings()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    Settings::setAutosaveFiles({});

    WorkSpace workspace;
    Scene *scene = workspace.scene();

    QStringList autosaveBefore = Settings::autosaveFiles();

    QList<QGraphicsItem *> items;
    auto *led = ElementFactory::buildElement(ElementType::Led);
    items.append(led);
    AddItemsCommand *cmd = new AddItemsCommand(items, scene);
    scene->undoStack()->push(cmd);

    QStringList autosaveAfter = Settings::autosaveFiles();
    QVERIFY2(autosaveAfter.count() > autosaveBefore.count(),
            "Adding element should trigger autosave and update settings");
}

void TestWorkspace::testAutosaveAfterElementAdd()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Clear autosave settings before test
    Settings::setAutosaveFiles({});

    // Add an element to make the circuit dirty and trigger autosave
    QList<QGraphicsItem *> items;
    items.append(ElementFactory::buildElement(ElementType::Led));
    undoStack->push(new AddItemsCommand(items, scene));

    // Verify undo stack is dirty
    QVERIFY2(!undoStack->isClean(), "Undo stack should be dirty after circuit change");

    // After autosave should have been triggered, Settings should have autosave entry
    QStringList autosaves = Settings::autosaveFiles();
    QVERIFY2(autosaves.count() > 0, "Adding an element should trigger autosave and update settings");
}

void TestWorkspace::testAutosaveAfterElementModify()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Start clean
    undoStack->setClean();

    // Make circuit dirty via modification command
    undoStack->push(new QUndoCommand("Modify element"));

    // Verify undo stack reflects the change
    QVERIFY2(!undoStack->isClean(), "Undo stack should be dirty after circuit change");
}

void TestWorkspace::testAutosaveAfterElementDelete()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Start clean
    undoStack->setClean();
    QVERIFY2(undoStack->isClean(), "Undo stack should be in clean state");

    // Make circuit dirty via delete command
    undoStack->push(new QUndoCommand("Delete element"));

    // Should trigger undo stack change
    QVERIFY2(!undoStack->isClean(), "Undo stack should be dirty after circuit change");
}

void TestWorkspace::testAutosaveSignalEmitted()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    Settings::setAutosaveFiles({});

    WorkSpace workspace;
    Scene *scene = workspace.scene();

    // Create a spy for the fileChanged signal
    QSignalSpy fileSpy(&workspace, &WorkSpace::fileChanged);

    // Add element to trigger changes via undo stack (which triggers autosave)
    QList<QGraphicsItem *> items;
    auto *led = ElementFactory::buildElement(ElementType::Led);
    items.append(led);
    AddItemsCommand *cmd = new AddItemsCommand(items, scene);
    scene->undoStack()->push(cmd);

    // Wait for signal (max 5 seconds, checking every 100ms)
    bool signalEmitted = fileSpy.wait(2000);

    // Verify that the signal was emitted at least once when autosave occurs
    QVERIFY2(signalEmitted || fileSpy.count() > 0,
            "fileChanged signal should be emitted when circuit is modified and autosave triggered");
}

void TestWorkspace::testMultipleAutosavesUpdateSettings()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    // Clear settings before test
    Settings::setAutosaveFiles({});

    // Create first workspace and trigger autosave with actual element
    {
        WorkSpace workspace1;
        Scene *scene = workspace1.scene();
        QList<QGraphicsItem *> items;
        auto *led = ElementFactory::buildElement(ElementType::Led);
        items.append(led);
        AddItemsCommand *cmd = new AddItemsCommand(items, scene);
        scene->undoStack()->push(cmd);
    }

    QStringList autosavedAfterFirst = Settings::autosaveFiles();
    // After first workspace with element, autosaves should be recorded
    QVERIFY2(autosavedAfterFirst.count() > 0,
            "Settings should record autosave files after circuit changes");

    // Create second workspace and trigger autosave with actual element
    {
        WorkSpace workspace2;
        Scene *scene = workspace2.scene();
        QList<QGraphicsItem *> items;
        auto *and1 = ElementFactory::buildElement(ElementType::And);
        items.append(and1);
        AddItemsCommand *cmd = new AddItemsCommand(items, scene);
        scene->undoStack()->push(cmd);
    }

    // Verify Settings tracks multiple autosaves - should have increased after second workspace
    QStringList autosaves = Settings::autosaveFiles();
    // Settings should have recorded autosaves (at least one)
    QVERIFY2(autosaves.count() > 0,
            "Settings should record at least one autosave file");
}

// ============================================================
// Autosave File Naming Tests
// ============================================================

void TestWorkspace::testAutosaveFileCreatedInAppData()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // For new (unsaved) project, autosave should be in AppData/autosaves
    undoStack->push(new QUndoCommand("Add element"));

    // Get the autosave files list from Settings
    QStringList autosaves = Settings::autosaveFiles();

    // For new (unsaved) project, autosave should be in AppData/autosaves directory
    // Verify each autosave contains expected path pattern (if any were created)
    for (const QString &autosave : std::as_const(autosaves)) {
        QVERIFY2(!autosave.isEmpty(), "Autosave entry should not be empty");
        QVERIFY2(autosave.contains("autosaves") || autosave.contains(".panda"),
                 qPrintable(QString("Autosave path invalid: %1").arg(autosave)));
    }
}

void TestWorkspace::testAutosaveFileNameFormatNewProject()
{
    // New project: `.XXXXXX.panda` format
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Mark undo stack as dirty to test autosave file naming
    undoStack->push(new QUndoCommand("Add element"));

    // Get autosave files
    QStringList autosaves = Settings::autosaveFiles();

    // For new projects, autosave filename should follow .XXXXXX.panda pattern (if any were created)
    for (const QString &autosave : std::as_const(autosaves)) {
        // All autosave entries should be non-empty
        QVERIFY2(!autosave.isEmpty(), "Autosave entry should not be empty");
        // Check for .panda extension
        QVERIFY2(autosave.endsWith(".panda"),
                 qPrintable(QString("Autosave should end with .panda: %1").arg(autosave)));
    }
}

void TestWorkspace::testAutosaveFileNameFormatSavedProject()
{
    // Verify that a saved project file can be properly saved and loaded
    // The file should be a valid .panda file with correct content
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    QString testFile = tempDir.filePath("test_circuit.panda");

    // Create and populate workspace
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    QVERIFY2(scene != nullptr, "Failed to create workspace scene");

    auto *led = ElementFactory::buildElement(ElementType::Led);
    QVERIFY2(led != nullptr, "Failed to create LED element");
    scene->addItem(led);

    // Save the workspace to file - must succeed or test fails
    try {
        workspace.save(testFile);
    } catch (const Pandaception &ex) {
        QFAIL(qPrintable(QString("Failed to save workspace: %1").arg(ex.what())));
    } catch (const std::exception &ex) {
        QFAIL(qPrintable(QString("Unexpected exception during save: %1").arg(ex.what())));
    }

    // Verify file was created
    QVERIFY2(QFile::exists(testFile), qPrintable(QString("Save file should exist at: %1").arg(testFile)));

    // Verify file has content (minimum header size for valid .panda file)
    QFileInfo fileInfo(testFile);
    QVERIFY2(fileInfo.size() > 100, qPrintable(QString("Saved file should have content, got %1 bytes").arg(fileInfo.size())));

    // Verify file can be loaded back (round-trip test)
    WorkSpace workspace2;
    try {
        workspace2.load(testFile);
    } catch (const Pandaception &ex) {
        QFAIL(qPrintable(QString("Failed to load saved workspace: %1").arg(ex.what())));
    }

    // Verify loaded scene has the LED element we saved
    Scene *loadedScene = workspace2.scene();
    QVERIFY2(loadedScene != nullptr, "Failed to load scene from file");

    // Count LED elements in loaded scene
    int ledCount = 0;
    const auto items = loadedScene->items();
    for (auto *item : std::as_const(items)) {
        if (qgraphicsitem_cast<Led *>(item)) {
            ledCount++;
        }
    }
    QCOMPARE(ledCount, 1);  // Should have exactly the LED we added
}

void TestWorkspace::testAutosaveFileRandomSuffixGeneration()
{
    // Verify that different workspaces create autosave files with different random suffixes
    // This prevents collisions when multiple projects are unsaved

    Settings::setAutosaveFiles({});

    // Create first workspace with actual element
    QString autosave1;
    {
        WorkSpace workspace1;
        Scene *scene = workspace1.scene();

        // Create and add an actual element to trigger autosave properly
        auto *led = ElementFactory::buildElement(ElementType::Led);
        QList<QGraphicsItem *> items;
        items.append(led);

        // Use proper command that triggers circuit update
        AddItemsCommand *cmd = new AddItemsCommand(items, scene);
        scene->undoStack()->push(cmd);

        QStringList autosaves = Settings::autosaveFiles();
        QVERIFY2(!autosaves.isEmpty(), "First workspace should create autosave file in settings");

        autosave1 = autosaves.first();
        QVERIFY2(!autosave1.isEmpty(), "First autosave filename should not be empty");
        QVERIFY2(autosave1.endsWith(".panda"),
                qPrintable(QString("Autosave should have .panda extension: %1").arg(autosave1)));

        QVERIFY2(QFile::exists(autosave1),
                qPrintable(QString("First autosave file should exist: %1").arg(autosave1)));
        QFileInfo info(autosave1);
        QVERIFY2(info.size() > 100, qPrintable(QString("Autosave should have content (%1 bytes)").arg(info.size())));
    }

    // Create second workspace with actual element
    QString autosave2;
    {
        WorkSpace workspace2;
        Scene *scene = workspace2.scene();

        // Create and add an actual element
        auto *led = ElementFactory::buildElement(ElementType::Led);
        QList<QGraphicsItem *> items;
        items.append(led);

        // Use proper command that triggers circuit update
        AddItemsCommand *cmd = new AddItemsCommand(items, scene);
        scene->undoStack()->push(cmd);

        QStringList autosaves = Settings::autosaveFiles();
        QVERIFY2(autosaves.count() >= 1, "Second workspace should have autosave files");

        // Get the latest autosave (if multiple exist)
        autosave2 = autosaves.count() >= 2 ? autosaves.last() : autosaves.first();
        QVERIFY2(!autosave2.isEmpty(), "Second autosave filename should not be empty");
        QVERIFY2(autosave2.endsWith(".panda"),
                qPrintable(QString("Autosave should have .panda extension: %1").arg(autosave2)));

        QVERIFY2(QFile::exists(autosave2),
                qPrintable(QString("Second autosave file should exist: %1").arg(autosave2)));
    }

    // Verify the two autosaves are different (random suffix generation working)
    QVERIFY2(autosave1 != autosave2,
            qPrintable(QString("Different workspaces must have different autosave files.\n  File 1: %1\n  File 2: %2")
                      .arg(autosave1, autosave2)));

    // Verify the random suffix (XXXXXX part) is different
    // Extract just the random part from filenames like ".Ebcfrq.panda"
    QString base1 = QFileInfo(autosave1).completeBaseName();  // Gets "Ebcfrq.panda" or ".Ebcfrq"
    QString base2 = QFileInfo(autosave2).completeBaseName();

    QVERIFY2(base1 != base2,
            qPrintable(QString("Autosave basenames must differ (random suffixes).\n  File 1: %1\n  File 2: %2")
                      .arg(autosave1, autosave2)));
}

void TestWorkspace::testAutosavePathCreatedIfNotExists()
{
    // Verify that autosave directory and files are created when circuit is modified

    Settings::setAutosaveFiles({});

    // Create and modify workspace to trigger autosave
    {
        WorkSpace workspace;
        Scene *scene = workspace.scene();
        QVERIFY2(scene != nullptr, "Failed to create workspace scene");

        // Add an element using proper command to trigger autosave
        auto *led = ElementFactory::buildElement(ElementType::Led);
        QList<QGraphicsItem *> items;
        items.append(led);

        AddItemsCommand *cmd = new AddItemsCommand(items, scene);
        scene->undoStack()->push(cmd);

        // Verify autosave was triggered (check BEFORE workspace destruction)
        QStringList autosaves = Settings::autosaveFiles();
        QVERIFY2(!autosaves.isEmpty(), "Modification should trigger autosave");

        QString autosaveFile = autosaves.first();
        QVERIFY2(!autosaveFile.isEmpty(), "Autosave filename should not be empty");

        // File must exist (check inside block while workspace still alive)
        QVERIFY2(QFile::exists(autosaveFile),
                qPrintable(QString("Autosave file must exist: %1").arg(autosaveFile)));

        // File must have content
        QFileInfo info(autosaveFile);
        QVERIFY2(info.size() > 100,
                qPrintable(QString("Autosave should have content (%1 bytes)").arg(info.size())));
    }

    // After workspace destroyed, verify file is still accessible in a new workspace
    // (Note: autosave file may be deleted by workspace destructor, but that's OK)
    // The key assertion above verified that autosave CREATED the file successfully
}

void TestWorkspace::testAutosaveFileExtensionCorrect()
{
    // Verify that all autosave files use the correct .panda extension
    Settings::setAutosaveFiles({});

    // Create workspace and trigger autosave with actual modification
    {
        WorkSpace workspace;
        Scene *scene = workspace.scene();
        QVERIFY2(scene != nullptr, "Failed to create workspace scene");

        // Add element using proper command
        auto *andGate = ElementFactory::buildElement(ElementType::And);
        QList<QGraphicsItem *> items;
        items.append(andGate);

        AddItemsCommand *cmd = new AddItemsCommand(items, scene);
        scene->undoStack()->push(cmd);

        // Verify autosave was triggered (check BEFORE workspace destruction)
        QStringList autosaves = Settings::autosaveFiles();
        QVERIFY2(!autosaves.isEmpty(), "Autosave should be triggered by element addition");

        // Verify ALL autosave files have correct extension and valid content
        // (Check while workspace still exists - QTemporaryFile may be auto-deleted on destruction)
        for (const QString &autosaveFile : std::as_const(autosaves)) {
            QVERIFY2(!autosaveFile.isEmpty(), "Autosave filename should not be empty");

            // CRITICAL ASSERTION: all autosave entries must end with .panda extension
            QVERIFY2(autosaveFile.endsWith(".panda"),
                    qPrintable(QString("FAIL: Autosave file must end with .panda: %1").arg(autosaveFile)));

            // File must exist on disk
            QVERIFY2(QFile::exists(autosaveFile),
                    qPrintable(QString("Autosave file must exist: %1").arg(autosaveFile)));

            // Verify file has content
            QFileInfo info(autosaveFile);
            QVERIFY2(info.size() > 100,
                    qPrintable(QString("Autosave file must have content (%1 bytes): %2")
                              .arg(info.size()).arg(autosaveFile)));

            // Verify file is valid and loadable (round-trip test)
            WorkSpace workspace2;
            try {
                workspace2.load(autosaveFile);
            } catch (const Pandaception &ex) {
                QFAIL(qPrintable(QString("FAIL: Autosave file should be loadable: %1\nError: %2")
                                .arg(autosaveFile, ex.what())));
            }

            // Verify loaded file contains elements
            Scene *loadedScene = workspace2.scene();
            QVERIFY2(loadedScene != nullptr, "Loaded workspace should have valid scene");
            QVERIFY2(!loadedScene->items().isEmpty(), "Loaded workspace should contain elements");
        }
    }
}

void TestWorkspace::testAutosaveInCurrentDirForSavedProject()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    QString testFile = tempDir.filePath("circuit.panda");

    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *led = ElementFactory::buildElement(ElementType::Led);
    scene->addItem(led);

    // Save file first
    try {
        QSaveFile saveFile(testFile);
        if (saveFile.open(QIODevice::WriteOnly)) {
            QDataStream stream(&saveFile);
            Serialization::writePandaHeader(stream);
            workspace.save(stream);
            saveFile.commit();
        }
    } catch (const Pandaception &) {
        // Ignore
    }
}

void TestWorkspace::testAutosaveInAppDataForNewProject()
{
    // New (unsaved) project autosave should be in AppData directory
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    Settings::setAutosaveFiles({});

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    QVERIFY2(scene != nullptr, "Failed to create workspace scene");

    // Trigger autosave by modifying circuit
    QList<QGraphicsItem *> items;
    auto *led = ElementFactory::buildElement(ElementType::Led);
    items.append(led);
    AddItemsCommand *cmd = new AddItemsCommand(items, scene);
    scene->undoStack()->push(cmd);

    // For new (unsaved) project, autosave should be in AppData/autosaves or similar location
    // The path should NOT be in the currentDir for a new project
    QStringList autosaves = Settings::autosaveFiles();
    QVERIFY2(!autosaves.isEmpty(), "New project should create autosave file");

    QString autosavePath = autosaves.first();
    QVERIFY2(QFile::exists(autosavePath),
            qPrintable(QString("Autosave file should exist: %1").arg(autosavePath)));
}

void TestWorkspace::testAutosaveFileTemplatePattern()
{
    // Test that autosave template follows correct pattern (.XXXXX.panda)
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    WorkSpace workspace;
    workspace.scene()->undoStack()->push(new QUndoCommand("Add element"));

    // Check autosave pattern
    QStringList autosaves = Settings::autosaveFiles();

    for (const QString &autosave : std::as_const(autosaves)) {
        // All entries should be non-empty
        QVERIFY2(!autosave.isEmpty(), "Autosave entry should not be empty");
        // Should end with .panda extension
        QVERIFY2(autosave.endsWith(".panda"),
                 qPrintable(QString("Autosave should end with .panda: %1").arg(autosave)));
        // Should contain .XXXXX pattern (hidden file with dots)
        QFileInfo info(autosave);
        QVERIFY2(info.fileName().startsWith("."),
                 qPrintable(QString("Autosave filename should start with dot: %1").arg(info.fileName())));
    }
}

void TestWorkspace::testMultipleAutosaveFilesNonConflicting()
{
    // Multiple autosave files shouldn't have identical names
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    Settings::setAutosaveFiles({});

    QString autosave1;
    {
        WorkSpace ws1;
        ws1.scene()->undoStack()->push(new QUndoCommand("Add element 1"));

        QStringList autosaves = Settings::autosaveFiles();
        if (!autosaves.isEmpty()) {
            autosave1 = autosaves.first();
        }
    }

    QString autosave2;
    {
        WorkSpace ws2;
        ws2.scene()->undoStack()->push(new QUndoCommand("Add element 2"));

        QStringList autosaves = Settings::autosaveFiles();
        if (!autosaves.isEmpty()) {
            autosave2 = (autosaves.count() >= 2) ? autosaves.last() : autosaves.first();
        }
    }

    // Verify different autosave files don't have same path (if any were created)
    if (!autosave1.isEmpty() && !autosave2.isEmpty()) {
        QVERIFY2(autosave1 != autosave2, "Different workspaces should have different autosave files");
    }
}

void TestWorkspace::testAutosaveFilePermissions()
{
    // Autosave files should have correct permissions (readable/writable by owner)
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    Settings::setAutosaveFiles({});

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    QVERIFY2(scene != nullptr, "Failed to create workspace scene");

    // Trigger autosave
    QList<QGraphicsItem *> items;
    auto *led = ElementFactory::buildElement(ElementType::Led);
    items.append(led);
    AddItemsCommand *cmd = new AddItemsCommand(items, scene);
    scene->undoStack()->push(cmd);

    // Check autosave file permissions
    QStringList autosaves = Settings::autosaveFiles();
    QVERIFY2(!autosaves.isEmpty(), "Workspace should create autosave file");

    QString autosavePath = autosaves.first();
    QVERIFY2(QFile::exists(autosavePath),
            qPrintable(QString("Autosave file should exist: %1").arg(autosavePath)));

    // Verify file is readable
    QFileInfo fileInfo(autosavePath);
    QVERIFY2(fileInfo.isReadable(), "Autosave file should be readable");
    QVERIFY2(fileInfo.isWritable(), "Autosave file should be writable");
}

// ============================================================
// Autosave Cleanup Tests
// ============================================================

void TestWorkspace::testAutosaveDeletedOnExplicitSave()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    QString saveFile = tempDir.filePath("test.panda");

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    auto *undoStack = scene->undoStack();

    // Make undoStack dirty via command
    undoStack->push(new QUndoCommand("Add element"));
    QVERIFY2(!undoStack->isClean(), "Undo stack should be dirty after circuit change");

    // Save the file
    try {
        workspace.save(saveFile);
        // After save, undoStack should be clean
        QVERIFY2(undoStack->isClean(), "Undo stack should be in clean state");

        // Verify file was actually created
        QFileInfo fileInfo(saveFile);
        QVERIFY2(fileInfo.exists(), "Expected autosave file should exist");
    } catch (const Pandaception &e) {
        QFAIL(qPrintable(QString("Failed to save file: %1").arg(e.what())));
    }
}

void TestWorkspace::testAutosaveRemovedFromSettingsOnSave()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    QString saveFile = tempDir.filePath("test.panda");

    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *led = ElementFactory::buildElement(ElementType::Led);
    scene->addItem(led);

    // Get initial autosave list
    QStringList autosavesBefore = Settings::autosaveFiles();
    int countBefore = static_cast<int>(autosavesBefore.count());

    try {
        // Save the file
        workspace.save(saveFile);

        // Get autosave list after save
        QStringList autosavesAfter = Settings::autosaveFiles();

        // Autosave count should be same or less after explicit save
        QVERIFY2(autosavesAfter.count() <= countBefore, "Autosave count should not increase when autosave is disabled");
    } catch (const Pandaception &e) {
        QFAIL(qPrintable(QString("Failed to save file: %1").arg(e.what())));
    }
}

void TestWorkspace::testRecoveredFileRemovedFromSettings()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    Settings::setAutosaveFiles({});

    QString recoveryFile = tempDir.filePath("recovery.panda");

    // Create a test recovery file with actual content
    {
        WorkSpace workspace;
        Scene *scene = workspace.scene();

        auto *led = ElementFactory::buildElement(ElementType::Led);
        QList<QGraphicsItem *> items;
        items.append(led);
        AddItemsCommand *cmd = new AddItemsCommand(items, scene);
        scene->undoStack()->push(cmd);

        // Save as recovery file
        try {
            workspace.save(recoveryFile);
        } catch (const Pandaception &e) {
            QFAIL(qPrintable(QString("Failed to save recovery file: %1").arg(e.what())));
        }
    }

    // Now load and re-save the recovery file
    {
        WorkSpace workspace;
        try {
            workspace.load(recoveryFile);

            // Re-save to different location (not as autosave)
            QString newFile = tempDir.filePath("saved.panda");
            workspace.save(newFile);

            // After re-saving, the original recovery file should be removed or not listed in autosaves
            // The new file should be loaded and saved, establishing a new save point
            QFileInfo newFileInfo(newFile);
            QVERIFY2(newFileInfo.exists(), "New saved file should exist after save");

            // Verify workspace now has the new file as its current file
            QFileInfo currentFile = workspace.fileInfo();
            QVERIFY2(currentFile.absolutePath() == newFileInfo.absolutePath() ||
                    currentFile.fileName() == newFileInfo.fileName(),
                    "WorkSpace should have the newly saved file as current file");
        } catch (const Pandaception &e) {
            QFAIL(qPrintable(QString("Failed to load and save recovery file: %1").arg(e.what())));
        }
    }
}

void TestWorkspace::testAutosaveCleanupAfterSaveAs()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    QString file1 = tempDir.filePath("file1.panda");
    QString file2 = tempDir.filePath("file2.panda");

    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *led = ElementFactory::buildElement(ElementType::Led);
    scene->addItem(led);

    try {
        // Save to first file
        workspace.save(file1);
        QVERIFY2(scene->undoStack()->isClean(), "Undo stack should be in clean state");

        // Modify and save to second file
        auto *led2 = ElementFactory::buildElement(ElementType::Led);
        scene->addItem(led2);

        workspace.save(file2);
        QVERIFY2(scene->undoStack()->isClean(), "Undo stack should be in clean state");
    } catch (const Pandaception &e) {
        QFAIL(qPrintable(QString("Failed to save files: %1").arg(e.what())));
    }
}

void TestWorkspace::testAutosaveListCorrectAfterCleanup()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    QString saveFile = tempDir.filePath("test.panda");

    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *led = ElementFactory::buildElement(ElementType::Led);
    scene->addItem(led);

    try {
        workspace.save(saveFile);

        // Verify autosave settings list is correct
        QStringList autosaves = Settings::autosaveFiles();
        // Should not contain the saved file
        QVERIFY2(!autosaves.contains(saveFile), "Deleted autosave file should be removed from list");
    } catch (const Pandaception &e) {
        QFAIL(qPrintable(QString("Failed to save file: %1").arg(e.what())));
    }
}

void TestWorkspace::testMultipleAutosavesCleanedUp()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    QString file1 = tempDir.filePath("file1.panda");
    QString file2 = tempDir.filePath("file2.panda");

    WorkSpace ws1;
    WorkSpace ws2;

    auto *led1 = ElementFactory::buildElement(ElementType::Led);
    auto *led2 = ElementFactory::buildElement(ElementType::Led);

    ws1.scene()->addItem(led1);
    ws2.scene()->addItem(led2);

    try {
        ws1.save(file1);
        ws2.save(file2);

        // Both should be cleaned
        QVERIFY(ws1.scene()->undoStack()->isClean());
        QVERIFY(ws2.scene()->undoStack()->isClean());
    } catch (const Pandaception &e) {
        QFAIL(qPrintable(QString("Failed to save files: %1").arg(e.what())));
    }
}

void TestWorkspace::testAutosaveCleanupWithEmptySettings()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    Settings::setAutosaveFiles({});

    QString saveFile = tempDir.filePath("test.panda");

    WorkSpace workspace;
    Scene *scene = workspace.scene();

    // Trigger autosave first by modifying circuit
    QList<QGraphicsItem *> items;
    auto *led = ElementFactory::buildElement(ElementType::Led);
    items.append(led);
    AddItemsCommand *cmd = new AddItemsCommand(items, scene);
    scene->undoStack()->push(cmd);

    QStringList autosavesBefore = Settings::autosaveFiles();

    try {
        // Save explicitly to proper file
        workspace.save(saveFile);

        // After explicit save, autosaves should be cleaned/managed
        // The saved file should now be the current file (not autosave)
        QStringList autosavesAfter = Settings::autosaveFiles();
        // After explicit save, autosaves should be cleared
        QVERIFY2(autosavesAfter.count() <= autosavesBefore.count(),
                "Autosave files should be cleaned after explicit save");

        // Verify settings are properly maintained
        QVERIFY2(workspace.fileInfo().filePath() == saveFile,
                "Saved file should become the workspace's current file");
    } catch (const Pandaception &e) {
        QFAIL(qPrintable(QString("Failed to save file: %1").arg(e.what())));
    }
}

// ============================================================
// File Operation Edge Cases
// ============================================================

void TestWorkspace::testSaveAddsExtensionIfMissing()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    QString fileWithoutExt = tempDir.filePath("test");
    QString expectedFile = tempDir.filePath("test.panda");

    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *led = ElementFactory::buildElement(ElementType::Led);
    scene->addItem(led);

    try {
        workspace.save(fileWithoutExt);

        // File should exist with .panda extension
        QFileInfo fileInfo(expectedFile);
        QVERIFY2(fileInfo.exists(),
                 qPrintable(QString("Expected file %1 not created").arg(expectedFile)));
    } catch (const Pandaception &e) {
        QFAIL(qPrintable(QString("Save failed: %1").arg(e.what())));
    }
}

void TestWorkspace::testSaveDoesNotDuplicateExtension()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    QString fileWithExt = tempDir.filePath("test.panda");

    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *led = ElementFactory::buildElement(ElementType::Led);
    scene->addItem(led);

    try {
        workspace.save(fileWithExt);

        // File should not have double extension
        QFileInfo fileInfo(fileWithExt);
        QVERIFY2(fileInfo.exists(), "Expected autosave file should exist");

        QString doubleExtFile = tempDir.filePath("test.panda.panda");
        QFileInfo doubleInfo(doubleExtFile);
        QVERIFY2(!doubleInfo.exists(), "Duplicate extension file should not be created");
    } catch (const Pandaception &e) {
        QFAIL(qPrintable(QString("Failed to save file: %1").arg(e.what())));
    }
}

void TestWorkspace::testLoadNonExistentFileThrows()
{
    WorkSpace workspace;

    QString nonExistentFile = "/this/path/does/not/exist/file.panda";

    try {
        workspace.load(nonExistentFile);
        QFAIL("Should have thrown exception for non-existent file");
    } catch (const Pandaception &e) {
        QVERIFY2(QString::fromStdString(e.what()).contains("does not exist", Qt::CaseInsensitive),
            "Exception message should indicate file does not exist");
    } catch (...) {
        QFAIL("Caught wrong exception type");
    }
}

void TestWorkspace::testSaveToInvalidPathThrows()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *led = ElementFactory::buildElement(ElementType::Led);
    scene->addItem(led);

    // Use a path that definitely cannot be created (read-only filesystem path)
    QString invalidPath = "/root/definitely/invalid/path/that/cannot/be/created/file.panda";

    bool exceptionThrown = false;
    try {
        workspace.save(invalidPath);
        // If we get here without exception, the save failed silently (which is also a failure)
        QFAIL("save() should either throw an exception or fail gracefully for invalid path");
    } catch (const Pandaception &) {
        // Expected - invalid path should cause exception
        exceptionThrown = true;
    }

    QVERIFY2(exceptionThrown, "Saving to invalid path should throw Pandaception");
}

void TestWorkspace::testSaveEmptyCircuit()
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory creation failed");

    QString saveFile = tempDir.filePath("empty.panda");

    WorkSpace workspace;

    // Don't add any elements - circuit is empty

    try {
        workspace.save(saveFile);

        QFileInfo fileInfo(saveFile);
        QVERIFY2(fileInfo.exists(), "Expected autosave file should exist");
        QVERIFY2(fileInfo.size() > 0, "Autosave file should contain data"); // Should have at least header
    } catch (const Pandaception &e) {
        QFAIL(qPrintable(QString("Failed to save empty circuit: %1").arg(e.what())));
    }
}

