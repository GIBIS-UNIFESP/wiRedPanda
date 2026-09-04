// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestFiles.h"

#include <QDataStream>
#include <QDir>
#include <QTemporaryFile>

#include "App/Core/Enums.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/QuickShell/IC/QuickTestUtils.h"

void TestFiles::testFiles_data()
{
    QTest::addColumn<QString>("filePath");

    const QDir examplesDir(QuickTestUtils::examplesDir());
    const auto files = examplesDir.entryInfoList(QStringList("*.panda"));
    QVERIFY2(!files.empty(), "No .panda example files found in examples directory");

    for (const auto &fileInfo : files) {
        QTest::newRow(qPrintable(fileInfo.baseName()))
            << fileInfo.absoluteFilePath();
    }
}

void TestFiles::testFiles()
{
    QFETCH(QString, filePath);

    QFileInfo fileInfo(filePath);

    QuickWorkSpace workspace;
    QVERIFY2(fileInfo.exists(), qPrintable(QString("File %1 does not exist").arg(filePath)));

    QFile pandaFile(filePath);
    QVERIFY2(pandaFile.exists(), qPrintable(QString("File %1 cannot be accessed").arg(filePath)));
    QVERIFY2(pandaFile.open(QIODevice::ReadOnly),
             qPrintable(QString("Could not open %1 for reading").arg(filePath)));

    QDataStream stream(&pandaFile);
    QVersionNumber version = Serialization::readPandaHeader(stream);
    workspace.load(stream, version, fileInfo.absolutePath());
    CanvasItem *canvas = workspace.canvas();

    // Verify connections have valid structure
    for (auto *conn : canvas->connections()) {
        QVERIFY2(conn->startPort() != nullptr,
                 qPrintable(QString("Connection in %1 missing start port").arg(fileInfo.baseName())));
        QVERIFY2(conn->endPort() != nullptr,
                 qPrintable(QString("Connection in %1 missing end port").arg(fileInfo.baseName())));
        QVERIFY2(conn->startPort()->isValid(),
                 qPrintable(QString("Start port invalid in %1").arg(fileInfo.baseName())));
    }

    // Test round-trip: save and reload
    QTemporaryFile tempFile;
    QVERIFY2(tempFile.open(), qPrintable(QString("Could not create temp file for %1").arg(fileInfo.baseName())));

    QDataStream stream2(&tempFile);
    Serialization::writePandaHeader(stream2);
    workspace.save(stream2);
    tempFile.close();

    // Capture item count before reload
    const qsizetype originalItemCount = canvas->elements().size() + canvas->connections().size();

    QFile pandaFile2(tempFile.fileName());
    QVERIFY2(pandaFile2.open(QIODevice::ReadOnly),
             qPrintable(QString("Could not open temp file for %1").arg(fileInfo.baseName())));

    // Use a separate workspace for reload to avoid item duplication
    QuickWorkSpace workspaceReload;
    QDataStream stream3(&pandaFile2);
    QVersionNumber version3 = Serialization::readPandaHeader(stream3);
    workspaceReload.load(stream3, version3, fileInfo.absolutePath());
    CanvasItem *reloadedCanvas = workspaceReload.canvas();
    const qsizetype reloadedItemCount = reloadedCanvas->elements().size() + reloadedCanvas->connections().size();

    QVERIFY2(reloadedItemCount != 0,
             qPrintable(QString("Round-trip failed for %1: scene is empty").arg(fileInfo.baseName())));
    QVERIFY2(reloadedItemCount == originalItemCount,
             qPrintable(QString("Round-trip changed item count for %1: %2 → %3")
                .arg(fileInfo.baseName()).arg(originalItemCount).arg(reloadedItemCount)));
}

void TestFiles::testBackwardCompatibility_data()
{
    QTest::addColumn<QString>("filename");

    const QDir bcDir(QuickTestUtils::backwardCompatibilityDir());
    if (!bcDir.exists()) {
        // Skip instead of returning zero rows: running a data-driven body with
        // an empty table makes QFETCH hit a QFATAL assert and abort the whole
        // test binary.
        QSKIP("BackwardCompatibility directory not found");
    }

    const auto versionDirs = bcDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const auto &versionEntry : versionDirs) {
        QDir versionDir(versionEntry.absoluteFilePath());
        const auto files = versionDir.entryList(QStringList("*.panda"), QDir::Files);
        for (const auto &file : files) {
            const QString rowName = versionEntry.fileName() + "/" + file;
            const QString absolutePath = versionDir.absoluteFilePath(file);
            QTest::newRow(qPrintable(rowName)) << absolutePath;
        }
    }
}

void TestFiles::testBackwardCompatibility()
{
    QFETCH(QString, filename);

    QFileInfo fileInfo(filename);

    try {
        QuickWorkSpace workspace;
        QFile file(fileInfo.absoluteFilePath());
        QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable("Failed to open " + fileInfo.fileName()));

        QDataStream stream(&file);
        QVersionNumber version = Serialization::readPandaHeader(stream);
        workspace.load(stream, version, fileInfo.absolutePath());
        CanvasItem *canvas = workspace.canvas();

        // Verify circuit loaded successfully with non-empty, non-Unknown elements
        const auto &elements = canvas->elements();
        QVERIFY2(!elements.isEmpty(), qPrintable(QString("File %1 loaded empty").arg(fileInfo.fileName())));

        for (auto *elem : elements) {
            QVERIFY2(elem->elementType() != ElementType::Unknown,
                     qPrintable(QString("File %1 contains unknown element type").arg(fileInfo.fileName())));
        }

        // Check for invalid connections
        bool hasInvalidConnections = false;
        for (auto *conn : canvas->connections()) {
            if (!conn->startPort() || !conn->endPort()) {
                hasInvalidConnections = true;
                break;
            }
        }
        QVERIFY2(!hasInvalidConnections, qPrintable(QString("File has invalid connections: %1").arg(fileInfo.absoluteFilePath())));

        // Round-trip test: save and reload
        const qsizetype originalItemCount = canvas->elements().size() + canvas->connections().size();

        QTemporaryFile tempFile;
        QVERIFY2(tempFile.open(), qPrintable(QString("Cannot create temp file for round-trip test: %1").arg(fileInfo.absoluteFilePath())));

        QDataStream saveStream(&tempFile);
        Serialization::writePandaHeader(saveStream);
        workspace.save(saveStream);
        tempFile.close();

        QFile reloadFile(tempFile.fileName());
        QVERIFY2(reloadFile.open(QIODevice::ReadOnly), qPrintable(QString("Cannot reopen temp file for round-trip test: %1").arg(fileInfo.absoluteFilePath())));

        QuickWorkSpace workspaceReload;
        QDataStream reloadStream(&reloadFile);
        QVersionNumber reloadVersion = Serialization::readPandaHeader(reloadStream);
        workspaceReload.load(reloadStream, reloadVersion, fileInfo.absolutePath());
        CanvasItem *reloadedCanvas = workspaceReload.canvas();
        const qsizetype reloadedItemCount = reloadedCanvas->elements().size() + reloadedCanvas->connections().size();

        QVERIFY2(reloadedItemCount == originalItemCount,
            qPrintable(QString("Round-trip changed item count for %1: %2 → %3")
                .arg(fileInfo.fileName())
                .arg(originalItemCount).arg(reloadedItemCount)));

        file.close();
    } catch (const std::exception &e) {
        QString errorMsg = QString::fromUtf8(e.what());
        QFAIL(qPrintable(QString("Failed to load %1: %2").arg(fileInfo.fileName(), errorMsg)));
    }
}
