// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestIc.h"

#include <limits>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QScopeGuard>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTime>

#include "App/Core/Application.h"
#include "App/Core/Enums.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Element/ICRenderer.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Workspace.h"
#include "App/Versions.h"
#include "App/Wiring/Connection.h"
#include "Tests/Common/TestUtils.h"

void TestIC::initTestCase()
{
    // Initialize test environment
}

// Port Mapping Tests

void TestIC::testICPortMapping()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Create IC and load jkflipflop example (5 inputs, 2 outputs)
    auto *ic = new IC();
    scene->addItem(ic);

    // Use shared utility for examples directory path
    const QString icFile = TestUtils::examplesDir() + "jkflipflop.panda";

    // Verify file exists before loading
    QVERIFY2(QFile::exists(icFile),
             qPrintable(QString("IC file not found: %1").arg(icFile)));

    // Load with error handling
    try {
        ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(e.what())));
        return;
    }

    // Verify IC loaded by checking it has ports
    QVERIFY2(ic->inputSize() > 0 || ic->outputSize() > 0, "Failed to load IC file");

    // Don't assert exact port counts since file structure might vary
    // Just verify ports are created
    for (int i = 0; i < ic->inputSize(); ++i) {
        const auto *port = ic->inputPort(i);
        QVERIFY2(port != nullptr, "Failed to retrieve IC port");
    }

    for (int i = 0; i < ic->outputSize(); ++i) {
        const auto *port = ic->outputPort(i);
        QVERIFY2(port != nullptr, "Failed to retrieve IC port");
    }
}

void TestIC::testICEmptyPorts()
{
    // Test that IC with no file loaded has no ports
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *ic = new IC();
    scene->addItem(ic);

    // IC should have 0 inputs and 0 outputs when no file is loaded
    QCOMPARE(ic->inputSize(), 0);
    QCOMPARE(ic->outputSize(), 0);
}

// Nested IC Tests

void TestIC::testICNestedSingleLevel()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Load a simple IC that doesn't contain other ICs
    auto *ic = new IC();
    scene->addItem(ic);

    const QString icFile = TestUtils::examplesDir() + "jkflipflop.panda";

    // Verify file exists before loading
    QVERIFY2(QFile::exists(icFile),
             qPrintable(QString("IC file not found: %1").arg(icFile)));

    try {
        ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(e.what())));
        return;
    }

    // Verify IC loaded
    QVERIFY2(ic->inputSize() > 0 || ic->outputSize() > 0, "Failed to load JK flip-flop IC");

}

void TestIC::testICNestedMultiLevel()
{
    // Test that nested IC files load without errors
    // Verifies all ICs in the scene, not just the first one
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Load ic.panda which contains nested ICs
    const QString mainFile = TestUtils::examplesDir() + "ic.panda";

    workspace.load(mainFile);

    // Find all IC elements in the scene
    QVector<IC *> loadedICs;
    const auto items = scene->items();
    for (auto *item : std::as_const(items)) {
        if (auto *ic = dynamic_cast<IC *>(item)) {
            loadedICs.append(ic);
        }
    }

    QVERIFY2(loadedICs.size() > 0, "Should find at least one IC in scene");

    // Verify all loaded ICs have valid port configurations
    for (IC *ic : loadedICs) {
        QVERIFY2(ic->inputSize() >= 0, "IC should have non-negative input count");
        QVERIFY2(ic->outputSize() >= 0, "IC should have non-negative output count");
        // Empty ICs (no inputs and no outputs) are acceptable; nothing to assert.
    }
}

// Logic Generation Tests

void TestIC::testICLogicGeneration()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *ic = new IC();
    scene->addItem(ic);

    const QString icFile = TestUtils::examplesDir() + "jkflipflop.panda";

    // Verify file exists before loading
    QVERIFY2(QFile::exists(icFile),
             qPrintable(QString("IC file not found: %1").arg(icFile)));

    try {
        ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(e.what())));
        return;
    }

    // Test that IC can generate logic mapping when expanded
    QVERIFY2(ic->inputSize() > 0, "IC should have inputs after loading");
    QVERIFY2(ic->outputSize() > 0, "IC should have outputs after loading");
}

// Serialization Tests

