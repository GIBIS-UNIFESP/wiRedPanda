// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestICFixtureLayout.h"

#include <QDir>
#include <QFile>
#include <QScopeGuard>

#include "App/Core/ItemWithId.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Wiring/Connection.h"
#include "Tests/QuickShell/IC/QuickTestUtils.h"

namespace {

// Scene-free raw load of a .panda file's elements/connections, mirroring
// ICLoader::loadFileDirectly()'s own parse step (before IC boundary-port
// substitution, which doesn't apply here -- this test loads whole circuit
// fixtures, not IC sub-circuits) and WorkSpace::load()'s equivalent path.
// Caller owns every returned item and must qDeleteAll() them (connections
// first, same double-free hazard ICLoader::loadFileDirectly() itself guards
// against via ~InputPort/~OutputPort draining their own connections).
QList<ItemWithId *> loadPandaFileRaw(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error(QString("Could not open %1").arg(filePath).toStdString());
    }

    QDataStream stream(&file);
    auto preamble = Serialization::readPreamble(stream);
    auto blobRegistry = Serialization::deserializeBlobRegistry(preamble.metadata, preamble.version);

    QHash<quint64, Port *> portMap;
    SerializationContext context = {.portMap = portMap, .version = preamble.version,
                                     .purpose = SerializationPurpose::PortableFile,
                                     .contextDir = QFileInfo(filePath).absolutePath()};
    context.blobRegistry = blobRegistry.isEmpty() ? nullptr : &blobRegistry;

    QDataStream elementsStream(&preamble.remainingPayload, QIODevice::ReadOnly);
    elementsStream.setVersion(QDataStream::Qt_5_12);
    return Serialization::deserialize(elementsStream, context);
}

} // namespace

void TestICFixtureLayout::testLayout_data()
{
    QTest::addColumn<QString>("filePath");

    const QDir componentsDir(QuickTestUtils::cpuComponentsDir());
    const auto files = componentsDir.entryInfoList(QStringList("*.panda"), QDir::Files, QDir::Name);
    QVERIFY2(!files.empty(), "No .panda fixtures found in IC Components directory");

    for (const auto &fileInfo : files) {
        QTest::newRow(qPrintable(fileInfo.baseName())) << fileInfo.absoluteFilePath();
    }
}

void TestICFixtureLayout::testLayout()
{
    QFETCH(QString, filePath);

    QList<ItemWithId *> items;
    try {
        items = loadPandaFileRaw(filePath);
    } catch (const std::exception &e) {
        QFAIL(e.what());
    }

    auto itemsGuard = qScopeGuard([&items] {
        for (qsizetype i = 0; i < items.size(); ++i) {
            if (dynamic_cast<Connection *>(items[i])) {
                delete items[i];
                items[i] = nullptr;
            }
        }
        qDeleteAll(items);
    });

    QVector<GraphicElement *> elements;
    for (auto *item : std::as_const(items)) {
        if (auto *element = dynamic_cast<GraphicElement *>(item)) {
            elements.append(element);
        }
    }

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

    // Labels' scene rects are tracked separately from element bodies (they aren't
    // unioned into GraphicElement::boundingRect() -- see labelSceneBoundingRect()'s
    // own doc comment), so a label can visually collide with a neighboring element
    // or another label even when every element-body check above passes.
    QList<QRectF> labelRects(elements.size());
    for (qsizetype i = 0; i < elements.size(); ++i) {
        labelRects[i] = elements.at(i)->labelSceneBoundingRect();
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
