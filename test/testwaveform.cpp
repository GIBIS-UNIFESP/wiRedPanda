// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testwaveform.h"

#include "editor.h"
#include "simplewaveform.h"

#include <QTemporaryFile>
#include <QTest>
#include <stdexcept>

void TestWaveForm::testDisplay4Bits()
{
    auto *editor = new Editor(this);
    editor->setupWorkspace();

    const QDir examplesDir(QString(CURRENTDIR) + "/../examples/");
    const QString fileName = examplesDir.absoluteFilePath("display-4bits.panda");

    try {
        QFile pandaFile(fileName);
        QVERIFY(pandaFile.open(QFile::ReadOnly));
        QDataStream ds(&pandaFile);
        editor->load(ds);
    } catch (std::runtime_error &e) {
        QFAIL("Could not load the file! Error: " + QString(e.what()).toUtf8());
    }

    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QTextStream tempStream(&tempFile);
    QVERIFY(SimpleWaveform::saveToTxt(tempStream, editor));
    tempStream.flush();
    QVERIFY(tempFile.reset());

    QFile referenceFile(examplesDir.absoluteFilePath("display-4bits.txt"));

    QVERIFY(referenceFile.open(QFile::ReadOnly));

    QCOMPARE(tempFile.readAll().replace("\r\n", "\n"), referenceFile.readAll().replace("\r\n", "\n"));
}