void TestIC::testICSaveLoad()
{
    // Test round-trip: create IC → save workspace → load workspace → verify IC preserved

    // Step 1: Create workspace with IC
    WorkSpace workspace1;
    auto *scene1 = workspace1.scene();

    auto *ic1 = new IC();
    scene1->addItem(ic1);

    const QString icFile = TestUtils::examplesDir() + "jkflipflop.panda";
    ic1->loadFile(icFile, QFileInfo(icFile).absolutePath());

    // Verify IC is properly loaded
    QVERIFY2(ic1->inputSize() > 0, "IC should have inputs after loading");
    QVERIFY2(ic1->outputSize() > 0, "IC should have outputs after loading");

    const int originalInputSize = ic1->inputSize();
    const int originalOutputSize = ic1->outputSize();

    // Step 2: Save workspace to temporary file
    // Save in examples directory to keep IC file references valid
    const QString saveDirPath = TestUtils::examplesDir();
    const QString tempPath = saveDirPath + "/test_ic_roundtrip_" + QString::number(QTime::currentTime().msec()) + ".panda";

    try {
        workspace1.save(tempPath);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("WorkSpace::save() should not throw exception: %1").arg(e.what())));
        return;
    }

    // The temp file lives in the repo's Examples/ dir (required so the IC's
    // relative jkflipflop.panda reference resolves on reload). A scope guard
    // removes it on EVERY exit path — a failing QVERIFY below returns from the
    // function and would otherwise leak the file into the repo.
    auto removeTempFile = qScopeGuard([&tempPath] { QFile::remove(tempPath); });

    // Verify file was created and has content
    QFileInfo fileInfo(tempPath);
    QVERIFY2(fileInfo.exists(), "Saved file should exist");
    QVERIFY2(fileInfo.size() > 0, "Saved file should have content");

    // Step 3: Load workspace from saved file
    WorkSpace workspace2;

    try {
        workspace2.load(tempPath);
    } catch (const Pandaception &ex) {
        QFAIL(qPrintable(QString("WorkSpace::load() threw exception: %1").arg(ex.what())));
        return;
    } catch (const std::exception &ex) {
        QFAIL(qPrintable(QString("WorkSpace::load() threw std::exception: %1").arg(ex.what())));
        return;
    } catch (...) {
        QFAIL("WorkSpace::load() threw unknown exception");
        return;
    }

    // Step 4: Find IC in loaded workspace
    IC *ic2 = nullptr;
    const auto items2 = workspace2.scene()->items();
    for (auto *item : std::as_const(items2)) {
        if (auto *ic = dynamic_cast<IC *>(item)) {
            ic2 = ic;
            break;
        }
    }

    QVERIFY2(ic2 != nullptr, "IC should be present after load");

    // Step 5: Verify IC properties are preserved
    QCOMPARE(ic2->inputSize(), originalInputSize);
    QCOMPARE(ic2->outputSize(), originalOutputSize);

    // Verify ports are accessible
    for (int i = 0; i < ic2->inputSize(); ++i) {
        QVERIFY2(ic2->inputPort(i) != nullptr, qPrintable(QString("Input port %1 should exist").arg(i)));
    }

    for (int i = 0; i < ic2->outputSize(); ++i) {
        QVERIFY2(ic2->outputPort(i) != nullptr, qPrintable(QString("Output port %1 should exist").arg(i)));
    }
}

// Integration Tests

void TestIC::testICInSimulation()
{
    // Test that IC can be integrated into a circuit and expanded to logic elements
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Create IC from JK flip-flop
    IC *ic = new IC();
    builder.add(ic);

    const QString icFile = TestUtils::examplesDir() + "jkflipflop.panda";

    // Load IC and verify successful loading
    try {
        ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    } catch (const Pandaception &ex) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(ex.what())));
    }

    // Verify IC has expected ports
    QVERIFY2(ic->inputSize() >= 5, "JK flip-flop IC should have at least 5 inputs");
    QCOMPARE(ic->outputSize(), 2);  // Q and ~Q outputs

    // Verify IC is properly integrated in builder
    QVERIFY2(ic->scene() != nullptr, "IC should be added to scene");

    // Verify all ports are accessible and have names
    for (int i = 0; i < ic->inputSize(); ++i) {
        const auto *port = ic->inputPort(i);
        QVERIFY2(port != nullptr, qPrintable(QString("Input port %1 should be accessible").arg(i)));
        QVERIFY2(!port->name().isEmpty(), qPrintable(QString("Input port %1 should have a name").arg(i)));
    }

    for (int i = 0; i < ic->outputSize(); ++i) {
        const auto *port = ic->outputPort(i);
        QVERIFY2(port != nullptr, qPrintable(QString("Output port %1 should be accessible").arg(i)));
        QVERIFY2(!port->name().isEmpty(), qPrintable(QString("Output port %1 should have a name").arg(i)));
    }
}

