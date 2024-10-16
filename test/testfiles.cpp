
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



// SPDX-License-Identifier: GPL-3.0-or-later

#include "testfiles.h"

#include "qneconnection.h"
#include "scene.h"
#include "workspace.h"

#include <QTemporaryFile>
#include <QTest>

void TestFiles::testFiles()
{
    const QDir examplesDir(QString(CURRENTDIR) + "/../examples/");
    const auto files = examplesDir.entryInfoList(QStringList("*.panda"));
    QVERIFY(!files.empty());

    for (const auto &fileInfo : files) {
        WorkSpace workspace;
        QVERIFY(fileInfo.exists());
        QFile pandaFile(fileInfo.absoluteFilePath());
        QVERIFY(pandaFile.exists());
        QVERIFY(pandaFile.open(QIODevice::ReadOnly));
        QDataStream stream(&pandaFile);
        stream.setVersion(QDataStream::Qt_5_12);
        workspace.load(stream);
        const auto items = workspace.scene()->items();

        for (auto *item : items) {
            if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
                QVERIFY(conn != nullptr);
                QVERIFY(conn->startPort() != nullptr);
                QVERIFY(conn->endPort() != nullptr);
            }
        }

        QTemporaryFile tempFile;
        QVERIFY(tempFile.open());
        QDataStream stream2(&tempFile);
        stream2.setVersion(QDataStream::Qt_5_12);
        workspace.save(stream2);
        tempFile.close();
        QFile pandaFile2(tempFile.fileName());
        QVERIFY(pandaFile2.open(QIODevice::ReadOnly));

        QDataStream stream3(&pandaFile2);
        stream3.setVersion(QDataStream::Qt_5_12);
        workspace.load(stream3);
    }
}
