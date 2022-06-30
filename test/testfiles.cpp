// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testfiles.h"

#include "globalproperties.h"
#include "qneconnection.h"
#include "scene.h"
#include "workspace.h"

#include <QTemporaryFile>
#include <QTest>
#include <stdexcept>

void TestFiles::testFiles()
{
    const QDir examplesDir(QString(CURRENTDIR) + "/../examples/");
    const auto files = examplesDir.entryInfoList(QStringList("*.panda"));
    QVERIFY(!files.empty());

    for (const auto &fileInfo : files) {
        auto workspace = std::make_unique<WorkSpace>();
        QVERIFY(fileInfo.exists());
        QFile pandaFile(fileInfo.absoluteFilePath());
        QVERIFY(pandaFile.exists());
        QVERIFY(pandaFile.open(QIODevice::ReadOnly));
        QDataStream stream(&pandaFile);
        workspace->load(stream);
        const auto items = workspace->scene()->items(Qt::SortOrder(-1));

        for (auto *item : items) {
            if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
                QVERIFY(conn != nullptr);
                QVERIFY(conn->start() != nullptr);
                QVERIFY(conn->end() != nullptr);
            }
        }

        QTemporaryFile tempFile;
        QVERIFY(tempFile.open());
        QDataStream stream2(&tempFile);
        workspace->save(stream2);
        tempFile.close();
        QFile pandaFile2(tempFile.fileName());
        QVERIFY(pandaFile2.open(QIODevice::ReadOnly));

        QDataStream stream3(&pandaFile2);
        workspace->load(stream3);
    }
}