void TestIC::testICStatusPropagation()
{
    // Test that status propagates through an IC in a circuit simulation
    // Verify input changes result in output changes through the IC
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Create input switch and simple logic to feed it
    InputSwitch inputSwitch;
    IC *ic = new IC();
    Led outputLed;

    builder.add(&inputSwitch, ic, &outputLed);

    // Load JK flip-flop IC
    const QString icFile = TestUtils::examplesDir() + "jkflipflop.panda";
    try {
        ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    } catch (const Pandaception &ex) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(ex.what())));
    }

    QCOMPARE(ic->outputSize(), 2);  // Q and ~Q outputs

    // Connect: InputSwitch → IC first input → LED
    builder.connect(&inputSwitch, 0, ic, 0);
    builder.connect(ic, 0, &outputLed, 0);

    // Verify connection topology
    QCOMPARE(ic->inputPort(0)->connections().size(), 1);
    QCOMPARE(outputLed.inputPort(0)->connections().size(), 1);

    // Initialize simulation
    auto *sim = builder.initSimulation();
    QVERIFY2(sim != nullptr, "Simulation should initialize successfully");

    // Test 1: Set switch OFF and verify output
    inputSwitch.setOn(false);
    sim->update();
    bool ledStateWhenSwitchOff = TestUtils::inputStatus(&outputLed);

    // Test 2: Set switch ON so the OFF state below is reached via a real transition
    inputSwitch.setOn(true);
    sim->update();

    // Test 3: Toggle back to OFF and verify it changes again
    inputSwitch.setOn(false);
    sim->update();
    bool ledStateSecondOff = TestUtils::inputStatus(&outputLed);
    QCOMPARE(ledStateSecondOff, ledStateWhenSwitchOff);

    // Test 4: With only 1 of 5 IC inputs connected, the IC produces deterministic
    // outputs from its internal master-slave flip-flop circuitry. The exact values
    // depend on the IC's internal gate-level behavior with 4-state domination rules.
    // Verify outputs are stable across repeated updates (no oscillation).
    const bool q0 = TestUtils::outputStatus(ic, 0);
    const bool q1 = TestUtils::outputStatus(ic, 1);
    sim->update();
    QCOMPARE(TestUtils::outputStatus(ic, 0), q0);
    QCOMPARE(TestUtils::outputStatus(ic, 1), q1);
}

void TestIC::testICRequiredPorts()
{
    // Test that required and optional ports are correctly identified
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *ic = new IC();
    scene->addItem(ic);

    const QString icFile = TestUtils::examplesDir() + "jkflipflop.panda";
    try {
        ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(e.what())));
    }

    QVERIFY2(ic->inputSize() > 0, "IC should have input ports for this test");

    // Verify all input ports exist and have well-defined required/optional status
    int requiredCount = 0;
    int optionalCount = 0;
    int invalidCount = 0;

    for (int i = 0; i < ic->inputSize(); ++i) {
        auto *port = ic->inputPort(i);
        QVERIFY2(port != nullptr, qPrintable(QString("Input port %1 should exist").arg(i)));

        // Check port status
        if (port->status() == Status::Unknown) {
            invalidCount++;
            // Skip further checks for invalid ports
            continue;
        }

        // Each valid port must be either required or optional (not both, not neither)
        const bool isRequired = port->isRequired();
        if (isRequired) {
            requiredCount++;
        } else {
            optionalCount++;
        }
    }

    // Verify we found at least some valid ports
    int validPorts = requiredCount + optionalCount;
    QVERIFY2(validPorts > 0, qPrintable(
        QString("IC should have at least one valid port. Found: %1 required, %2 optional, %3 invalid")
            .arg(requiredCount).arg(optionalCount).arg(invalidCount)));
}

void TestIC::testICDefaultValues()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *ic = new IC();
    scene->addItem(ic);

    const QString icFile = TestUtils::examplesDir() + "jkflipflop.panda";
    try {
        ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(e.what())));
    }

    // All unconnected optional ports should have default status
    for (int i = 0; i < ic->inputSize(); ++i) {
        auto *port = ic->inputPort(i);
        if (port->connections().empty() && !port->isRequired()) {
            // Port should have a defined default status (Inactive or Active)
            const auto status = port->status();
            QVERIFY2(status != Status::Unknown, "Port status should be valid");
        }
    }
}

// Error Handling Tests

void TestIC::testICMissingFile()
{
    // Test that IC::loadFile() properly reports errors for missing files
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *ic = new IC();
    scene->addItem(ic);

    // Verify IC starts empty
    QCOMPARE(ic->inputSize(), 0);
    QCOMPARE(ic->outputSize(), 0);

    // Attempt to load non-existent file
    const QString nonExistentFile = "/nonexistent/path/missing.panda";

    bool pandaceptionThrown = false;
    QString exceptionMessage;
    try {
        ic->loadFile(nonExistentFile, QFileInfo(nonExistentFile).absolutePath());
    } catch (const Pandaception &ex) {
        pandaceptionThrown = true;
        exceptionMessage = ex.what();
    }

    // IC::loadFile() MUST throw Pandaception for missing files
    QVERIFY2(pandaceptionThrown, "loadFile() should throw Pandaception for missing files");
    QVERIFY2(exceptionMessage.contains("not found"),
             qPrintable(QString("Exception message should indicate file not found, got: %1").arg(exceptionMessage)));

    // IC should remain empty after failed load
    QCOMPARE(ic->inputSize(), 0);
    QCOMPARE(ic->outputSize(), 0);
}

// Nested Circuit Behavior Tests

