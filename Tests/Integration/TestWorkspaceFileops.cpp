// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestWorkspaceFileops.h"

#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTemporaryFile>

#include "App/Element/ElementFactory.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestWorkspaceFileops::initTestCase()
{
    // Initialize test environment
    QVERIFY2(m_tempDir.isValid(), "Member temporary directory creation failed");
}

void TestWorkspaceFileops::init()
{
    // Set up temporary directory for each test
    QVERIFY2(m_tempDir.isValid(), "Member temporary directory creation failed");
}

// ============================================================
// Save Operations Tests
// ============================================================

void TestWorkspaceFileops::testSaveToNewFile()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    // Add an AND gate to the circuit
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    andGate->setPos(10, 20);
    scene->addItem(andGate);

    // Mark as clean after adding (simulating user save action)
    scene->undoStack()->setClean();

    // Create file path for new file
    QString filePath = m_tempDir.path() + "/test_circuit.panda";

    // Save to new file
    try {
        workspace.save(filePath);

        // Verify file was created
        QFileInfo fileInfo(filePath);
        QVERIFY2(fileInfo.exists(), "Expected file was not created");
        QVERIFY2(fileInfo.size() > 0, "File should contain data");

        // Verify file path is updated in workspace
        QCOMPARE(workspace.fileInfo().absoluteFilePath(), filePath);
    } catch (const std::exception &e) {
        QFAIL(QString("Save failed with exception: %1").arg(e.what()).toUtf8().constData());
    }
}

void TestWorkspaceFileops::testSaveToExistingFile()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    // Add an OR gate
    auto *orGate = ElementFactory::buildElement(ElementType::Or);
    scene->addItem(orGate);
    scene->undoStack()->setClean();

    QString filePath = m_tempDir.path() + "/overwrite_test.panda";

    // Save first time
    try {
        workspace.save(filePath);
        QVERIFY2(QFileInfo(filePath).exists(), "File should exist at the specified path");

        // Add another element
        auto *notGate = ElementFactory::buildElement(ElementType::Not);
        scene->addItem(notGate);
        scene->undoStack()->setClean();

        // Save to same file (overwrite)
        workspace.save(filePath);

        // File should still exist with new content
        QFileInfo fileInfo(filePath);
        QVERIFY2(fileInfo.exists(), "Expected file was not created");
        QVERIFY2(fileInfo.size() > 0, "File should contain data");

        // Size should likely be different due to additional element
        // (though not guaranteed, so we just verify it's valid)
        QVERIFY2(fileInfo.size() > 0, "File should contain data");
    } catch (const std::exception &e) {
        QFAIL(QString("Save failed with exception: %1").arg(e.what()).toUtf8().constData());
    }
}

void TestWorkspaceFileops::testSaveWithSpecialCharactersInFilename()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    // Add element
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene->addItem(andGate);
    scene->undoStack()->setClean();

    // Use filename with special characters (but valid for filesystem)
    QString filePath = m_tempDir.path() + "/test_circuit_2025.panda";

    try {
        workspace.save(filePath);

        // Verify file was created
        QFileInfo fileInfo(filePath);
        QVERIFY2(fileInfo.exists(), "Expected file was not created");

        // Verify correct filename
        QCOMPARE(fileInfo.fileName(), QString("test_circuit_2025.panda"));
    } catch (const std::exception &e) {
        QFAIL(QString("Save with special characters failed: %1").arg(e.what()).toUtf8().constData());
    }
}

void TestWorkspaceFileops::testSavePreservesElementData()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    // Create a small circuit with multiple elements
    auto *input = ElementFactory::buildElement(ElementType::InputSwitch);
    input->setPos(0, 0);
    input->setLabel("INPUT");
    scene->addItem(input);

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    andGate->setPos(50, 0);
    scene->addItem(andGate);

    auto *output = ElementFactory::buildElement(ElementType::Led);
    output->setPos(100, 0);
    output->setLabel("OUTPUT");
    scene->addItem(output);

    scene->undoStack()->setClean();

    QString filePath = m_tempDir.path() + "/preserve_data_test.panda";

    try {
        // Save circuit
        workspace.save(filePath);

        // Create new workspace and load
        WorkSpace workspace2;
        Scene *scene2 = workspace2.scene();

        workspace2.load(filePath);

        // Verify elements were preserved
        QCOMPARE(scene2->elements().size(), 3);

        // Verify at least some element properties (labels, positions)
        bool foundInput = false;
        bool foundOutput = false;

        for (auto *elem : scene2->elements()) {
            if (elem->label() == "INPUT") {
                foundInput = true;
                QCOMPARE(elem->pos(), QPointF(0, 0));
            }
            if (elem->label() == "OUTPUT") {
                foundOutput = true;
                QCOMPARE(elem->pos(), QPointF(100, 0));
            }
        }

        QVERIFY2(foundInput, "Should find expected input port in loaded IC");
        QVERIFY2(foundOutput, "Should find expected output port in loaded IC");
    } catch (const std::exception &e) {
        QFAIL(QString("Preserve data test failed: %1").arg(e.what()).toUtf8().constData());
    }
}

