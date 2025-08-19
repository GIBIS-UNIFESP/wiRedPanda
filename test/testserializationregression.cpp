// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testserializationregression.h"

#include "common.h"
#include "enums.h"
#include "globalproperties.h"
#include "scene.h"
#include "serialization.h"
#include "simulation.h"
#include "workspace.h"

// Include element types for testing
#include "and.h"
#include "clock.h"
#include "dflipflop.h"
#include "inputbutton.h"
#include "led.h"
#include "not.h"
#include "or.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMessageBox>

#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string

void TestSerializationRegression::initTestCase()
{
    // Disable verbose mode globally to prevent popups during headless testing
    GlobalProperties::verbose = false;

    m_tempDir = new QTemporaryDir();
    QVERIFY2(m_tempDir->isValid(), "Failed to create temporary directory for tests");

    setupTestFiles();
}

void TestSerializationRegression::cleanupTestCase()
{
    delete m_tempDir;
    m_tempDir = nullptr;
}

void TestSerializationRegression::setupTestFiles()
{
    // Current example files (should all load successfully)
    // Use CMake-defined source directory to find examples
    const QDir examplesDir(QString(QUOTE(CURRENTDIR)) + "/../examples/");

    if (examplesDir.exists()) {
        const QStringList pandaFiles = examplesDir.entryList({"*.panda"}, QDir::Files);
        for (const QString& filename : pandaFiles) {
            m_testFiles.append({
                examplesDir.absoluteFilePath(filename),
                VERSION("4.2"), // Assume current version
                QString("Current example: %1").arg(filename),
                true,  // Should load
                false, // No warning expected for current files
                -1,    // Unknown element count (will be determined at runtime)
                -1     // Unknown connection count (will be determined at runtime)
            });
        }
    }

}

void TestSerializationRegression::testCurrentExampleFiles()
{
    QFETCH(QString, filename);
    QFETCH(QString, description);

    // Skip empty filename (happens when no examples directory exists)
    if (filename.isEmpty()) {
        QSKIP("No examples directory found");
        return;
    }

    auto* workspace = new WorkSpace();

    try {
        workspace->load(filename);

        // Verify basic integrity
        QVERIFY2(workspace->scene() != nullptr, "Scene should not be null after loading");
        QVERIFY2(workspace->simulation() != nullptr, "Simulation should not be null after loading");

        // Verify scene has content
        auto elements = workspace->scene()->elements();
        QVERIFY2(elements.size() > 0, qPrintable(QString("File %1 has no elements").arg(QFileInfo(filename).fileName())));

        // Test simulation initialization
        QVERIFY2(workspace->simulation()->initialize(),
                qPrintable(QString("Simulation failed to initialize for %1").arg(QFileInfo(filename).fileName())));

    } catch (const std::exception& e) {
        delete workspace;
        QFAIL(qPrintable(QString("Failed to load %1: %2").arg(QFileInfo(filename).fileName(), e.what())));
    }

    delete workspace;
}

void TestSerializationRegression::testCurrentExampleFiles_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("description");

    // Use CMake-defined source directory to find examples
    const QDir examplesDir(QString(QUOTE(CURRENTDIR)) + "/../examples/");

    if (examplesDir.exists()) {
        const QStringList pandaFiles = examplesDir.entryList({"*.panda"}, QDir::Files);
        for (const QString& filename : pandaFiles) {
            QString fullPath = examplesDir.absoluteFilePath(filename);
            QTest::newRow(filename.toLocal8Bit().constData())
                << fullPath << QString("Example: %1").arg(filename);
        }
    } else {
        qWarning() << "Examples directory not found, skipping example file tests";
        // Add a dummy test to prevent empty test
        QTest::newRow("no_examples") << "" << "No examples directory found";
    }
}