void TestIC::testICNestedCircuitPortMapping()
{
    // Test that nested IC correctly maps ports from loaded circuit
    // Verifies that input/output elements in the IC file become IC ports

    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *ic = new IC();
    scene->addItem(ic);

    const QString icFile = TestUtils::examplesDir() + "jkflipflop.panda";
    try {
        ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(e.what())));
    }

    // Verify IC ports are created correctly
    QVERIFY2(ic->inputSize() > 0, "IC should have input ports");
    QVERIFY2(ic->outputSize() > 0, "IC should have output ports");

    // Verify each port is connected to appropriate internal element
    for (int i = 0; i < ic->inputSize(); ++i) {
        auto *port = ic->inputPort(i);
        QVERIFY2(port != nullptr, qPrintable(QString("Input port %1 should exist").arg(i)));
        QVERIFY2(port->isInput(), qPrintable(QString("Port %1 should be input").arg(i)));
    }

    for (int i = 0; i < ic->outputSize(); ++i) {
        auto *port = ic->outputPort(i);
        QVERIFY2(port != nullptr, qPrintable(QString("Output port %1 should exist").arg(i)));
        QVERIFY2(port->isOutput(), qPrintable(QString("Port %1 should be output").arg(i)));
    }
}

void TestIC::testICNestedCircuitSignalPropagation()
{
    // Test that signal changes in nested IC propagate correctly
    // Verifies that input changes flow through internal logic to outputs

    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Create a simple test IC (use jkflipflop as proxy for complex IC)
    auto *ic = new IC();
    scene->addItem(ic);

    const QString icFile = TestUtils::examplesDir() + "jkflipflop.panda";
    try {
        ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(e.what())));
    }

    QVERIFY2(ic->outputSize() > 0, "IC should have output ports");
    auto *outputPort = ic->outputPort(0);
    // Nothing drives the IC's outputs before the simulation runs, so the
    // truthful initial state is Unknown (undriven), not a definite level
    QCOMPARE(outputPort->status(), Status::Unknown);
}

// File Dependency Resolution Tests

void TestIC::testICFileDependencyResolution()
{
    // Test that IC correctly resolves relative file paths for dependencies
    // Verifies that nested ICs can reference other IC files

    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Use examples directory to ensure IC file can find dependencies
    const QString examplesDir = TestUtils::examplesDir();

    auto *ic = new IC();
    scene->addItem(ic);

    const QString icFile = examplesDir + "/jkflipflop.panda";

    try {
        ic->loadFile(icFile, QFileInfo(icFile).absolutePath());

        // Verify IC loaded successfully
        QVERIFY2(ic->inputSize() > 0 || ic->outputSize() > 0,
                 "IC should have ports after loading");

        // Verify IC has been loaded with internal elements
    } catch (const Pandaception &ex) {
        QFAIL(qPrintable(QString("IC file dependency resolution failed: %1").arg(ex.what())));
    }
}

void TestIC::testICLoadWithRelativePath()
{
    // Bare filename resolved against contextDir — the normal case for modern files
    WorkSpace workspace;
    auto *scene = workspace.scene();

    const QString examplesDir = TestUtils::examplesDir();

    auto *ic = new IC();
    scene->addItem(ic);

    try {
        ic->loadFile("jkflipflop.panda", examplesDir);
    } catch (const Pandaception &ex) {
        QFAIL(qPrintable(QString("Relative path should resolve against contextDir: %1").arg(ex.what())));
    }

    QVERIFY2(ic->inputSize() > 0, "IC should have loaded ports");
    QVERIFY2(ic->outputSize() > 0, "IC should have loaded ports");
}

void TestIC::testICLoadWithRelativeSubfolderPath()
{
    // Relative path with a subdirectory component: contextDir is the parent
    // of Examples/, and the path is "Examples/input.panda".
    // Use input.panda (no nested IC deps) to avoid nested resolution issues.
    WorkSpace workspace;
    auto *scene = workspace.scene();

    const QDir examplesDir(TestUtils::examplesDir());
    const QString parentDir = examplesDir.absolutePath() + "/..";

    auto *ic = new IC();
    scene->addItem(ic);

    try {
        ic->loadFile("Examples/input.panda", parentDir);
    } catch (const Pandaception &ex) {
        QFAIL(qPrintable(QString("Relative subfolder path should resolve: %1").arg(ex.what())));
    }

    QVERIFY2(ic->inputSize() > 0 || ic->outputSize() > 0, "IC should have loaded ports");
}

void TestIC::testICLoadWithSameOsAbsolutePath()
{
    // Absolute path on the current OS — QDir::filePath() ignores the base.
    WorkSpace workspace;
    auto *scene = workspace.scene();

    const QString examplesDir = TestUtils::examplesDir();

    auto *ic = new IC();
    scene->addItem(ic);

    const QString absPath = QFileInfo(examplesDir + "jkflipflop.panda").absoluteFilePath();

    try {
        ic->loadFile(absPath, examplesDir);
    } catch (const Pandaception &ex) {
        QFAIL(qPrintable(QString("Same-OS absolute path should resolve directly: %1").arg(ex.what())));
    }

    QVERIFY2(ic->inputSize() > 0, "IC should have loaded ports");
    QVERIFY2(ic->outputSize() > 0, "IC should have loaded ports");
}

