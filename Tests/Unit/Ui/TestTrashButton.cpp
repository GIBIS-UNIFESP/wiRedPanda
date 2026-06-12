// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestTrashButton.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QSignalSpy>
#include <QTimer>

#include "App/Core/Enums.h"
#include "App/Core/MimeTypes.h"
#include "App/IO/Serialization.h"
#include "App/UI/TrashButton.h"
#include "Tests/Common/TestUtils.h"

/// Exposes the protected drag-and-drop handlers; delivering drop events
/// through QApplication::sendEvent requires platform drag state that the
/// offscreen platform does not provide.
struct TestableTrashButton : TrashButton {
    using TrashButton::dragEnterEvent;
    using TrashButton::dropEvent;
};

/// Builds the same drag payload ElementLabel::mimeData() produces for an IC entry
static QMimeData *makeIcDragPayload(const QString &fileName)
{
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    stream << QPoint(0, 0) << ElementType::IC << fileName << false << QString();

    auto *mimeData = new QMimeData();
    mimeData->setData(MimeType::DragDrop, itemData);
    return mimeData;
}

void TestTrashButton::testDragEnterEvent()
{
    TestableTrashButton button;

    std::unique_ptr<QMimeData> icData(makeIcDragPayload("foo.panda"));
    QDragEnterEvent enter(QPoint(5, 5), Qt::CopyAction, icData.get(), Qt::LeftButton, Qt::NoModifier);
    enter.ignore();
    button.dragEnterEvent(&enter);
    QVERIFY(enter.isAccepted());

    QMimeData plainData;
    plainData.setData("text/plain", "not an IC");
    QDragEnterEvent enterPlain(QPoint(5, 5), Qt::CopyAction, &plainData, Qt::LeftButton, Qt::NoModifier);
    enterPlain.ignore();
    button.dragEnterEvent(&enterPlain);
    QVERIFY(!enterPlain.isAccepted());
}

void TestTrashButton::testDropEvent()
{
    TestableTrashButton button;
    QSignalSpy spy(&button, &TrashButton::removeICFile);

    std::unique_ptr<QMimeData> icData(makeIcDragPayload("foo.panda"));
    QDropEvent drop(QPointF(5, 5), Qt::CopyAction, icData.get(), Qt::LeftButton, Qt::NoModifier);

    // dropEvent execs a confirmation QMessageBox; answer Yes once its event
    // loop is running
    QTimer::singleShot(0, &button, []() {
        auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        QVERIFY(box != nullptr);
        box->button(QMessageBox::Yes)->click();
    });
    button.dropEvent(&drop);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), QString("foo.panda"));
}

void TestTrashButton::testEmptyDrop()
{
    TestableTrashButton button;
    QSignalSpy spy(&button, &TrashButton::removeICFile);

    // A drop without the IC drag MIME type must be ignored entirely
    QMimeData plainData;
    plainData.setData("text/plain", "not an IC");
    QDropEvent drop(QPointF(5, 5), Qt::CopyAction, &plainData, Qt::LeftButton, Qt::NoModifier);
    drop.ignore();
    button.dropEvent(&drop);

    QCOMPARE(spy.count(), 0);
    QVERIFY(!drop.isAccepted());
}