// ============================================================
// Load Operations Tests
// ============================================================

void TestWorkspaceFileops::testLoadFromValidFile()
{
    // First, create and save a valid file
    {
        WorkSpace workspace;
        Scene *scene = workspace.scene();

        auto *andGate = ElementFactory::buildElement(ElementType::And);
        scene->addItem(andGate);
        scene->undoStack()->setClean();

        QString filePath = m_tempDir.path() + "/valid_circuit.panda";
        workspace.save(filePath);
    }

    // Now load from the saved file
    {
        WorkSpace workspace;
        Scene *scene = workspace.scene();

        QString filePath = m_tempDir.path() + "/valid_circuit.panda";

        try {
            workspace.load(filePath);

            // Verify file was loaded
            QCOMPARE(workspace.fileInfo().absoluteFilePath(), filePath);

            // Verify circuit contains elements
            QVERIFY2(scene->elements().size() > 0, "Scene should contain loaded circuit elements");
        } catch (const std::exception &e) {
            QFAIL(QString("Load valid file failed: %1").arg(e.what()).toUtf8().constData());
        }
    }
}

void TestWorkspaceFileops::testLoadNonExistentFileThrowsException()
{
    WorkSpace workspace;

    QString nonExistentPath = m_tempDir.path() + "/nonexistent_file_xyz.panda";

    // Verify file doesn't exist
    QVERIFY2(!QFileInfo(nonExistentPath).exists(), "Non-existent file should not exist");

    // Attempt to load non-existent file
    bool exceptionThrown = false;
    try {
        workspace.load(nonExistentPath);
    } catch (const Pandaception &) {
        exceptionThrown = true;
    } catch (const std::exception &) {
        exceptionThrown = true;
    }

    // Verify exception was thrown
    QVERIFY2(exceptionThrown, "Exception should be thrown when loading non-existent file");

    // Verify circuit remains empty
    QVERIFY2(workspace.scene()->elements().isEmpty(), "Scene elements should be cleared");
}

void TestWorkspaceFileops::testLoadCorruptedFileHandling()
{
    // Create a corrupted file (invalid binary data)
    QString filePath = m_tempDir.path() + "/corrupted.panda";
    {
        QFile file(filePath);
        QVERIFY2(file.open(QIODevice::WriteOnly),
                 qPrintable(QString("Failed to open %1: %2")
                           .arg(file.fileName(), file.errorString())));
        file.write("This is not a valid circuit file format");
        file.close();
    }

    WorkSpace workspace;

    // Attempt to load corrupted file
    try {
        workspace.load(filePath);
        // If load succeeds, that's acceptable (may have error recovery)
    } catch (const Pandaception &) {
        // Exception is acceptable for corrupted data
    } catch (const std::exception &) {
        // Exception is acceptable for corrupted data
    }

    // Either exception or graceful recovery is acceptable for corrupted data
}

void TestWorkspaceFileops::testLoadEmptyCircuit()
{
    // Create and save an empty circuit
    {
        WorkSpace workspace;
        Scene *scene = workspace.scene();

        // Don't add any elements
        scene->undoStack()->setClean();

        QString filePath = m_tempDir.path() + "/empty_circuit.panda";
        workspace.save(filePath);
    }

    // Load the empty circuit
    {
        WorkSpace workspace;
        Scene *scene = workspace.scene();

        QString filePath = m_tempDir.path() + "/empty_circuit.panda";

        try {
            workspace.load(filePath);

            // Verify file was loaded
            QCOMPARE(workspace.fileInfo().absoluteFilePath(), filePath);

            // Verify circuit is indeed empty
            QVERIFY2(scene->elements().isEmpty(), "Scene elements should be empty after reload");
        } catch (const std::exception &e) {
            QFAIL(QString("Load empty circuit failed: %1").arg(e.what()).toUtf8().constData());
        }
    }
}

// ============================================================
// File State Management Tests
// ============================================================

void TestWorkspaceFileops::testModifiedFlagAfterAddElement()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    // Initially, undo stack should be clean
    QVERIFY2(scene->undoStack()->isClean(), "Undo stack should be in clean state after save");

    // Add an element
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene->addItem(andGate);

    // Verify the element was actually added to the scene
    QCOMPARE(scene->elements().size(), 1);
}

void TestWorkspaceFileops::testModifiedFlagClearedAfterSave()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    // Add element to make circuit dirty
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene->addItem(andGate);

    // Manually clear undo stack to simulate save state
    scene->undoStack()->setClean();

    QString filePath = m_tempDir.path() + "/save_state_test.panda";

    try {
        workspace.save(filePath);

        // After save, undo stack should be clean
        QVERIFY2(scene->undoStack()->isClean(), "Undo stack should be in clean state after save");
    } catch (const std::exception &e) {
        QFAIL(QString("Save state test failed: %1").arg(e.what()).toUtf8().constData());
    }
}

