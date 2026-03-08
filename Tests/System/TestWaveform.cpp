// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/System/TestWaveform.h"

#include <QTemporaryFile>

// Standard headers
#include <exception>

#include "App/BeWavedDolphin/BeWavedDolphin.h"
#include "App/IO/Serialization.h"

#include "Tests/Common/TestUtils.h"

void TestWaveform::testDisplay4Bits()
{
    WorkSpace workspace;

    const QDir examplesDir(TestUtils::examplesDir());
    const QString fileName = examplesDir.absoluteFilePath("display-4bits.panda");

    QFile pandaFile(fileName);
    QVERIFY2(pandaFile.open(QIODevice::ReadOnly),
             qPrintable(QString("Failed to open %1: %2")
                       .arg(pandaFile.fileName(), pandaFile.errorString())));
    QDataStream stream(&pandaFile);

    QVersionNumber version;
    try {
        version = Serialization::readPandaHeader(stream);
        workspace.load(stream, version);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load %1: %2").arg(fileName, e.what())));
        return;
    }

    BewavedDolphin bewavedDolphin(workspace.scene(), false);
    try {
        bewavedDolphin.createWaveform("");
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to create waveform from %1: %2").arg(fileName, e.what())));
        return;
    }

    QTemporaryFile tempFile;
    QVERIFY2(tempFile.open(),
             qPrintable(QString("Failed to open temporary file: %1")
                       .arg(tempFile.errorString())));
    QTextStream tempStream(&tempFile);
    bewavedDolphin.saveToTxt(tempStream);
    tempStream.flush();
    QVERIFY2(tempFile.reset(), "Failed to reset temporary file position");

    QFile referenceFile(examplesDir.absoluteFilePath("display-4bits.txt"));

    QVERIFY2(referenceFile.open(QIODevice::ReadOnly),
             qPrintable(QString("Failed to open %1: %2")
                       .arg(referenceFile.fileName(), referenceFile.errorString())));

    QCOMPARE(tempFile.readAll().replace("\r\n", "\n"), referenceFile.readAll().replace("\r\n", "\n"));
}
