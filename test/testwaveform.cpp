// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testwaveform.h"

#include "simplewaveform.h"
#include "workspace.h"

#include <QTemporaryFile>
#include <QTest>
#include <stdexcept>

void TestWaveForm::testDisplay4Bits()
{
    WorkSpace workspace;

    const QDir examplesDir(QString(CURRENTDIR) + "/../examples/");
    const QString fileName = examplesDir.absoluteFilePath("display-4bits.panda");

    QFile pandaFile(fileName);
    QVERIFY(pandaFile.open(QIODevice::ReadOnly));
    QDataStream stream(&pandaFile);
    workspace.load(stream);

    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QTextStream tempStream(&tempFile);
    QVERIFY(SimpleWaveform::saveToTxt(tempStream, &workspace));
    tempStream.flush();
    QVERIFY(tempFile.reset());

    QFile referenceFile(examplesDir.absoluteFilePath("display-4bits.txt"));

    QVERIFY(referenceFile.open(QIODevice::ReadOnly));

    QCOMPARE(tempFile.readAll().replace("\r\n", "\n"), referenceFile.readAll().replace("\r\n", "\n"));
}