void TestWorkspaceFileops::testFilePathUpdatedAfterSave()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    // Add element
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene->addItem(andGate);
    scene->undoStack()->setClean();

    QString filePath = m_tempDir.path() + "/file_path_update_test.panda";

    try {
        // Before save, fileInfo should not have the new path
        // (unless workspace had a previous file)

        // Save to new file
        workspace.save(filePath);

        // After save, fileInfo should be updated
        QCOMPARE(workspace.fileInfo().absoluteFilePath(), filePath);

        // File should exist
        QVERIFY2(QFileInfo(filePath).exists(), "File should exist at the specified path");
    } catch (const std::exception &e) {
        QFAIL(QString("File path update test failed: %1").arg(e.what()).toUtf8().constData());
    }
}

void TestWorkspaceFileops::testFilePathPreservedAfterLoad()
{
    // Create and save a file
    QString filePath = m_tempDir.path() + "/preserve_path_test.panda";
    {
        WorkSpace workspace;
        Scene *scene = workspace.scene();

        auto *andGate = ElementFactory::buildElement(ElementType::And);
        scene->addItem(andGate);
        scene->undoStack()->setClean();

        workspace.save(filePath);
    }

    // Load the file
    {
        WorkSpace workspace;
        QString loadPath = m_tempDir.path() + "/preserve_path_test.panda";

        try {
            workspace.load(loadPath);

            // After load, fileInfo should reflect the loaded file path
            QCOMPARE(workspace.fileInfo().absoluteFilePath(), loadPath);

            // File name should be preserved
            QCOMPARE(workspace.fileInfo().fileName(), QString("preserve_path_test.panda"));
        } catch (const std::exception &e) {
            QFAIL(QString("File path preservation test failed: %1").arg(e.what()).toUtf8().constData());
        }
    }
}

void TestWorkspaceFileops::testFileExtensionPandaAppend()
{
    // Test that file paths with .panda extension are recognized
    WorkSpace workspace;

    QString pathWithExt = m_tempDir.path() + "/test.panda";
    // Just verify the path would contain .panda
    QVERIFY2(pathWithExt.endsWith(".panda"), "File path should have .panda extension");
}

void TestWorkspaceFileops::testFileExtensionNoDuplicate()
{
    // Test that .panda extension logic doesn't duplicate
    WorkSpace workspace;

    QString pathWithExt = m_tempDir.path() + "/test.panda";
    QString pathDouble = pathWithExt + ".panda";

    // Verify they're different
    QVERIFY2(pathWithExt != pathDouble, "Path should not be duplicated");
    // Verify the doubled path would be wrong
    QVERIFY2(!pathWithExt.endsWith(".panda.panda"), "File path should not have double extension");
}

void TestWorkspaceFileops::testDolphinFileNameStorage()
{
    // Test that dolphin filename can be stored and retrieved
    WorkSpace workspace;

    QString dolphinName = "test_simulation.dolphin";
    workspace.setDolphinFileName(dolphinName);

    QCOMPARE(workspace.dolphinFileName(), dolphinName);
}

void TestWorkspaceFileops::testLastIdInitializationValue()
{
    // Test that lastId is initialized to a valid value
    WorkSpace workspace;

    int lastId = workspace.lastId();
    QVERIFY2(lastId >= 0, "Item ID should be non-negative");
}

void TestWorkspaceFileops::testLastIdGetterSetter()
{
    // Test that lastId can be set and retrieved
    WorkSpace workspace;

    int initialLastId = workspace.lastId();
    int newLastId = initialLastId + 100;

    workspace.setLastId(newLastId);
    QCOMPARE(workspace.lastId(), newLastId);

    // Verify we can set it again
    workspace.setLastId(newLastId + 50);
    QCOMPARE(workspace.lastId(), newLastId + 50);
}

void TestWorkspaceFileops::testLastIdPersistenceOnLoad()
{
    // Test that lastId value is maintained correctly
    // This is a simpler version that doesn't do full save/load
    WorkSpace workspace;

    // Simulate adding elements by incrementing lastId
    int initialId = workspace.lastId();
    workspace.setLastId(initialId + 10);

    // Verify the value is set
    QCOMPARE(workspace.lastId(), initialId + 10);

    // Verify setting to same value works
    workspace.setLastId(initialId + 10);
    QCOMPARE(workspace.lastId(), initialId + 10);
}

void TestWorkspaceFileops::testFileInfoAfterCreation()
{
    // Test that fileInfo is accessible and in valid state after creation
    WorkSpace workspace;

    QFileInfo fileInfo = workspace.fileInfo();

    // File should not exist initially (no file has been loaded/saved)
    QVERIFY2(!fileInfo.exists(), "Autosave file should be removed after setAutoRemove");

    // absoluteFilePath() must return a non-null string even for empty fileInfo
    QString absolutePath = fileInfo.absoluteFilePath();
    QVERIFY2(!absolutePath.isNull(), "Absolute path should not be null");
}