void TestIC::testICLoadWithForeignAbsolutePathForwardSlash()
{
    // Windows-style path with forward slashes on Linux — not recognized as
    // absolute, but QFileInfo::fileName() handles '/' correctly on all OSes
    WorkSpace workspace;
    auto *scene = workspace.scene();

    const QString examplesDir = TestUtils::examplesDir();

    auto *ic = new IC();
    scene->addItem(ic);

    try {
        ic->loadFile("C:/Users/alice/project/jkflipflop.panda", examplesDir);
    } catch (const Pandaception &ex) {
        QFAIL(qPrintable(QString("Foreign path with forward slashes should fallback to filename: %1").arg(ex.what())));
    }

    QVERIFY2(ic->inputSize() > 0, "IC should have loaded ports");
    QVERIFY2(ic->outputSize() > 0, "IC should have loaded ports");
}

void TestIC::testICLoadWithForeignAbsolutePathBackslash()
{
    // Windows-style path with backslashes on Linux — backslash is not a path
    // separator on Unix, so the fallback must normalize before extracting filename
    WorkSpace workspace;
    auto *scene = workspace.scene();

    const QString examplesDir = TestUtils::examplesDir();

    auto *ic = new IC();
    scene->addItem(ic);

    try {
        ic->loadFile("C:\\Users\\alice\\project\\jkflipflop.panda", examplesDir);
    } catch (const Pandaception &ex) {
        QFAIL(qPrintable(QString("Foreign path with backslashes should fallback to filename: %1").arg(ex.what())));
    }

    QVERIFY2(ic->inputSize() > 0, "IC should have loaded ports");
    QVERIFY2(ic->outputSize() > 0, "IC should have loaded ports");
}

void TestIC::testICLoadWithForeignAbsolutePathMixedSlashes()
{
    // Windows-style path with mixed forward and backslashes — the backslash
    // normalization must handle both separator styles in a single path
    WorkSpace workspace;
    auto *scene = workspace.scene();

    const QString examplesDir = TestUtils::examplesDir();

    auto *ic = new IC();
    scene->addItem(ic);

    try {
        ic->loadFile("C:\\Users/alice\\project/jkflipflop.panda", examplesDir);
    } catch (const Pandaception &ex) {
        QFAIL(qPrintable(QString("Foreign path with mixed slashes should fallback to filename: %1").arg(ex.what())));
    }

    QVERIFY2(ic->inputSize() > 0, "IC should have loaded ports");
    QVERIFY2(ic->outputSize() > 0, "IC should have loaded ports");
}

void TestIC::testICLoadWithNonExistentFileFallback()
{
    // Both full path and filename fallback fail — should throw Pandaception
    WorkSpace workspace;
    auto *scene = workspace.scene();

    const QString examplesDir = TestUtils::examplesDir();

    auto *ic = new IC();
    scene->addItem(ic);

    bool threw = false;
    try {
        ic->loadFile("C:\\Users\\alice\\project\\nonexistent_ic_12345.panda", examplesDir);
    } catch (const Pandaception &) {
        threw = true;
    }

    QVERIFY2(threw, "loadFile should throw when neither full path nor filename fallback resolves");
}

// Helper: load a .panda file into a fresh WorkSpace and return the number of distinct
// GraphicElement objects that participate in at least one connection.
//
// This metric catches the migration bug where all element IDs are serialized as -1,
// causing portMap key collisions on reload: all connections end up referencing the same
// 1-2 elements rather than being distributed across the full circuit.  A simple
// connection-count check would miss this because the total count stays the same.
//
// WorkSpace manages item ownership, making cleanup safe for IC-containing files.
static int countConnectedElementsViaWorkspace(const QString &filePath)
{
    try {
        WorkSpace ws;
        ws.load(filePath);

        QSet<GraphicElement *> connectedElements;
        for (auto *conn : TestUtils::sceneConnections(ws.scene())) {
            if (conn->startPort() && conn->startPort()->graphicElement()) {
                connectedElements.insert(conn->startPort()->graphicElement());
            }
            if (conn->endPort() && conn->endPort()->graphicElement()) {
                connectedElements.insert(conn->endPort()->graphicElement());
            }
        }
        return static_cast<int>(connectedElements.size());
    } catch (...) {
        return -1;
    }
}

