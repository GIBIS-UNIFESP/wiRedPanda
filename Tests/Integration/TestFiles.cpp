// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestFiles.h"

#include <QDir>
#include <QTemporaryFile>

#include "App/IO/Serialization.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestFiles::testFiles_data()
{
    QTest::addColumn<QString>("filePath");

    const QDir examplesDir(TestUtils::examplesDir());
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

    WorkSpace workspace;
    QVERIFY2(fileInfo.exists(), qPrintable(QString("File %1 does not exist").arg(filePath)));

    QFile pandaFile(filePath);
    QVERIFY2(pandaFile.exists(), qPrintable(QString("File %1 cannot be accessed").arg(filePath)));
    QVERIFY2(pandaFile.open(QIODevice::ReadOnly),
             qPrintable(QString("Could not open %1 for reading").arg(filePath)));

    QDataStream stream(&pandaFile);
    QVersionNumber version = Serialization::readPandaHeader(stream);
    workspace.load(stream, version, fileInfo.absolutePath());
    const auto items = workspace.scene()->items();

    // Verify connections have valid structure
    for (auto *item : items) {
        if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
            QVERIFY2(conn->startPort() != nullptr,
                     qPrintable(QString("Connection in %1 missing start port").arg(fileInfo.baseName())));
            QVERIFY2(conn->endPort() != nullptr,
                     qPrintable(QString("Connection in %1 missing end port").arg(fileInfo.baseName())));
            // Verify port validity
            QVERIFY2(conn->startPort()->isValid(),
                     qPrintable(QString("Start port invalid in %1").arg(fileInfo.baseName())));
        }
    }

    // Test round-trip: save and reload
    QTemporaryFile tempFile;
    QVERIFY2(tempFile.open(), qPrintable(QString("Could not create temp file for %1").arg(fileInfo.baseName())));

    QDataStream stream2(&tempFile);
    Serialization::writePandaHeader(stream2);
    workspace.save(stream2);
    tempFile.close();

    // Capture item count before reload
    const int originalItemCount = workspace.scene()->items().size();

    QFile pandaFile2(tempFile.fileName());
    QVERIFY2(pandaFile2.open(QIODevice::ReadOnly),
             qPrintable(QString("Could not open temp file for %1").arg(fileInfo.baseName())));

    // Use a separate workspace for reload to avoid item duplication
    WorkSpace workspaceReload;
    QDataStream stream3(&pandaFile2);
    QVersionNumber version3 = Serialization::readPandaHeader(stream3);
    workspaceReload.load(stream3, version3, fileInfo.absolutePath());
    QVERIFY2(!workspaceReload.scene()->items().isEmpty(),
             qPrintable(QString("Round-trip failed for %1: scene is empty").arg(fileInfo.baseName())));
    QVERIFY2(workspaceReload.scene()->items().size() == originalItemCount,
             qPrintable(QString("Round-trip changed item count for %1: %2 → %3")
                .arg(fileInfo.baseName()).arg(originalItemCount).arg(workspaceReload.scene()->items().size())));
}

void TestFiles::testBackwardCompatibility_data()
{
    QTest::addColumn<QString>("filename");

    const QDir bcDir(TestUtils::backwardCompatibilityDir());
    if (!bcDir.exists()) {
        return;
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
        // Load file
        WorkSpace workspace;
        QFile file(fileInfo.absoluteFilePath());
        QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable("Failed to open " + fileInfo.fileName()));

        QDataStream stream(&file);
        QVersionNumber version = Serialization::readPandaHeader(stream);
        workspace.load(stream, version, fileInfo.absolutePath());

        // Verify circuit loaded successfully with non-empty, non-Unknown elements
        auto elements = workspace.scene()->elements();
        QVERIFY2(!elements.isEmpty(), qPrintable(QString("File %1 loaded empty").arg(fileInfo.fileName())));

        for (auto *elem : elements) {
            QVERIFY2(elem->elementType() != ElementType::Unknown,
                     qPrintable(QString("File %1 contains unknown element type").arg(fileInfo.fileName())));
        }

        // Check for invalid connections
        const auto items = workspace.scene()->items();
        bool hasInvalidConnections = false;
        for (auto *item : items) {
            if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
                if (!conn->startPort() || !conn->endPort()) {
                    hasInvalidConnections = true;
                    break;
                }
            }
        }
        QVERIFY2(!hasInvalidConnections, qPrintable(QString("File has invalid connections: %1").arg(fileInfo.absoluteFilePath())));

        // Round-trip test: save and reload
        const int originalItemCount = workspace.scene()->items().size();

        QTemporaryFile tempFile;
        QVERIFY2(tempFile.open(), qPrintable(QString("Cannot create temp file for round-trip test: %1").arg(fileInfo.absoluteFilePath())));

        QDataStream saveStream(&tempFile);
        Serialization::writePandaHeader(saveStream);
        workspace.save(saveStream);
        tempFile.close();

        QFile reloadFile(tempFile.fileName());
        QVERIFY2(reloadFile.open(QIODevice::ReadOnly), qPrintable(QString("Cannot reopen temp file for round-trip test: %1").arg(fileInfo.absoluteFilePath())));

        WorkSpace workspaceReload;
        QDataStream reloadStream(&reloadFile);
        QVersionNumber reloadVersion = Serialization::readPandaHeader(reloadStream);
        workspaceReload.load(reloadStream, reloadVersion, fileInfo.absolutePath());

        QVERIFY2(workspaceReload.scene()->items().size() == originalItemCount,
            qPrintable(QString("Round-trip changed item count for %1: %2 → %3")
                .arg(fileInfo.fileName())
                .arg(originalItemCount).arg(workspaceReload.scene()->items().size())));

        file.close();
    } catch (const std::exception &e) {
        QString errorMsg = QString::fromUtf8(e.what());
        QFAIL(qPrintable(QString("Failed to load %1: %2").arg(fileInfo.fileName(), errorMsg)));
    }
}
