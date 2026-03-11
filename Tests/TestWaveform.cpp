// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/TestWaveform.h"

#include <QApplication>
#include <QTemporaryFile>
#include <QTest>

#include "App/BeWavedDolphin/BeWavedDolphin.h"
#include "App/Core/Common.h"
#include "App/IO/Serialization.h"
#include "App/RegisterTypes.h"
#include "App/Scene/Workspace.h"

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

    TestWaveForm testWaveForm;
    return QTest::qExec(&testWaveForm, argc, argv);
}

void TestWaveForm::testDisplay4Bits()
{
    WorkSpace workspace;

    const QDir examplesDir(QString(QUOTE(CURRENTDIR)) + "/../Examples/");
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