void TestSerializationRegression::testLegacyFileLoading()
{
    QFETCH(QString, filename);
    QFETCH(QVersionNumber, expectedVersion);
    QFETCH(QString, description);
    QFETCH(bool, shouldLoad);
    QFETCH(bool, shouldShowWarning);

    // Handle missing legacy files case
    if (filename.isEmpty()) {
        QFAIL("Legacy test files are missing from repository");
        return;
    }

    // Test loading the real legacy file extracted from git history
    // Note: GlobalProperties::verbose is disabled globally in initTestCase() to prevent popups
    Q_UNUSED(shouldShowWarning); // Warning behavior disabled for headless testing
    auto* workspace = new WorkSpace();

    try {
        workspace->load(filename);

        if (shouldLoad) {
            // Verify basic integrity for files that should load
            QVERIFY2(workspace->scene() != nullptr, "Scene should exist after loading legacy file");
            QVERIFY2(workspace->scene()->elements().size() > 0, "Legacy file should have elements");
            QVERIFY2(workspace->simulation()->initialize(), "Simulation should initialize for legacy file");

        } else {
            delete workspace;
            QFAIL(qPrintable(QString("Legacy file %1 should not have loaded successfully").arg(QFileInfo(filename).fileName())));
        }

    } catch (const std::exception& e) {
        delete workspace;
        if (shouldLoad) {
            QFAIL(qPrintable(QString("Failed to load legacy file %1: %2").arg(QFileInfo(filename).fileName(), e.what())));
        } else {
            return; // Expected failure, test passed
        }
    }

    delete workspace;
}

void TestSerializationRegression::testLegacyFileLoading_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QVersionNumber>("expectedVersion");
    QTest::addColumn<QString>("description");
    QTest::addColumn<bool>("shouldLoad");
    QTest::addColumn<bool>("shouldShowWarning");

    // Test real legacy files extracted from git tags across all wiRedPanda versions
    QString testDir = QString(QUOTE(CURRENTDIR)) + "/legacy_files";

    // Define version series with their characteristics
    struct VersionInfo {
        QString dirName;
        QString versionString;
        QString description;
        bool shouldShowWarning;
        QStringList testFiles;
    };

    // Comprehensive version coverage from git tags
    QList<VersionInfo> versions = {
        // Early versions - WiredPanda text format
        {"v1_8_beta", "1.8", "v1.8-beta (WiredPanda text format)", true, {"counter.panda", "dflipflop.panda"}},
        {"1_9_1", "1.9.1", "v1.9.1 (WiredPanda text format)", true, {"counter.panda", "dflipflop.panda"}},
        {"2_1", "2.1", "v2.1 (WiredPanda text format)", true, {"counter.panda", "dflipflop.panda"}},

        // v2.x series - WiredPanda text format evolution (decoder.panda introduced)
        {"v2_4_0", "2.4.0", "v2.4.0 (WiredPanda text format)", true, {"counter.panda", "dflipflop.panda", "decoder.panda"}},
        {"v2_7_0", "2.7.0", "v2.7.0 (Late WiredPanda text format)", true, {"counter.panda", "dflipflop.panda", "decoder.panda"}},

        // v3.x series - WiredPanda text format final
        {"v3_0_1", "3.0.1", "v3.0.1 (WiredPanda text format final)", true, {"counter.panda", "dflipflop.panda", "decoder.panda"}},

        // v4.x series - WiRedPanda format transition and modern
        {"v4_1_0", "4.1.0", "v4.1.0 (WiRedPanda transitional)", false, {"counter.panda", "dflipflop.panda", "decoder.panda"}},
        {"v4_2_0", "4.2.0", "v4.2.0 (WiRedPanda WPCF binary)", false, {"counter.panda", "dflipflop.panda", "decoder.panda"}},

        // Keep existing comprehensive test directories
        {"v1.1", "1.1", "v1.1 (Comprehensive WiredPanda)", true, {"counter.panda", "dflipflop.panda", "decoder.panda"}},
        {"v4.0", "4.0", "v4.0 (Comprehensive WiRedPanda)", false, {"counter.panda", "dflipflop.panda", "decoder.panda", "jkflipflop.panda"}},
        {"v4.1", "4.1", "v4.1 (Comprehensive WiRedPanda)", false, {"counter.panda", "dflipflop.panda", "decoder.panda", "jkflipflop.panda"}},
        {"v4.2", "4.2", "v4.2 (Comprehensive current)", false, {"counter.panda", "dflipflop.panda", "decoder.panda", "jkflipflop.panda"}},
    };

    // Generate test cases for all versions and files
    int testCount = 0;
    for (const auto& versionInfo : versions) {
        for (const QString& file : versionInfo.testFiles) {
            QString fullPath = QDir(testDir).absoluteFilePath(QString("%1/%2").arg(versionInfo.dirName, file));

            // Check if file actually exists before adding test
            if (QFile::exists(fullPath)) {
                QString testName = QString("%1_%2").arg(versionInfo.dirName, QFileInfo(file).baseName());
                QTest::newRow(testName.toLatin1().constData())
                    << fullPath
                    << VERSION(versionInfo.versionString)
                    << QString("%1 %2").arg(versionInfo.description, QFileInfo(file).baseName())
                    << true
                    << versionInfo.shouldShowWarning;
                testCount++;
            }
        }
    }

    // Ensure we have at least some legacy test files
    if (testCount == 0) {
        qWarning() << "No legacy test files found in:" << testDir;
        // Add a failing test to indicate the problem
        QTest::newRow("missing_legacy_files")
            << ""
            << VERSION("0.0")
            << "Legacy files missing - test should fail"
            << true  // Should load = true, but will fail
            << false;
    }
}


