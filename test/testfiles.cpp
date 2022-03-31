// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testfiles.h"

#include "editor.h"
#include "globalproperties.h"
#include "qneconnection.h"

#include <QTemporaryFile>
#include <QTest>
#include <stdexcept>

void TestFiles::testFiles()
{
    // qDebug() << "CURRENTDIR: " << CURRENTDIR;
    const QDir examplesDir(QString(CURRENTDIR) + "/../examples/");
    // qDebug() << "Examples dir: " << examplesDir.absolutePath();
    const auto files = examplesDir.entryInfoList(QStringList("*.panda"));
    QVERIFY(!files.empty());

    for (const auto &fileInfo : files) {
        auto *editor = new Editor(this);
        editor->setupWorkspace();
        QVERIFY(fileInfo.exists());
        GlobalProperties::currentFile = fileInfo.absoluteFilePath();

        try {
            QFile pandaFile(fileInfo.absoluteFilePath());
            QVERIFY(pandaFile.exists());
            QVERIFY(pandaFile.open(QFile::ReadOnly));
            QDataStream ds(&pandaFile);
            editor->load(ds);
        } catch (std::runtime_error &e) {
            QFAIL("Could not load the file! Error: " + QString(e.what()).toUtf8());
        }

        const auto items = editor->getScene()->items();

        for (auto *item : items) {
            if (item->type() == QNEConnection::Type) {
                auto *conn = qgraphicsitem_cast<QNEConnection *>(item);
                QVERIFY(conn != nullptr);
                QVERIFY(conn->start() != nullptr);
                QVERIFY(conn->end() != nullptr);
            }
        }

        QTemporaryFile tempfile;

        if (!tempfile.open()) {
            QFAIL("Could not open temporary file in ReadWrite mode: " + tempfile.fileName().toUtf8());
        }

        // qDebug() << tempfile.fileName();
        QDataStream ds(&tempfile);

        try {
            editor->save(ds, "");
        } catch (std::runtime_error &) {
            QFAIL("Error saving project: " + tempfile.fileName().toUtf8());
        }

        tempfile.close();

        try {
            QFile pandaFile(tempfile.fileName());
            QVERIFY(pandaFile.open(QFile::ReadOnly));
            QDataStream ds2(&pandaFile);
            editor->load(ds2);
        } catch (std::runtime_error &e) {
            QFAIL("Could not load the file! Error: " + QString(e.what()).toUtf8());
        }
    }
}
