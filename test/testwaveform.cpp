// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testwaveform.h"

#include "bewaveddolphin.h"
#include "serialization.h"
#include "workspace.h"

#include <QTemporaryFile>
#include <QTest>

#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string

void TestWaveForm::testDisplay4Bits()
{
    WorkSpace workspace;

    const QDir examplesDir(QString(QUOTE(CURRENTDIR)) + "/../examples/");
    const QString fileName = examplesDir.absoluteFilePath("display-4bits.panda");

    QFile pandaFile(fileName);
    QVERIFY(pandaFile.open(QIODevice::ReadOnly));
    QDataStream stream(&pandaFile);
    QVersionNumber version = Serialization::readPandaHeader(stream);
    workspace.load(stream, version);

    BewavedDolphin bewavedDolphin(workspace.scene(), false);
    bewavedDolphin.createWaveform("");

    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QTextStream tempStream(&tempFile);
    bewavedDolphin.saveToTxt(tempStream);
    tempStream.flush();
    QVERIFY(tempFile.reset());

    QFile referenceFile(examplesDir.absoluteFilePath("display-4bits.txt"));

    QVERIFY(referenceFile.open(QIODevice::ReadOnly));

    QCOMPARE(tempFile.readAll().replace("\r\n", "\n"), referenceFile.readAll().replace("\r\n", "\n"));
}
