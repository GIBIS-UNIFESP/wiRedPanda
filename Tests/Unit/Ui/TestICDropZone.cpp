// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestICDropZone.h"

#include <QApplication>
#include <QDataStream>
#include <QDropEvent>
#include <QMimeData>
#include <QSignalSpy>

#include "App/Core/Enums.h"
#include "App/Core/MimeTypes.h"
#include "App/IO/Serialization.h"
#include "App/UI/ICDropZone.h"
#include "Tests/Common/TestUtils.h"

void TestICDropZone::testICDropEvent()
{
    ICDropZone dropZone(ICDropZone::Section::FileBased);
    QVERIFY(dropZone.acceptDrops());
}

void TestICDropZone::testICMimeData()
{
    ICDropZone dropZone(ICDropZone::Section::Embedded);
    QVERIFY(dropZone.acceptDrops());
}

void TestICDropZone::testDropEventRejectsOversizedPayload()
{
    // Crafted payload: valid panda header + offset + type, then an implausible
    // icFileName length prefix with no real data behind it.
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    stream << QPoint(0, 0);
    stream << ElementType::IC;
    stream << static_cast<quint32>(0x7FFFFFFF);

    QMimeData mimeData;
    mimeData.setData(MimeType::DragDrop, itemData);

    ICDropZone dropZone(ICDropZone::Section::FileBased);
    QSignalSpy embedSpy(&dropZone, &ICDropZone::embedByFileRequested);
    QSignalSpy extractSpy(&dropZone, &ICDropZone::extractByBlobNameRequested);

    QDropEvent dropEvent(QPointF(5, 5), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&dropZone, &dropEvent);

    QCOMPARE(embedSpy.count(), 0);
    QCOMPARE(extractSpy.count(), 0);
}
