// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElementLabel.h"

#include <algorithm>
#include <memory>

#include <QApplication>
#include <QDataStream>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPoint>
#include <QSignalSpy>
#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementLabel.h"
#include "App/IO/Serialization.h"
#include "App/Presentation/ElementPictures.h"
#include "Tests/Common/TestUtils.h"

// ============================================================================
// Constructor Tests
// ============================================================================

void TestElementLabel::testConstructorWithPixmapPointer()
{
    QPixmap pixmap = ElementPictures::typePicture(ElementType::And);
    ElementLabel label(pixmap, ElementType::And, "");

    QCOMPARE(label.elementType(), ElementType::And);
    QVERIFY(!label.pixmap().isNull());
}

void TestElementLabel::testConstructorWithPixmapReference()
{
    QPixmap pixmap = ElementPictures::typePicture(ElementType::Or);
    ElementLabel label(pixmap, ElementType::Or, "");

    QCOMPARE(label.elementType(), ElementType::Or);
    QVERIFY(!label.pixmap().isNull());
}

// ============================================================================
// Data Accessor Tests
// ============================================================================

void TestElementLabel::testElementType()
{
    QPixmap pixmap = ElementPictures::typePicture(ElementType::Nand);
    ElementLabel label(pixmap, ElementType::Nand, "");

    QCOMPARE(label.elementType(), ElementType::Nand);
}

void TestElementLabel::testPixmap()
{
    QPixmap pixmap = ElementPictures::typePicture(ElementType::And);
    ElementLabel label(pixmap, ElementType::And, "");

    QVERIFY(!label.pixmap().isNull());
    QCOMPARE(label.pixmap().size(), pixmap.size());
}

void TestElementLabel::testName()
{
    QPixmap pixmap = ElementPictures::typePicture(ElementType::Not);
    ElementLabel label(pixmap, ElementType::Not, "");

    QString name = label.name();
    QVERIFY(!name.isEmpty());
    // Name should be translated name of element
    QCOMPARE(name, ElementFactory::translatedName(ElementType::Not));
}

void TestElementLabel::testIcFileName()
{
    QPixmap pixmap = ElementPictures::typePicture(ElementType::And);
    QString fileName = "test_circuit.panda";
    ElementLabel label(pixmap, ElementType::And, fileName);

    QCOMPARE(label.icFileName(), fileName);
}

// ============================================================================
// Name Management Tests
// ============================================================================

void TestElementLabel::testUpdateNameStandardElement()
{
    QPixmap pixmap = ElementPictures::typePicture(ElementType::And);
    ElementLabel label(pixmap, ElementType::And, "");

    QString originalName = label.name();
    label.updateName();

    // Name should remain the same after update for standard elements
    QCOMPARE(label.name(), originalName);
    QCOMPARE(label.name(), ElementFactory::translatedName(ElementType::And));
}

void TestElementLabel::testUpdateNameICElement()
{
    QPixmap pixmap = ElementPictures::typePicture(ElementType::IC);
    ElementLabel label(pixmap, ElementType::IC, "my_circuit");

    // For IC elements, name should be based on filename
    QString name = label.name();
    QVERIFY(!name.isEmpty());

    // After update, it should still be based on filename
    label.updateName();
    QVERIFY(!label.name().isEmpty());
}

void TestElementLabel::testNameForDifferentElementTypes()
{
    QVector<ElementType> types = {
        ElementType::And, ElementType::Or, ElementType::Not,
        ElementType::Nand, ElementType::Nor, ElementType::Xor
    };

    for (ElementType type : types) {
        QPixmap pixmap = ElementPictures::typePicture(type);
        ElementLabel label(pixmap, type, "");

        QString name = label.name();
        QVERIFY(!name.isEmpty());
        QCOMPARE(name, ElementFactory::translatedName(type));
    }
}

// ============================================================================
// Icon Tests
// ============================================================================

void TestElementLabel::testSetPixmapShowsIt()
{
    ElementLabel label(ElementPictures::typePicture(ElementType::And), ElementType::And, "");
    const QPixmap other = ElementPictures::typePicture(ElementType::Or);

    label.setPixmap(other);

    QCOMPARE(label.pixmap().toImage(), other.toImage());
    const auto children = label.findChildren<QLabel *>();
    QVERIFY(std::any_of(children.cbegin(), children.cend(),
                        [&other](const QLabel *child) { return child->pixmap().toImage() == other.toImage(); }));
}

// ============================================================================
// MIME Data Tests
// ============================================================================

void TestElementLabel::testMimeData()
{
    QPixmap pixmap = ElementPictures::typePicture(ElementType::And);
    ElementLabel label(pixmap, ElementType::And, "");

    auto data = std::unique_ptr<QMimeData>(label.mimeData());
    QVERIFY(data != nullptr);

    // Should have wiredpanda-dragdrop format
    QVERIFY(data->hasFormat("application/x-wiredpanda-dragdrop"));
}

