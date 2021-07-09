// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testfiles.h"

#include <stdexcept>

#include "commands.h"
#include "globalproperties.h"
#include "mainwindow.h"
#include "qneconnection.h"

void TestFiles::init()
{

}

void TestFiles::cleanup()
{

}

void TestFiles::testFiles()
{
    /*  qDebug( ) << "CURRENTDIR: " << CURRENTDIR; */
    QDir examplesDir(QString("%1/../examples/").arg(CURRENTDIR));
    /*  qDebug( ) << "Examples dir:" << examplesDir.absolutePath( ); */
    QStringList entries;
    entries << "*.panda";
    QFileInfoList files = examplesDir.entryInfoList(entries);
    QVERIFY(files.size() > 0);
    int counter = 0;
    for (const QFileInfo &f : qAsConst(files)) {
          editor = new Editor(this);
        if (counter++ > 8)
            continue;
        /*    qDebug( ) << "File " << counter++ << " from " << files.size( ) << ": " << f.fileName( ); */
        QVERIFY(f.exists());
        QFile pandaFile(f.absoluteFilePath());
        GlobalProperties::currentFile = f.absoluteFilePath();
        QVERIFY(pandaFile.exists());
        QVERIFY(pandaFile.open(QFile::ReadOnly));

        QDataStream ds(&pandaFile);
        try {
            editor->load(ds, f.absoluteFilePath());
        } catch (std::runtime_error &e) {
            QFAIL(QString("Could not load the file! Error: %1").arg(QString::fromStdString(e.what())).toUtf8().constData());
        }

        QList<QGraphicsItem *> items = editor->getScene()->items();
        for (QGraphicsItem *item : qAsConst(items)) {
            if (item->type() == QNEConnection::Type) {
                QNEConnection *conn = qgraphicsitem_cast<QNEConnection *>(item);
                QVERIFY(conn != nullptr);
                QVERIFY(conn->start() != nullptr);
                QVERIFY(conn->end() != nullptr);
            }
        }
        pandaFile.close();
        QTemporaryFile outfile;
        if (outfile.open()) {
            qDebug() << outfile.fileName();
            QDataStream ds2(&outfile);
            try {
                editor->save(ds2, "");
            } catch (std::runtime_error &) {
                QFAIL(QString("Error saving project: " + outfile.fileName()).toUtf8().constData());
            }
        } else {
            QFAIL(QString("Could not open file in WriteOnly mode : " + outfile.fileName()).toUtf8().constData());
        }
        outfile.flush();
        outfile.close();

        QFile pandaFile2(outfile.fileName());
        QVERIFY(pandaFile2.open(QFile::ReadOnly));
        QDataStream ds3(&pandaFile2);
        try {
            editor->load(ds3, outfile.fileName());
        } catch (std::runtime_error &e) {
            QFAIL(QString("Could not load the file! Error: %1").arg(QString::fromStdString(e.what())).toUtf8().constData());
        }
        outfile.remove();
            delete editor;
    }
}