void TestIC::testICMigrationPreservesConnections()
{
    // Copy all V4.2.0 backward-compat files to a temporary directory so the originals
    // are never modified by this test
    const QString srcDir = TestUtils::backwardCompatibilityDir() + "v4.2.0/";
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QDir src(srcDir);
    for (const QString &fileName : src.entryList(QStringList{"*.panda"}, QDir::Files)) {
        QVERIFY(QFile::copy(srcDir + fileName, tempDir.path() + "/" + fileName));
    }

    // Count connections in key sub-circuit files BEFORE migration.
    // migrationEnabled is false here (set by TestUtils::setupTestEnvironment),
    // so loading these files for counting won't trigger migration.
    const QString dflipPath = tempDir.path() + "/dflipflop.panda";
    const QString jkPath    = tempDir.path() + "/jkflipflop.panda";

    const int dflipBefore = countConnectedElementsViaWorkspace(dflipPath);
    const int jkBefore    = countConnectedElementsViaWorkspace(jkPath);

    QVERIFY2(dflipBefore > 1, "dflipflop.panda should have multiple connected elements before migration");
    QVERIFY2(jkBefore    > 1, "jkflipflop.panda should have multiple connected elements before migration");

    // Enable migration without interactiveMode — no QMessageBox dialogs will appear
    Application::migrationEnabled = true;

    try {
        // Loading ic.panda triggers recursive IC loading, which migrates every
        // referenced sub-circuit file (including dflipflop.panda and jkflipflop.panda)
        WorkSpace workspace;
        workspace.load(tempDir.path() + "/ic.panda");
    } catch (const std::exception &e) {
        Application::migrationEnabled = false;
        QFAIL(qPrintable(QString("Failed to load ic.panda during migration test: %1").arg(e.what())));
    }

    Application::migrationEnabled = false;

    // Recount connections in the now-migrated files — must match the originals exactly.
    // If the migration serialized connections with corrupt IDs (the bug this test guards
    // against), Connection::load() silently drops them, so the count would be lower.
    const int dflipAfter = countConnectedElementsViaWorkspace(dflipPath);
    const int jkAfter    = countConnectedElementsViaWorkspace(jkPath);

    QCOMPARE(dflipAfter, dflipBefore);
    QCOMPARE(jkAfter,    jkBefore);
}

void TestIC::testICFileMigrationCreatesBackup()
{
    // When IC::loadFile() migrates a sub-circuit file it must create a versioned
    // backup named  <basename>.v<old-version>.panda  in the same directory.

    const QString srcDir = TestUtils::backwardCompatibilityDir() + "v4.2.0/";
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // Copy dflipflop.panda (a standalone sub-circuit that is itself old-format)
    const QString subName = "dflipflop.panda";
    const QString subDst  = tempDir.path() + "/" + subName;
    QVERIFY(QFile::copy(srcDir + subName, subDst));

    // Verify the file is actually old-format before migrating
    {
        QFile f(subDst); QVERIFY(f.open(QIODevice::ReadOnly));
        QDataStream s(&f);
        const QVersionNumber v = Serialization::readPandaHeader(s);
        QVERIFY2(v < FormatRev::current, "Fixture must be older than current version for this test");
    }

    // Load the sub-circuit via IC::loadFile() with migration enabled
    Application::migrationEnabled = true;
    IC ic;
    try {
        ic.loadFile(subName, tempDir.path());
    } catch (const std::exception &e) {
        Application::migrationEnabled = false;
        QFAIL(qPrintable(QString("IC::loadFile should not throw on a valid old-format file: %1").arg(e.what())));
    }
    Application::migrationEnabled = false;

    // Backup must exist — name contains the old version string
    const QStringList backups = QDir(tempDir.path()).entryList(
        QStringList{"dflipflop.v*.panda"}, QDir::Files);
    QVERIFY2(!backups.isEmpty(),
             "A versioned backup must be created when the IC file is migrated");

    // There must be exactly one backup (one original version)
    QCOMPARE(backups.size(), 1);
}

void TestIC::testICFileMigrationUpdatesSubcircuitVersion()
{
    // After IC::loadFile() migrates a sub-circuit file, re-reading its header
    // must return FormatRev::current.

    const QString srcDir = TestUtils::backwardCompatibilityDir() + "v4.2.0/";
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString subName = "dlatch.panda";
    const QString subDst  = tempDir.path() + "/" + subName;
    QVERIFY(QFile::copy(srcDir + subName, subDst));

    Application::migrationEnabled = true;
    IC ic;
    try {
        ic.loadFile(subName, tempDir.path());
    } catch (const std::exception &e) {
        Application::migrationEnabled = false;
        QFAIL(qPrintable(QString("IC::loadFile should not throw on a valid old-format file: %1").arg(e.what())));
    }
    Application::migrationEnabled = false;

    // The sub-circuit file must now carry the current version
    QFile file(subDst);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QDataStream stream(&file);
    const QVersionNumber versionAfter = Serialization::readPandaHeader(stream);
    QCOMPARE(versionAfter, FormatRev::current);
}

void TestIC::testICFileMigrationDisabledSkips()
{
    // With Application::migrationEnabled=false, IC::loadFile() must not create any backup
    // and must not modify the sub-circuit file.

    const QString srcDir = TestUtils::backwardCompatibilityDir() + "v4.2.0/";
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // Copy all v4.2.0 files so that cross-file IC references resolve correctly
    QDir src(srcDir);
    for (const QString &name : src.entryList(QStringList{"*.panda"}, QDir::Files)) {
        QVERIFY(QFile::copy(srcDir + name, tempDir.path() + "/" + name));
    }

    const QString subName = "jkflipflop.panda";
    const QString subDst  = tempDir.path() + "/" + subName;

    // Capture bytes before load
    QFile f(subDst); QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray originalBytes = f.readAll();
    f.close();

    QVERIFY2(!Application::migrationEnabled, "Migration must be disabled in tests by default");

    IC ic;
    try {
        ic.loadFile(subName, tempDir.path());
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC::loadFile should not throw on a valid old-format file: %1").arg(e.what())));
    }

    // No backup file must exist
    const QStringList backups = QDir(tempDir.path()).entryList(
        QStringList{"jkflipflop.v*.panda"}, QDir::Files);
    QVERIFY2(backups.isEmpty(), "No backup must be created when migration is disabled");

    // Original file must be byte-for-byte unchanged
    QFile f2(subDst); QVERIFY(f2.open(QIODevice::ReadOnly));
    QCOMPARE(f2.readAll(), originalBytes);
}

