// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testfiles.h"

#include "common.h"
#include "globalproperties.h"
#include "qneconnection.h"
#include "registertypes.h"
#include "scene.h"
#include "serialization.h"
#include "workspace.h"

#include <QApplication>
#include <QTemporaryFile>
#include <QTest>

#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string

int main(int argc, char **argv)
{
#ifdef Q_OS_LINUX
    qputenv("QT_QPA_PLATFORM", "offscreen");
#endif

    registerTypes();

    Comment::setVerbosity(-1);

    QApplication app(argc, argv);
    app.setOrganizationName("GIBIS-UNIFESP");
    app.setApplicationName("wiRedPanda");
    app.setApplicationVersion(APP_VERSION);

    TestFiles testFiles;
    return QTest::qExec(&testFiles, argc, argv);
}

void TestFiles::testFiles()
{
    const QDir examplesDir(QString(QUOTE(CURRENTDIR)) + "/../examples/");
    const auto files = examplesDir.entryInfoList(QStringList("*.panda"));
    QVERIFY(!files.empty());

    for (const auto &fileInfo : files) {
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
                QVERIFY(conn != nullptr);
                QVERIFY(conn->startPort() != nullptr);
                QVERIFY(conn->endPort() != nullptr);
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