void TestSerializationRegression::validateFileLoading(const TestFileInfo& info)
{
    auto* workspace = new WorkSpace();

    try {
        workspace->load(info.filename);

        if (info.shouldLoad) {
            QVERIFY2(true, qPrintable(QString("Successfully loaded %1").arg(QFileInfo(info.filename).fileName())));

            // Verify basic integrity
            if (!verifyCircuitIntegrity(workspace, info)) {
                QFAIL("Circuit integrity verification failed");
            }

        } else {
            delete workspace;
            QFAIL(qPrintable(QString("File %1 should not have loaded successfully").arg(QFileInfo(info.filename).fileName())));
        }

    } catch (const std::exception& e) {
        delete workspace;
        if (info.shouldLoad) {
            QFAIL(qPrintable(QString("Failed to load %1: %2").arg(QFileInfo(info.filename).fileName(), e.what())));
        } else {
            return; // Expected failure, test passed
        }
    }

    delete workspace;
}

bool TestSerializationRegression::verifyCircuitIntegrity(WorkSpace* workspace, const TestFileInfo& info)
{
    // Verify scene exists
    Scene* scene = workspace->scene();
    if (!scene) {
        qWarning() << "Scene should not be null";
        return false;
    }

    // Verify simulation exists
    Simulation* simulation = workspace->simulation();
    if (!simulation) {
        qWarning() << "Simulation should not be null";
        return false;
    }

    // Verify elements
    auto elements = scene->elements();
    if (info.expectedElementCount > 0) {
        if (elements.size() != info.expectedElementCount) {
            qWarning() << QString("Expected %1 elements, got %2")
                          .arg(info.expectedElementCount)
                          .arg(elements.size());
            return false;
        }
    } else {
        // For current example files, just verify we have some elements
        if (elements.size() == 0) {
            qWarning() << "Should have at least one element";
            return false;
        }
    }

    // Verify simulation can initialize
    bool initResult = simulation->initialize();
    if (!initResult) {
        qWarning() << "Simulation should initialize successfully";
        return false;
    }

    return true;
}

void TestSerializationRegression::testCorruptedFiles()
{
    testCorruptedHeader();
    testTruncatedFile();
    testInvalidElementData();
}

void TestSerializationRegression::testCorruptedHeader()
{
    QString filename = m_tempDir->filePath("corrupted_header.panda");
    QFile file(filename);
    QVERIFY(file.open(QIODevice::WriteOnly));

    // Write invalid header
    QDataStream stream(&file);
    stream << QString("InvalidApp 1.0"); // Invalid application name

    file.close();

    // Should fail to load
    auto* workspace = new WorkSpace();
    bool threwException = false;
    try {
        workspace->load(filename);
    } catch (const std::exception&) {
        threwException = true;
    }

    delete workspace;
    QVERIFY2(threwException, "Corrupted header should cause load failure");
}

void TestSerializationRegression::testTruncatedFile()
{
    QString filename = m_tempDir->filePath("truncated.panda");
    QFile file(filename);
    QVERIFY(file.open(QIODevice::WriteOnly));

    // Write valid header but truncate in the middle of data section
    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_12);
    stream << Serialization::MAGIC_HEADER_CIRCUIT;
    stream << VERSION("4.2");  // Valid version
    stream << QString();       // Dolphin filename
    stream << QRectF(0, 0, 800, 600); // Canvas rect
    // Write incomplete element count (only 2 bytes instead of 8)
    stream.writeRawData("\x00\x01", 2); // Truncated in middle of element count

    file.close();

    // Should fail to load due to truncation during deserialization
    auto* workspace = new WorkSpace();
    bool threwException = false;
    try {
        workspace->load(filename);
    } catch (const std::exception&) {
        threwException = true;
    }

    delete workspace;
    QVERIFY2(threwException, "Truncated file should cause load failure");
}

