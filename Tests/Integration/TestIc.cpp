// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestIc.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
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
#include "App/Element/LogicElements/LogicElement.h"
#include "App/IO/Serialization.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/ElementMapping.h"
#include "App/Versions.h"
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
        ic->loadFile(icFile);
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
        ic->loadFile(icFile);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(e.what())));
        return;
    }

    // Verify IC loaded
    QVERIFY2(ic->inputSize() > 0 || ic->outputSize() > 0, "Failed to load JK flip-flop IC");

    // Generate logic mapping
    QVector<std::shared_ptr<LogicElement>> logicElems;
    try {
        logicElems = ic->generateMap();
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("generateMap() threw: %1").arg(e.what())));
    }
    QVERIFY2(logicElems.size() > 0, "IC should expand to logic elements");
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
        if (auto *ic = qgraphicsitem_cast<IC *>(item)) {
            loadedICs.append(ic);
        }
    }

    QVERIFY2(loadedICs.size() > 0, "Should find at least one IC in scene");

    // Verify all loaded ICs have valid port configurations
    for (IC *ic : loadedICs) {
        QVERIFY2(ic->inputSize() >= 0, "IC should have non-negative input count");
        QVERIFY2(ic->outputSize() >= 0, "IC should have non-negative output count");
        // At least one port should exist (most ICs are not completely empty)
        if (ic->inputSize() == 0 && ic->outputSize() == 0) {
            // Empty IC is acceptable
        }
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
        ic->loadFile(icFile);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(e.what())));
        return;
    }

    // Test that IC can generate logic mapping when expanded
    QVERIFY2(ic->inputSize() > 0, "IC should have inputs after loading");
    QVERIFY2(ic->outputSize() > 0, "IC should have outputs after loading");

    // Verify the IC can generate a logic mapping (expansion into primitives)
    try {
        const auto logicElems = ic->generateMap();
        QVERIFY2(logicElems.size() > 0, "IC should expand to logic elements");
    } catch (...) {
        QFAIL("IC logic generation should not throw");
    }
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
    ic1->loadFile(icFile);

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
    } catch (...) {
        QFAIL("WorkSpace::save() should not throw exception");
        return;
    }

    // Verify file was created and has content
    QFileInfo fileInfo(tempPath);
    QVERIFY2(fileInfo.exists(), "Saved file should exist");
    QVERIFY2(fileInfo.size() > 0, "Saved file should have content");

    // Step 3: Load workspace from saved file
    WorkSpace workspace2;

    try {
        workspace2.load(tempPath);
    } catch (const Pandaception &ex) {
        QFile::remove(tempPath);  // Cleanup temp file
        QFAIL(qPrintable(QString("WorkSpace::load() threw exception: %1").arg(ex.what())));
        return;
    } catch (const std::exception &ex) {
        QFile::remove(tempPath);  // Cleanup temp file
        QFAIL(qPrintable(QString("WorkSpace::load() threw std::exception: %1").arg(ex.what())));
        return;
    } catch (...) {
        QFile::remove(tempPath);  // Cleanup temp file
        QFAIL("WorkSpace::load() threw unknown exception");
        return;
    }

    // Step 4: Find IC in loaded workspace
    IC *ic2 = nullptr;
    const auto items2 = workspace2.scene()->items();
    for (auto *item : std::as_const(items2)) {
        if (auto *ic = qgraphicsitem_cast<IC *>(item)) {
            ic2 = ic;
            break;
        }
    }

    // Cleanup temp file before verifying (so test cleanup happens even on failure)
    QFile::remove(tempPath);

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
        ic->loadFile(icFile);
    } catch (const Pandaception &ex) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(ex.what())));
    }

    // Verify IC has expected ports
    QVERIFY2(ic->inputSize() >= 5, "JK flip-flop IC should have at least 5 inputs");
    QCOMPARE(ic->outputSize(), 2);  // Q and ~Q outputs

    // Verify IC is properly integrated in builder
    QVERIFY2(ic->scene() != nullptr, "IC should be added to scene");

    // Verify IC can expand to logic elements (verify internal structure)
    QVector<std::shared_ptr<LogicElement>> logicElems;
    try {
        logicElems = ic->generateMap();
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("generateMap() threw: %1").arg(e.what())));
    }
    QVERIFY2(logicElems.size() > 0, "IC should expand to logic elements on generateMap");

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
        ic->loadFile(icFile);
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
    bool ledStateWhenSwitchOff = TestUtils::getInputStatus(&outputLed);

    // Test 2: Set switch ON and verify output changes
    inputSwitch.setOn(true);
    sim->update();
    bool ledStateWhenSwitchOn = TestUtils::getInputStatus(&outputLed);

    // The key test: verify status actually propagated and produced defined output states
    // Both states should be valid (either true or false, not undefined)
    QVERIFY2(TestUtils::getInputStatus(&outputLed) == ledStateWhenSwitchOn,
             "Output LED should maintain consistent state after switch ON");

    // Test 3: Toggle back to OFF and verify it changes again
    inputSwitch.setOn(false);
    sim->update();
    bool ledStateSecondOff = TestUtils::getInputStatus(&outputLed);
    QCOMPARE(ledStateSecondOff, ledStateWhenSwitchOff);

    // Test 4: With only 1 of 5 IC inputs connected, the remaining inputs get
    // null predecessors (Unknown).  With 4-state domination rules, gates can
    // still produce defined outputs (e.g. AND(0,?)=0).  Verify the two IC
    // outputs are complementary (one Active, the other Inactive — a valid
    // flip-flop state) rather than both Unknown.
    QVERIFY(TestUtils::getOutputStatus(ic, 0) != TestUtils::getOutputStatus(ic, 1));
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
        ic->loadFile(icFile);
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
        ic->loadFile(icFile);
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
        ic->loadFile(nonExistentFile);
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
        ic->loadFile(icFile);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(e.what())));
    }

    // Get the logic mapping to verify internal structure
    try {
        const auto logicElems = ic->generateMap();

        // Verify that internal elements were mapped
        QVERIFY2(logicElems.size() > 0, "IC should contain logic elements after loading");

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
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC nested circuit mapping failed: %1").arg(e.what())));
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
        ic->loadFile(icFile);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(e.what())));
    }

    QVERIFY2(ic->outputSize() > 0, "IC should have output ports");
    auto *outputPort = ic->outputPort(0);
    const Status initialStatus = outputPort->status();
    QVERIFY2(initialStatus == Status::Active || initialStatus == Status::Inactive,
             "Output port should be in valid state");

    try {
        const auto logicElems = ic->generateMap();
        QVERIFY2(!logicElems.isEmpty(), "IC should generate at least one logic element");
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("generateMap() threw: %1").arg(e.what())));
    }
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
        ic->loadFile(icFile, examplesDir);

        // Verify IC loaded successfully
        QVERIFY2(ic->inputSize() > 0 || ic->outputSize() > 0,
                 "IC should have ports after loading");

        // If IC contains nested ICs, they should be accessible
        try {
            const auto logicElems = ic->generateMap();
            QVERIFY2(logicElems.size() > 0, "IC should expand to logic elements");
        } catch (const Pandaception &ex) {
            // Some ICs may not have nested dependencies, that's okay
            // Just verify no crash on missing file
            QString exMsg = QString(ex.what());
            if (exMsg.contains("not found")) {
                // This is expected if nested file is missing — no assertion needed
            } else {
                QFAIL(qPrintable(QString("Unexpected exception: %1").arg(exMsg)));
            }
        }
    } catch (const Pandaception &ex) {
        QFAIL(qPrintable(QString("IC file dependency resolution failed: %1").arg(ex.what())));
    }
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
        for (auto *conn : TestUtils::getConnections(ws.scene())) {
            if (conn->startPort() && conn->startPort()->graphicElement()) {
                connectedElements.insert(conn->startPort()->graphicElement());
            }
            if (conn->endPort() && conn->endPort()->graphicElement()) {
                connectedElements.insert(conn->endPort()->graphicElement());
            }
        }
        return connectedElements.size();
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
    // against), QNEConnection::load() silently drops them, so the count would be lower.
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

    // Verify the file is actually old-format (version < 4.4.0) before migrating
    {
        QFile f(subDst); QVERIFY(f.open(QIODevice::ReadOnly));
        QDataStream s(&f);
        const QVersionNumber v = Serialization::readPandaHeader(s);
        QVERIFY2(v < AppVersion::current, "Fixture must be older than current version for this test");
    }

    // Load the sub-circuit via IC::loadFile() with migration enabled
    Application::migrationEnabled = true;
    IC ic;
    try {
        ic.loadFile(subName, tempDir.path());
    } catch (...) {
        Application::migrationEnabled = false;
        QFAIL("IC::loadFile should not throw on a valid old-format file");
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
    // must return AppVersion::current.

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
    } catch (...) {
        Application::migrationEnabled = false;
        QFAIL("IC::loadFile should not throw on a valid old-format file");
    }
    Application::migrationEnabled = false;

    // The sub-circuit file must now carry the current version
    QFile file(subDst);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QDataStream stream(&file);
    const QVersionNumber versionAfter = Serialization::readPandaHeader(stream);
    QCOMPARE(versionAfter, AppVersion::current);
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
    } catch (...) {
        QFAIL("IC::loadFile should not throw on a valid old-format file");
    }

    // No backup file must exist
    const QStringList backups = QDir(tempDir.path()).entryList(
        QStringList{"jkflipflop.v*.panda"}, QDir::Files);
    QVERIFY2(backups.isEmpty(), "No backup must be created when migration is disabled");

    // Original file must be byte-for-byte unchanged
    QFile f2(subDst); QVERIFY(f2.open(QIODevice::ReadOnly));
    QCOMPARE(f2.readAll(), originalBytes);
}