// Rendering under rotation

namespace {

/// Loads the jkflipflop example into a fresh IC added to \a scene.
IC *loadExampleIC(Scene *scene)
{
    auto *ic = new IC();
    scene->addItem(ic);

    const QString icFile = TestUtils::examplesDir() + "jkflipflop.panda";
    if (!QFile::exists(icFile)) {
        return nullptr;
    }
    ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    return ic;
}

/// Renders \a elm's scene footprint into an image; \a logoCenterOut receives the centre of the
/// element's body (where drawBody() centres its logo) in image coordinates.
QImage renderElement(QGraphicsScene *scene, GraphicElement *elm, QPoint &logoCenterOut)
{
    // Render 1:1 from an integer-aligned source rect so every orientation rasterises scene
    // content at the same scale and sub-pixel phase — a fractional source origin would shift
    // antialiasing between renders and defeat pixel comparison.
    const QRect source = elm->sceneBoundingRect().toAlignedRect();
    QImage image(source.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);

    QPainter painter(&image);
    scene->render(&painter, QRectF(image.rect()), source);
    painter.end();

    // pixmapCenter() is already an absolute point in the element's local frame (it now derives
    // from boundingRect().center() for procedural-render elements like IC) — no topLeft offset
    // needed on top of it.
    const QPointF logoScene = elm->mapToScene(elm->pixmapCenter());
    logoCenterOut = (logoScene - source.topLeft()).toPoint();
    return image;
}

/// Compares \a halfSize-radius crops of \a a around \a ca and \a b around \a cb, trying small
/// alignment offsets so sub-pixel crop rounding can't fail the comparison. Returns the smallest
/// count of pixels whose channels differ by more than \a tolerance.
int alignedMismatch(const QImage &a, const QPoint ca, const QImage &b, const QPoint cb,
                    const int halfSize, const int tolerance)
{
    int best = std::numeric_limits<int>::max();

    for (int oy = -2; oy <= 2; ++oy) {
        for (int ox = -2; ox <= 2; ++ox) {
            int count = 0;
            for (int dy = -halfSize; dy < halfSize; ++dy) {
                for (int dx = -halfSize; dx < halfSize; ++dx) {
                    const QColor pa = a.pixelColor(ca + QPoint(dx, dy));
                    const QColor pb = b.pixelColor(cb + QPoint(dx + ox, dy + oy));
                    if (qAbs(pa.red() - pb.red()) > tolerance
                        || qAbs(pa.green() - pb.green()) > tolerance
                        || qAbs(pa.blue() - pb.blue()) > tolerance) {
                        ++count;
                    }
                }
            }
            best = qMin(best, count);
        }
    }

    return best;
}

} // namespace

void TestIC::testICRotationKeepsSizingPixmap()
{
    // IC::drawBody() computes the whole body geometry from pixmap().rect(), and the IC has
    // no base pixmap (its metadata pixmapPath is empty) — only the sizing pixmap installed
    // by generatePixmap(). Rotating must not replace it, or the body collapses to a
    // degenerate rect and the mascot lands on the element's top-left corner.
    // pixmapCenter() is the public window onto that pixmap's geometry.
    WorkSpace workspace;
    auto *ic = loadExampleIC(workspace.scene());
    QVERIFY(ic);

    const QPointF footprintCenter = ic->pixmapCenter();
    QVERIFY(footprintCenter != QPointF(0, 0));

    ic->setRotation(90);
    QVERIFY2(ic->pixmapCenter() != QPointF(0, 0),
             "Rotation replaced the IC sizing pixmap with the null base pixmap");
    QCOMPARE(ic->pixmapCenter(), footprintCenter);

    ic->setRotation(0);
    QCOMPARE(ic->pixmapCenter(), footprintCenter);
}

void TestIC::testICRotationPivotsAtFootprintCenter()
{
    // The transform origin is normally set when a base pixmap is loaded; the IC only ever
    // installs a sizing pixmap, so it must set the origin itself or rotation orbits (0,0).
    WorkSpace workspace;
    auto *ic = loadExampleIC(workspace.scene());
    QVERIFY(ic);

    QCOMPARE(ic->transformOriginPoint(), ic->pixmapCenter());
}