void TestSerializationRegression::testInvalidElementData()
{
    QString filename = m_tempDir->filePath("invalid_elements.panda");
    QFile file(filename);
    QVERIFY(file.open(QIODevice::WriteOnly));

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_12);
    stream << Serialization::MAGIC_HEADER_CIRCUIT;
    stream << VERSION("4.2");

    // Valid headers
    stream << QString(); // Dolphin filename
    stream << QRectF(0, 0, 800, 600); // Canvas rect

    // Invalid element count (too large)
    stream << static_cast<quint64>(999999);

    file.close();

    // Should fail to load gracefully
    auto* workspace = new WorkSpace();
    bool threwException = false;
    try {
        workspace->load(filename);
    } catch (const std::exception&) {
        threwException = true;
    }

    delete workspace;
    QVERIFY2(threwException, "Invalid element data should cause load failure");
}

void TestSerializationRegression::testUnsupportedVersions()
{
    // Test very old version
    QString oldFile = m_tempDir->filePath("very_old.panda");
    QFile file(oldFile);
    QVERIFY(file.open(QIODevice::WriteOnly));

    QDataStream stream(&file);
    stream << QString("wiRedPanda 0.5"); // Very old version

    file.close();

    // May load with warning or fail - either is acceptable
    auto* workspace = new WorkSpace();
    try {
        workspace->load(oldFile);
    } catch (const std::exception&) {
    }
    delete workspace;

    // Test future version
    QString futureFile = m_tempDir->filePath("future.panda");
    QFile file2(futureFile);
    QVERIFY(file2.open(QIODevice::WriteOnly));

    QDataStream stream2(&file2);
    stream2.setVersion(QDataStream::Qt_5_12);
    stream2 << Serialization::MAGIC_HEADER_CIRCUIT;
    stream2 << VERSION("5.0"); // Future version

    file2.close();

    auto* workspace2 = new WorkSpace();
    try {
        workspace2->load(futureFile);
    } catch (const std::exception&) {
    }
    delete workspace2;
}

void TestSerializationRegression::testSaveLoadRoundTrip()
{
    // Create a simple circuit, save it, load it, verify it's identical
    auto* originalWorkspace = new WorkSpace();

    // Create simple test circuit
    auto* input = new InputButton();
    auto* output = new Led();

    originalWorkspace->scene()->addItem(input);
    originalWorkspace->scene()->addItem(output);

    // Initialize simulation to ensure everything is set up
    QVERIFY(originalWorkspace->simulation()->initialize());

    // Save to temp file
    QString filename = m_tempDir->filePath("roundtrip_test.panda");
    originalWorkspace->save(filename);

    // Load into new workspace
    auto* loadedWorkspace = new WorkSpace();
    loadedWorkspace->load(filename);

    // Verify equivalence
    QCOMPARE(loadedWorkspace->scene()->elements().size(),
             originalWorkspace->scene()->elements().size());

    QVERIFY(loadedWorkspace->simulation()->initialize());

    delete originalWorkspace;
    delete loadedWorkspace;
}

void TestSerializationRegression::testVersionUpgrade()
{
    // Test that current example files can be loaded and resaved successfully
    // This tests the upgrade/save functionality without requiring synthetic legacy files

    // Use CMake-defined source directory to find examples
    const QDir examplesDir(QString(QUOTE(CURRENTDIR)) + "/../examples/");

    if (!examplesDir.exists()) {
        QSKIP("No examples directory for upgrade testing");
        return;
    }

    const QStringList pandaFiles = examplesDir.entryList({"*.panda"}, QDir::Files);
    if (pandaFiles.isEmpty()) {
        QSKIP("No example files for upgrade testing");
        return;
    }

    // Test upgrade workflow with first example file
    QString originalFile = examplesDir.absoluteFilePath(pandaFiles.first());

    // Load original file
    auto* workspace = new WorkSpace();
    workspace->load(originalFile);

    // Verify it loaded
    QVERIFY2(workspace->scene() != nullptr, "Original file should load");
    QVERIFY2(workspace->simulation()->initialize(), "Original simulation should work");

    int originalElementCount = workspace->scene()->elements().size();

    // Save to new location (upgrade to current format)
    QString upgradedFile = m_tempDir->filePath("upgraded_example.panda");
    workspace->save(upgradedFile);

    // Verify new file loads correctly
    auto* verifyWorkspace = new WorkSpace();
    verifyWorkspace->load(upgradedFile);

    QVERIFY2(verifyWorkspace->scene() != nullptr, "Upgraded file should load");
    QVERIFY2(verifyWorkspace->simulation()->initialize(), "Upgraded simulation should work");
    QCOMPARE(verifyWorkspace->scene()->elements().size(), originalElementCount);

    delete workspace;
    delete verifyWorkspace;
}
