// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testwaveform.h"
#include "simplewaveform.h"

#include <QTemporaryFile>

void TestWaveForm::init()
{
    editor = new Editor(this);
    editor->setupWorkspace();
}

void TestWaveForm::cleanup()
{
    editor->deleteLater();
}

void TestWaveForm::testDisplay4Bits()
{
    QDir examplesDir(QString("%1/../examples/").arg(CURRENTDIR));
    QString fileName = examplesDir.absoluteFilePath("display-4bits.panda");
    QFile pandaFile(fileName);
    QVERIFY(pandaFile.open(QFile::ReadOnly));
    QDataStream ds(&pandaFile);
    try {
        editor->load(ds, fileName);
    } catch (std::runtime_error &e) {
        QFAIL(QString("Could not load the file! Error: %1").arg(QString::fromStdString(e.what())).toUtf8().constData());
    }
    pandaFile.close();

    QTemporaryFile outFile;
    QVERIFY(outFile.open());
    QTextStream outStream(&outFile);
    QVERIFY(SimpleWaveform::saveToTxt(outStream, editor));

    outFile.flush();
    outFile.close();

    QFile firstFile(outFile.fileName());
    QFile secndFile(examplesDir.absoluteFilePath("display-4bits.txt"));

    QVERIFY(firstFile.open(QFile::ReadOnly));
    QVERIFY(secndFile.open(QFile::ReadOnly));

    QCOMPARE(firstFile.readAll().replace("\r\n", "\n"), secndFile.readAll().replace("\r\n", "\n"));

    firstFile.close();
    secndFile.close();

    outFile.remove();
}
