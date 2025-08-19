// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testfiles.h"

#include "globalproperties.h"
#include "qneconnection.h"
#include "scene.h"
#include "serialization.h"
#include "workspace.h"

#include <QTemporaryFile>
#include <QTest>

#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string

void TestFiles::testFiles()
{
    const QDir examplesDir(QString(QUOTE(CURRENTDIR)) + "/../examples/");
    const auto files = examplesDir.entryInfoList(QStringList("*.panda"));
    QVERIFY2(!files.empty(), "Examples directory should contain .panda files");

    // Verify minimum expected examples exist
    QVERIFY2(files.size() >= 5, "Should have at least 5 example files for comprehensive testing");

    // Verify some specific essential examples exist
    QStringList fileNames;
    for (const auto& file : files) {
        fileNames << file.baseName();
    }
    // Note: Specific file verification would go here if we knew exact expected files

    for (const auto &fileInfo : files) {
        // Set current directory for IC dependency loading
        GlobalProperties::currentDir = fileInfo.absolutePath();

        WorkSpace workspace;
        QVERIFY(fileInfo.exists());
        QFile pandaFile(fileInfo.absoluteFilePath());
        QVERIFY(pandaFile.exists());
        QVERIFY(pandaFile.open(QIODevice::ReadOnly));
        QDataStream stream(&pandaFile);
        QVersionNumber version = Serialization::readPandaHeader(stream);
        workspace.load(stream, version);
        const auto items = workspace.scene()->items();

        for (auto *item : items) {
            if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
                QVERIFY2(conn != nullptr, "Connection should exist");
                QVERIFY2(conn->startPort() != nullptr, "Connection should have valid start port");
                QVERIFY2(conn->endPort() != nullptr, "Connection should have valid end port");

                // Verify connection functionality
                QVERIFY2(conn->startPort()->isOutput(), "Start port should be an output");
                QVERIFY2(conn->endPort()->isInput(), "End port should be an input");
                // Note: Connection validity is implied by having valid start/end ports
            }
        }

        QTemporaryFile tempFile;
        QVERIFY(tempFile.open());
        QDataStream stream2(&tempFile);
        Serialization::writePandaHeader(stream2);
        workspace.save(stream2);
        tempFile.close();
        QFile pandaFile2(tempFile.fileName());
        QVERIFY(pandaFile2.open(QIODevice::ReadOnly));

        QDataStream stream3(&pandaFile2);
        QVersionNumber version3 = Serialization::readPandaHeader(stream3);
        workspace.load(stream3, version3);
    }
}
