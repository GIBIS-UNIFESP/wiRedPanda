// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestICFixtureLayout.h"

#include <QDir>

#include "App/IO/Serialization.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestICFixtureLayout::testLayout_data()
{
    QTest::addColumn<QString>("filePath");

    const QDir componentsDir(TestUtils::cpuComponentsDir());
    const auto files = componentsDir.entryInfoList(QStringList("*.panda"), QDir::Files, QDir::Name);
    QVERIFY2(!files.empty(), "No .panda fixtures found in IC Components directory");

    for (const auto &fileInfo : files) {
        QTest::newRow(qPrintable(fileInfo.baseName())) << fileInfo.absoluteFilePath();
    }
}

void TestICFixtureLayout::testLayout()
{
    QFETCH(QString, filePath);

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(QString("Could not open %1").arg(filePath)));

    QDataStream stream(&file);
    const QVersionNumber version = Serialization::readPandaHeader(stream);

    WorkSpace workspace;
    workspace.load(stream, version, QFileInfo(filePath).absolutePath());

    const auto elements = workspace.scene()->elements();

    // Two elements at their standard footprint never legitimately intersect by more
    // than a couple of px -- 104px-spaced neighbors clear this with a wide margin.
    constexpr qreal kTolerance = 2.0;

    QStringList overlaps;
    for (qsizetype i = 0; i < elements.size(); ++i) {
        for (qsizetype j = i + 1; j < elements.size(); ++j) {
            GraphicElement *a = elements.at(i);
            GraphicElement *b = elements.at(j);
            const QRectF overlap = a->sceneBoundingRect().intersected(b->sceneBoundingRect());
            if (overlap.width() > kTolerance && overlap.height() > kTolerance) {
                overlaps << QString("'%1' (id %2) overlaps '%3' (id %4) by %5x%6 px at (%7, %8)")
                                .arg(a->label()).arg(a->id())
                                .arg(b->label()).arg(b->id())
                                .arg(overlap.width(), 0, 'f', 1).arg(overlap.height(), 0, 'f', 1)
                                .arg(overlap.x(), 0, 'f', 1).arg(overlap.y(), 0, 'f', 1);
            }
        }
    }

    QVERIFY2(overlaps.isEmpty(), qPrintable(overlaps.join('\n')));
}
