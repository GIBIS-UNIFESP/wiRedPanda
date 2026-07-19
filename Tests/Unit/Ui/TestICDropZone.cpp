// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestICDropZone.h"

#include <QDataStream>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QLabel>
#include <QMimeData>
#include <QSignalSpy>

#include "App/Core/Enums.h"
#include "App/Core/MimeTypes.h"
#include "App/IO/Serialization.h"
#include "App/UI/ICDropZone.h"
#include "Tests/Common/TestUtils.h"

namespace {

// QApplication::sendEvent() only delivers a QDropEvent to a widget that has already accepted
// a QDragEnterEvent in the same session (confirmed by an isolated probe: a bare QDropEvent
// with no preceding accepted DragEnter is silently swallowed — sendEvent returns false and
// dropEvent() never runs). Calling dropEvent() directly sidesteps that ordering requirement
// entirely, so exposing the protected override is simpler and more robust than replicating
// the DragEnter/Drop sequence in every test — the same pattern TestCommands.cpp's
// PixmapProbe uses for a differently-inaccessible member.
struct TestableICDropZone : ICDropZone {
    using ICDropZone::ICDropZone;
    using ICDropZone::dropEvent;
};

// A well-formed drag-drop payload matching the layout readDragDropPayload()
// (App/Core/DragDropPayload.cpp) expects.
QByteArray makePayload(const QString &icFileName, bool isEmbedded, const QString &blobName)
{
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    stream << QPoint(0, 0);
    stream << ElementType::IC;
    stream << icFileName;
    stream << isEmbedded;
    stream << blobName;
    return itemData;
}

} // namespace

void TestICDropZone::testICDropEvent()
{
    // Dropping a file-based IC onto the Embedded section must request embedding
    // (App/UI/ICDropZone.cpp: Section::Embedded accepts !payload->isEmbedded drops).
    TestableICDropZone dropZone(ICDropZone::Section::Embedded);
    QVERIFY(dropZone.acceptDrops());

    QSignalSpy embedSpy(&dropZone, &ICDropZone::embedByFileRequested);
    QSignalSpy extractSpy(&dropZone, &ICDropZone::extractByBlobNameRequested);

    QMimeData mimeData;
    mimeData.setData(MimeType::DragDrop, makePayload("some_ic", false, QString()));
    QDropEvent dropEvent(QPointF(5, 5), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    dropZone.dropEvent(&dropEvent);

    QCOMPARE(embedSpy.count(), 1);
    QCOMPARE(embedSpy.constFirst().at(0).toString(), QStringLiteral("some_ic"));
    QCOMPARE(extractSpy.count(), 0);
}

void TestICDropZone::testICMimeData()
{
    // Dropping an embedded IC onto the FileBased section must request extraction
    // (App/UI/ICDropZone.cpp: Section::FileBased accepts payload->isEmbedded drops).
    TestableICDropZone dropZone(ICDropZone::Section::FileBased);
    QVERIFY(dropZone.acceptDrops());

    QSignalSpy embedSpy(&dropZone, &ICDropZone::embedByFileRequested);
    QSignalSpy extractSpy(&dropZone, &ICDropZone::extractByBlobNameRequested);

    QMimeData mimeData;
    mimeData.setData(MimeType::DragDrop, makePayload(QString(), true, "some_blob"));
    QDropEvent dropEvent(QPointF(5, 5), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    dropZone.dropEvent(&dropEvent);

    QCOMPARE(extractSpy.count(), 1);
    QCOMPARE(extractSpy.constFirst().at(0).toString(), QStringLiteral("some_blob"));
    QCOMPARE(embedSpy.count(), 0);
}

void TestICDropZone::testDropEventRejectsOversizedPayload()
{
    // Crafted payload: valid panda header + offset + type, then an implausible
    // icFileName length prefix with no real data behind it.
    //
    // dropEvent() itself doesn't catch this — Serialization::readBoundedString() throws and
    // relies on the caller to catch it. In production that's Application::notify()'s
    // defense-in-depth backstop around real Qt event dispatch; calling dropEvent() directly
    // (required — see TestableICDropZone above) bypasses that, so the throw is caught here
    // instead, same pattern as TestCommands::testSplitCommandRedoThrowsBeforeAllocation.
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    stream << QPoint(0, 0);
    stream << ElementType::IC;
    stream << static_cast<quint32>(0x7FFFFFFF);

    QMimeData mimeData;
    mimeData.setData(MimeType::DragDrop, itemData);

    TestableICDropZone dropZone(ICDropZone::Section::FileBased);
    QSignalSpy embedSpy(&dropZone, &ICDropZone::embedByFileRequested);
    QSignalSpy extractSpy(&dropZone, &ICDropZone::extractByBlobNameRequested);

    QDropEvent dropEvent(QPointF(5, 5), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);

    bool threw = false;
    try {
        dropZone.dropEvent(&dropEvent);
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(threw, "Oversized icFileName length prefix must throw, not silently proceed");

    QCOMPARE(embedSpy.count(), 0);
    QCOMPARE(extractSpy.count(), 0);
}

void TestICDropZone::testHintShownOnCompatibleDragEnter()
{
    // A file-based IC payload (isEmbedded = false).
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    stream << QPoint(0, 0) << ElementType::IC << QString("counter.panda") << false << QString();

    QMimeData mimeData;
    mimeData.setData(MimeType::DragDrop, itemData);

    // The embedded section accepts a file-based IC (for embedding).
    ICDropZone dropZone(ICDropZone::Section::Embedded);
    dropZone.resize(200, 150);

    auto *hint = dropZone.findChild<QLabel *>("icDropHint");
    QVERIFY(hint);
    QVERIFY(hint->isHidden());                                      // hidden at rest
    QVERIFY2(hint->text().contains(QStringLiteral("embed")), qPrintable(hint->text()));

    // Dragging a compatible IC over the zone reveals the hint...
    QDragEnterEvent enterEvent = makeDragEnterEvent(QPoint(5, 5), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&dropZone, &enterEvent);
    QVERIFY(!hint->isHidden());

    // ...and leaving the zone hides it again.
    QDragLeaveEvent leaveEvent;
    QApplication::sendEvent(&dropZone, &leaveEvent);
    QVERIFY(hint->isHidden());
}

void TestICDropZone::testAccessibleNameSet()
{
    ICDropZone fileBasedZone(ICDropZone::Section::FileBased);
    QVERIFY(!fileBasedZone.accessibleName().isEmpty());
    QVERIFY(!fileBasedZone.whatsThis().isEmpty());

    ICDropZone embeddedZone(ICDropZone::Section::Embedded);
    QVERIFY(!embeddedZone.accessibleName().isEmpty());
    QVERIFY(!embeddedZone.whatsThis().isEmpty());

    // The two sections describe different actions -- must not be identical text.
    QVERIFY(fileBasedZone.accessibleName() != embeddedZone.accessibleName());
    QVERIFY(fileBasedZone.whatsThis() != embeddedZone.whatsThis());
}