void TestElementLabel::testMimeDataContent()
{
    QPixmap pixmap = ElementPictures::typePicture(ElementType::Nand);
    ElementLabel label(pixmap, ElementType::Nand, "");

    auto data = std::unique_ptr<QMimeData>(label.mimeData());
    QVERIFY(data != nullptr);

    // Decode the actual serialized fields (ElementLabel::mimeData(): header, offset,
    // elementType, icFileName, isEmbedded, embedded-name-or-empty) instead of only checking
    // the payload isn't empty.
    QByteArray mimeBytes = data->data("application/x-wiredpanda-dragdrop");
    QVERIFY(mimeBytes.size() > 0);

    QDataStream stream(mimeBytes);
    Serialization::readPandaHeader(stream);
    QPoint offset;
    ElementType elementType;
    QString icFileName;
    bool isEmbedded = true;
    QString embeddedName;
    stream >> offset >> elementType >> icFileName >> isEmbedded >> embeddedName;

    QCOMPARE(stream.status(), QDataStream::Ok);
    QCOMPARE(elementType, ElementType::Nand);
    QVERIFY(icFileName.isEmpty());
    QVERIFY(!isEmbedded);
    QVERIFY(embeddedName.isEmpty());
}

void TestElementLabel::testMimeDataWithIC()
{
    QPixmap pixmap = ElementPictures::typePicture(ElementType::IC);
    ElementLabel label(pixmap, ElementType::IC, "custom_ic.panda", nullptr, true);

    auto data = std::unique_ptr<QMimeData>(label.mimeData());
    QVERIFY(data != nullptr);

    // Should contain the IC file information -- decode and check the actual fields,
    // including the embedded-name field, which only carries a value when isEmbedded is true.
    QByteArray mimeBytes = data->data("application/x-wiredpanda-dragdrop");
    QVERIFY(mimeBytes.size() > 0);

    QDataStream stream(mimeBytes);
    Serialization::readPandaHeader(stream);
    QPoint offset;
    ElementType elementType;
    QString icFileName;
    bool isEmbedded = false;
    QString embeddedName;
    stream >> offset >> elementType >> icFileName >> isEmbedded >> embeddedName;

    QCOMPARE(stream.status(), QDataStream::Ok);
    QCOMPARE(elementType, ElementType::IC);
    QCOMPARE(icFileName, QString("custom_ic.panda"));
    QVERIFY(isEmbedded);
    QCOMPARE(embeddedName, QString("custom_ic.panda"));
}

void TestElementLabel::testMousePressThenSmallMoveDoesNotStartDrag()
{
    QPixmap pixmap = ElementPictures::typePicture(ElementType::And);
    ElementLabel label(pixmap, ElementType::And, "");

    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&label, &pressEvent);

    // A move under QApplication::startDragDistance() must return before starting a native
    // drag session (QDrag::exec() blocks pumping an event loop -- this must complete promptly).
    QMouseEvent smallMove(QEvent::MouseMove, QPointF(11, 10), QPointF(11, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&label, &smallMove);

    QCOMPARE(label.elementType(), ElementType::And);
}

void TestElementLabel::testMousePressThenLargeMoveStartsDrag()
{
    QPixmap pixmap = ElementPictures::typePicture(ElementType::And);
    ElementLabel label(pixmap, ElementType::And, "");

    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&label, &pressEvent);

    // A move past the drag-distance threshold reaches startDrag(), which builds mimeData()
    // and the drag pixmap/hotspot from the label's own state and hands off to QDrag::exec().
    // In this (offscreen-less, real-display) test environment exec() returns immediately since
    // there is no real button held down to sustain a drag session -- confirmed empirically to
    // complete promptly rather than block.
    const int farEnough = QApplication::startDragDistance() + 20;
    QMouseEvent largeMove(QEvent::MouseMove, QPointF(10 + farEnough, 10), QPointF(10 + farEnough, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&label, &largeMove);

    // startDrag() must not have thrown or corrupted the label's own state.
    QCOMPARE(label.elementType(), ElementType::And);
    QVERIFY(!label.pixmap().isNull());
}

void TestElementLabel::testMouseDoubleClickEmitsAddToSceneRequested()
{
    QPixmap pixmap = ElementPictures::typePicture(ElementType::And);
    ElementLabel label(pixmap, ElementType::And, "");
    QSignalSpy spy(&label, &ElementLabel::addToSceneRequested);

    QMouseEvent doubleClickEvent(QEvent::MouseButtonDblClick, QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&label, &doubleClickEvent);

    QCOMPARE(spy.count(), 1);
    auto *mimeData = qvariant_cast<QMimeData *>(spy.at(0).at(0));
    QVERIFY(mimeData != nullptr);
    delete mimeData; // addToSceneRequested hands off ownership to whoever receives it; nobody did here.
}
