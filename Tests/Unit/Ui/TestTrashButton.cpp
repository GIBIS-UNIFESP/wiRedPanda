// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestTrashButton.h"

#include <QApplication>
#include <QDataStream>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QPoint>
#include <QSignalSpy>
#include <QTimer>

#include "App/Core/Enums.h"
#include "App/Core/MimeTypes.h"
#include "App/IO/Serialization.h"
#include "App/UI/TrashButton.h"
#include "Tests/Common/TestUtils.h"

namespace {

// QApplication::sendEvent() only delivers a QDropEvent to a widget that has already accepted
// a QDragEnterEvent in the same session (confirmed by an isolated probe: a bare QDropEvent
// with no preceding accepted DragEnter is silently swallowed — sendEvent returns false and
// dropEvent() never runs). Calling dropEvent() directly sidesteps that ordering requirement
// entirely, so exposing the protected overrides is simpler and more robust than replicating
// the DragEnter/Drop sequence in every test — the same pattern TestCommands.cpp's PixmapProbe
// uses for a differently-inaccessible member. Used for dragEnterEvent() too for consistency.
struct TestableTrashButton : TrashButton {
    using TrashButton::TrashButton;
    using TrashButton::dragEnterEvent;
    using TrashButton::dropEvent;
};

// A well-formed (non-oversized) drag-drop payload matching the layout readDragDropPayload()
// (App/Core/DragDropPayload.cpp) expects: offset, type, icFileName, isEmbedded, blobName.
QByteArray makeIcDragPayload()
{
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    stream << QPoint(0, 0);
    stream << ElementType::IC;
    stream << QStringLiteral("some_ic");
    stream << false; // isEmbedded
    return itemData;
}

} // namespace

void TestTrashButton::testDragEnterEvent()
{
    // dragEnterEvent() must accept the proposed action only for a recognised drag-drop MIME
    // format (App/UI/TrashButton.cpp).
    TestableTrashButton button;

    QMimeData mimeData;
    mimeData.setData(MimeType::DragDrop, makeIcDragPayload());

    QDragEnterEvent event(QPoint(5, 5), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    button.dragEnterEvent(&event);

    QVERIFY(event.isAccepted());
}

void TestTrashButton::testDropEvent()
{
    // A recognised drop shows a Yes/No confirmation QMessageBox (App/UI/TrashButton.cpp);
    // clicking Yes emits removeICFile() with the dropped IC's file name.
    TestableTrashButton button;
    QSignalSpy removeFileSpy(&button, &TrashButton::removeICFile);
    QSignalSpy removeEmbeddedSpy(&button, &TrashButton::removeEmbeddedIC);

    QMimeData mimeData;
    mimeData.setData(MimeType::DragDrop, makeIcDragPayload());

    auto dismisser = TestUtils::AutoDismisser::answerMessageBox(QMessageBox::Yes);

    QDropEvent dropEvent(QPointF(5, 5), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    button.dropEvent(&dropEvent);

    QVERIFY2(TestUtils::waitFor([&] { return dismisser.dismissCount() >= 1; }),"The delete-confirmation dialog must have appeared");
    QCOMPARE(removeFileSpy.count(), 1);
    QCOMPARE(removeFileSpy.constFirst().at(0).toString(), QStringLiteral("some_ic"));
    QCOMPARE(removeEmbeddedSpy.count(), 0);
}

void TestTrashButton::testEmptyDrop()
{
    // A drop whose MIME data carries neither recognised drag-drop format must be a no-op:
    // no confirmation dialog, no signal (TrashButton::dropEvent's outer format-check guard).
    TestableTrashButton button;
    QSignalSpy removeFileSpy(&button, &TrashButton::removeICFile);
    QSignalSpy removeEmbeddedSpy(&button, &TrashButton::removeEmbeddedIC);

    QMimeData mimeData; // no format set at all

    QDropEvent dropEvent(QPointF(5, 5), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    button.dropEvent(&dropEvent);

    QCOMPARE(removeFileSpy.count(), 0);
    QCOMPARE(removeEmbeddedSpy.count(), 0);
}

void TestTrashButton::testDropEventRejectsOversizedPayload()
{
    // Crafted payload: valid panda header + offset + type, then an implausible
    // icFileName length prefix with no real data behind it.
    //
    // dropEvent() itself doesn't catch this — Serialization::readBoundedString() throws and
    // relies on the caller to catch it. In production that's Application::notify()'s
    // defense-in-depth backstop around real Qt event dispatch; calling dropEvent() directly
    // (required — see TestableTrashButton above) bypasses that, so the throw is caught here
    // instead, same pattern as TestCommands::testSplitCommandRedoThrowsBeforeAllocation.
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    stream << QPoint(0, 0);
    stream << ElementType::IC;
    stream << static_cast<quint32>(0x7FFFFFFF);

    QMimeData mimeData;
    mimeData.setData(MimeType::DragDrop, itemData);

    TestableTrashButton button;
    QSignalSpy removeFileSpy(&button, &TrashButton::removeICFile);
    QSignalSpy removeEmbeddedSpy(&button, &TrashButton::removeEmbeddedIC);

    QDropEvent dropEvent(QPointF(5, 5), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);

    bool threw = false;
    try {
        button.dropEvent(&dropEvent);
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(threw, "Oversized icFileName length prefix must throw, not silently proceed");

    QCOMPARE(removeFileSpy.count(), 0);
    QCOMPARE(removeEmbeddedSpy.count(), 0);
}
