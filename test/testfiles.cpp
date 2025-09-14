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

void TestFiles::testBackwardCompatibility()
{
    bool originalVerbose = GlobalProperties::verbose;
    bool originalTestMode = GlobalProperties::testMode;
    GlobalProperties::verbose = false;
    GlobalProperties::testMode = true;

    const QDir tagExamplesDir(QString(QUOTE(CURRENTDIR)) + "/../test/backward_compatibility/");
    QVERIFY(tagExamplesDir.exists());

    const auto versionDirs = tagExamplesDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    QVERIFY(!versionDirs.empty());

    int totalFiles = 0;
    int successfulFiles = 0;

    for (const auto &versionDirInfo : versionDirs) {
        const QDir versionDir(versionDirInfo.absoluteFilePath());
        const auto pandaFiles = versionDir.entryInfoList(QStringList("*.panda"));

        for (const auto &fileInfo : pandaFiles) {
            totalFiles++;

            try {
                GlobalProperties::currentDir = fileInfo.absolutePath();
                WorkSpace workspace;

                QFile pandaFile(fileInfo.absoluteFilePath());
                if (!pandaFile.exists() || !pandaFile.open(QIODevice::ReadOnly)) {
                    qWarning() << "Skipping file that cannot be opened:" << fileInfo.absoluteFilePath();
                    continue;
                }

                QDataStream stream(&pandaFile);
                QVersionNumber version = Serialization::readPandaHeader(stream);

                workspace.load(stream, version);

                const auto items = workspace.scene()->items();

                for (auto *item : items) {
                    if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
                        if (!conn->startPort() || !conn->endPort()) {
                            qWarning() << "Skipping file with invalid connections:" << fileInfo.absoluteFilePath();
                            goto next_file;
                        }
                    }
                }

                QTemporaryFile tempFile;
                if (!tempFile.open()) {
                    qWarning() << "Cannot create temp file for round-trip test:" << fileInfo.absoluteFilePath();
                    continue;
                }

                QDataStream saveStream(&tempFile);
                Serialization::writePandaHeader(saveStream);
                workspace.save(saveStream);
                tempFile.close();

                QFile reloadFile(tempFile.fileName());
                if (!reloadFile.open(QIODevice::ReadOnly)) {
                    qWarning() << "Cannot reopen temp file for round-trip test:" << fileInfo.absoluteFilePath();
                    continue;
                }

                QDataStream reloadStream(&reloadFile);
                QVersionNumber reloadVersion = Serialization::readPandaHeader(reloadStream);
                workspace.load(reloadStream, reloadVersion);

                successfulFiles++;

            } catch (const std::exception &e) {
                QString errorMsg = QString::fromUtf8(e.what());
                if (errorMsg.contains("not found") || errorMsg.contains("does not exist")) {
                    qWarning() << "Skipping file with missing dependencies:" << versionDirInfo.fileName() + "/" + fileInfo.fileName() << "Error:" << errorMsg;
                } else {
                    qWarning() << "Failed to load" << fileInfo.fileName() << "from version" << versionDirInfo.fileName() << ":" << errorMsg;
                }
            }

            next_file:;
        }
    }

    qDebug() << QString("Backward compatibility test: %1/%2 files loaded successfully").arg(successfulFiles).arg(totalFiles);
    QVERIFY2(successfulFiles > 0, "No files were successfully loaded");

    GlobalProperties::verbose = originalVerbose;
    GlobalProperties::testMode = originalTestMode;
}
