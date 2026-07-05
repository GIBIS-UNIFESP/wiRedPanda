// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestICFixtureLayout.h"

#include <QDir>
#include <QGraphicsSimpleTextItem>

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

    // Labels aren't part of GraphicElement::boundingRect() (it unions only ports and
    // the pixmap), so a label can visually collide with a neighboring element or
    // another label even when every element-body check above passes.
    QList<QRectF> labelRects(elements.size());
    for (qsizetype i = 0; i < elements.size(); ++i) {
        for (auto *child : elements.at(i)->childItems()) {
            auto *text = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(child);
            if (text && !text->text().isEmpty()) {
                labelRects[i] = text->sceneBoundingRect();
                break;
            }
        }
    }

    for (qsizetype i = 0; i < elements.size(); ++i) {
        if (labelRects.at(i).isEmpty()) {
            continue;
        }

        for (qsizetype j = 0; j < elements.size(); ++j) {
            if (i == j) {
                continue;
            }
            GraphicElement *other = elements.at(j);
            const QRectF overlap = labelRects.at(i).intersected(other->sceneBoundingRect());
            if (overlap.width() > kTolerance && overlap.height() > kTolerance) {
                overlaps << QString("label '%1' (id %2) overlaps '%3' (id %4) by %5x%6 px at (%7, %8)")
                                .arg(elements.at(i)->label()).arg(elements.at(i)->id())
                                .arg(other->label()).arg(other->id())
                                .arg(overlap.width(), 0, 'f', 1).arg(overlap.height(), 0, 'f', 1)
                                .arg(overlap.x(), 0, 'f', 1).arg(overlap.y(), 0, 'f', 1);
            }
        }

        for (qsizetype j = i + 1; j < elements.size(); ++j) {
            if (labelRects.at(j).isEmpty()) {
                continue;
            }
            const QRectF overlap = labelRects.at(i).intersected(labelRects.at(j));
            if (overlap.width() > kTolerance && overlap.height() > kTolerance) {
                overlaps << QString("label '%1' (id %2) overlaps label '%3' (id %4) by %5x%6 px at (%7, %8)")
                                .arg(elements.at(i)->label()).arg(elements.at(i)->id())
                                .arg(elements.at(j)->label()).arg(elements.at(j)->id())
                                .arg(overlap.width(), 0, 'f', 1).arg(overlap.height(), 0, 'f', 1)
                                .arg(overlap.x(), 0, 'f', 1).arg(overlap.y(), 0, 'f', 1);
            }
        }
    }

    QVERIFY2(overlaps.isEmpty(), qPrintable(overlaps.join('\n')));
}