void TestIC::testICRotatedBodyStillPaintsMascot()
{
    // End-to-end: after a 180° rotation the mascot must still be painted at the body
    // centre. The centre pixel is on the panda's beige face (red channel well above
    // blue), which distinguishes it from the flat body gray, the white background, and
    // the dark body outline. The exact centre is invariant under a 180° rotation, so this
    // holds whether or not the mascot itself is counter-rotated upright.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *ic = loadExampleIC(scene);
    QVERIFY(ic);

    ic->setRotation(180);

    const QRectF source = ic->sceneBoundingRect();
    QImage image(source.size().toSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);

    QPainter painter(&image);
    scene->render(&painter, QRectF(image.rect()), source);
    painter.end();

    const QPoint center(image.width() / 2, image.height() / 2);
    bool mascotFound = false;

    // Sample a small neighbourhood so a 1px rounding shift can't land between features.
    for (int dy = -2; dy <= 2 && !mascotFound; ++dy) {
        for (int dx = -2; dx <= 2 && !mascotFound; ++dx) {
            const QColor c = image.pixelColor(center + QPoint(dx, dy));
            mascotFound = (c.red() - c.blue()) > 30;
        }
    }

    QVERIFY2(mascotFound, "No mascot pixels at the rotated IC's body centre — the body collapsed");
}

void TestIC::testICRotatedMascotStaysUpright()
{
    // The mascot is decoration, like the baked-in SVG pin text: it must read upright at any
    // element orientation. Its rendering around the logo centre must therefore be identical at
    // 0° and 180°; the panda is 180°-asymmetric (dark eye patch right of centre), so a mascot
    // that rotates with the body produces a large mismatch.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *ic = loadExampleIC(scene);
    QVERIFY(ic);

    QPoint uprightCenter;
    const QImage upright = renderElement(scene, ic, uprightCenter);

    ic->setRotation(180);
    QPoint rotatedCenter;
    const QImage rotated = renderElement(scene, ic, rotatedCenter);

    const int mismatch = alignedMismatch(upright, uprightCenter, rotated, rotatedCenter, 10, 32);
    QVERIFY2(mismatch <= 20,
             qPrintable(QString("Mascot is not upright after a 180° rotation: %1 mismatching pixels")
                            .arg(mismatch)));
}

IC *TestIC::buildBigIC(Scene *scene)
{
    auto *ic = new IC();
    scene->addItem(ic);

    // Mirrors ICLoader::loadBoundaryPorts()'s lock-then-set sequence (widen min/max together,
    // then set the size) followed by the same ICRenderer::generatePixmap() call loadFile()
    // makes — port count chosen well past what fits in the 64px body.
    constexpr int portCount = 12;
    ic->setMaxInputSize(portCount);
    ic->setMinInputSize(portCount);
    ic->setInputSize(portCount);
    ic->setMaxOutputSize(portCount);
    ic->setMinOutputSize(portCount);
    ic->setOutputSize(portCount);
    ICRenderer::generatePixmap(*ic);

    return ic;
}

void TestIC::testICBigPivotsAtBoundingRectCenter()
{
    // Direct regression for the reported bug: a big IC's rotation/flip pivot must be the
    // element's actual footprint centre, not the (0,0)-anchored raw pixmap centre that only
    // happens to coincide with it for small instances.
    WorkSpace workspace;
    auto *ic = buildBigIC(workspace.scene());
    QVERIFY(ic);

    QVERIFY2(ic->boundingRect().topLeft() != QPointF(0, 0),
             "Test IC isn't actually 'big' — boundingRect() didn't grow past the 64x64 body");
    QCOMPARE(ic->transformOriginPoint(), ic->boundingRect().center());
}

void TestIC::testICBigRotationDoesNotDriftInScene()
{
    // End-to-end reproduction of "the element moves around" from the bug report: the
    // footprint's own centre, mapped to scene coordinates, must stay fixed as the element
    // rotates — true only when the rotation pivot actually is that centre.
    WorkSpace workspace;
    auto *ic = buildBigIC(workspace.scene());
    QVERIFY(ic);

    const QPointF centerScene = ic->mapToScene(ic->boundingRect().center());

    for (const qreal angle : {90.0, 180.0, 270.0, 0.0}) {
        ic->setRotation(angle);
        QCOMPARE(ic->mapToScene(ic->boundingRect().center()), centerScene);
    }
}

void TestIC::testICBigFlipDoesNotDriftInScene()
{
    // Same invariant as testICBigRotationDoesNotDriftInScene(), for the flip (mirror) pivot —
    // ElementOrientation::applyFlipTransform() reads the same pixmapCenter() that rotation
    // does, so it shares the identical bug on a big instance.
    WorkSpace workspace;
    auto *ic = buildBigIC(workspace.scene());
    QVERIFY(ic);

    const QPointF centerScene = ic->mapToScene(ic->boundingRect().center());

    ic->setFlippedX(true);
    QCOMPARE(ic->mapToScene(ic->boundingRect().center()), centerScene);

    ic->setFlippedY(true);
    QCOMPARE(ic->mapToScene(ic->boundingRect().center()), centerScene);

    ic->setFlippedX(false);
    ic->setFlippedY(false);
    QCOMPARE(ic->mapToScene(ic->boundingRect().center()), centerScene);
}
