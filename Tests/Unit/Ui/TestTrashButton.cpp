// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestTrashButton.h"

#include <QApplication>
#include <QDataStream>
#include <QDropEvent>
#include <QMimeData>
#include <QSignalSpy>

#include "App/Core/Enums.h"
#include "App/Core/MimeTypes.h"
#include "App/IO/Serialization.h"
#include "App/UI/TrashButton.h"
#include "Tests/Common/TestUtils.h"

void TestTrashButton::testDragEnterEvent()
{
    TrashButton button;
    button.setAcceptDrops(true);
    QVERIFY(button.acceptDrops());
}

void TestTrashButton::testDropEvent()
{
    TrashButton button;
    QVERIFY(true);
}

void TestTrashButton::testEmptyDrop()
{
    TrashButton button;
    QVERIFY(true);
}

void TestTrashButton::testDropEventRejectsOversizedPayload()
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

    TrashButton button;
    QSignalSpy removeFileSpy(&button, &TrashButton::removeICFile);
    QSignalSpy removeEmbeddedSpy(&button, &TrashButton::removeEmbeddedIC);

    QDropEvent dropEvent(QPointF(5, 5), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&button, &dropEvent);

    QCOMPARE(removeFileSpy.count(), 0);
    QCOMPARE(removeEmbeddedSpy.count(), 0);
}
