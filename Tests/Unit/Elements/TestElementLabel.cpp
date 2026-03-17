// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElementLabel.h"

#include <memory>

#include <QMimeData>
#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementLabel.h"
#include "Tests/Common/TestUtils.h"

// ============================================================================
// Constructor Tests
// ============================================================================

void TestElementLabel::testConstructorWithPixmapPointer()
{
    QPixmap pixmap = ElementFactory::pixmap(ElementType::And);
    ElementLabel label(pixmap, ElementType::And, "");

    QCOMPARE(label.elementType(), ElementType::And);
    QVERIFY(!label.pixmap().isNull());
}

void TestElementLabel::testConstructorWithPixmapReference()
{
    QPixmap pixmap = ElementFactory::pixmap(ElementType::Or);
    ElementLabel label(pixmap, ElementType::Or, "");

    QCOMPARE(label.elementType(), ElementType::Or);
    QVERIFY(!label.pixmap().isNull());
}

// ============================================================================
// Data Accessor Tests
// ============================================================================

void TestElementLabel::testElementType()
{
    QPixmap pixmap = ElementFactory::pixmap(ElementType::Nand);
    ElementLabel label(pixmap, ElementType::Nand, "");

    QCOMPARE(label.elementType(), ElementType::Nand);
}

void TestElementLabel::testPixmap()
{
    QPixmap pixmap = ElementFactory::pixmap(ElementType::And);
    ElementLabel label(pixmap, ElementType::And, "");

    QVERIFY(!label.pixmap().isNull());
    QCOMPARE(label.pixmap().size(), pixmap.size());
}

void TestElementLabel::testName()
{
    QPixmap pixmap = ElementFactory::pixmap(ElementType::Not);
    ElementLabel label(pixmap, ElementType::Not, "");

    QString name = label.name();
    QVERIFY(!name.isEmpty());
    // Name should be translated name of element
    QCOMPARE(name, ElementFactory::translatedName(ElementType::Not));
}

void TestElementLabel::testIcFileName()
{
    QPixmap pixmap = ElementFactory::pixmap(ElementType::And);
    QString fileName = "test_circuit.panda";
    ElementLabel label(pixmap, ElementType::And, fileName);

    QCOMPARE(label.icFileName(), fileName);
}

// ============================================================================
// Name Management Tests
// ============================================================================

void TestElementLabel::testUpdateNameStandardElement()
{
    QPixmap pixmap = ElementFactory::pixmap(ElementType::And);
    ElementLabel label(pixmap, ElementType::And, "");

    QString originalName = label.name();
    label.updateName();

    // Name should remain the same after update for standard elements
    QCOMPARE(label.name(), originalName);
    QCOMPARE(label.name(), ElementFactory::translatedName(ElementType::And));
}

void TestElementLabel::testUpdateNameICElement()
{
    QPixmap pixmap = ElementFactory::pixmap(ElementType::IC);
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
        QPixmap pixmap = ElementFactory::pixmap(type);
        ElementLabel label(pixmap, type, "");

        QString name = label.name();
        QVERIFY(!name.isEmpty());
        QCOMPARE(name, ElementFactory::translatedName(type));
    }
}

// ============================================================================
// Theme Management Tests
// ============================================================================

void TestElementLabel::testUpdateTheme()
{
    QPixmap pixmap = ElementFactory::pixmap(ElementType::And);
    ElementLabel label(pixmap, ElementType::And, "");

    QPixmap original = label.pixmap();
    QVERIFY2(!original.isNull(), "Initial pixmap should be valid");

    label.updateTheme();

    // After theme update, pixmap should still be valid
    QVERIFY2(!label.pixmap().isNull(), "Pixmap should remain valid after theme update");
}

void TestElementLabel::testPixmapAfterThemeUpdate()
{
    QPixmap pixmap = ElementFactory::pixmap(ElementType::Or);
    ElementLabel label(pixmap, ElementType::Or, "");

    label.updateTheme();

    // Pixmap should still have valid size
    QVERIFY(label.pixmap().size().width() > 0);
    QVERIFY(label.pixmap().size().height() > 0);
}

// ============================================================================
// MIME Data Tests
// ============================================================================

void TestElementLabel::testMimeData()
{
    QPixmap pixmap = ElementFactory::pixmap(ElementType::And);
    ElementLabel label(pixmap, ElementType::And, "");

    auto data = std::unique_ptr<QMimeData>(label.mimeData());
    QVERIFY(data != nullptr);

    // Should have wiredpanda-dragdrop format
    QVERIFY(data->hasFormat("application/x-wiredpanda-dragdrop"));
}

void TestElementLabel::testMimeDataContent()
{
    QPixmap pixmap = ElementFactory::pixmap(ElementType::Nand);
    ElementLabel label(pixmap, ElementType::Nand, "");

    auto data = std::unique_ptr<QMimeData>(label.mimeData());
    QVERIFY(data != nullptr);

    // MIME data should contain valid serialized information
    QByteArray mimeBytes = data->data("application/x-wiredpanda-dragdrop");
    QVERIFY(mimeBytes.size() > 0);
}

void TestElementLabel::testMimeDataWithIC()
{
    QPixmap pixmap = ElementFactory::pixmap(ElementType::IC);
    ElementLabel label(pixmap, ElementType::IC, "custom_ic.panda");

    auto data = std::unique_ptr<QMimeData>(label.mimeData());
    QVERIFY(data != nullptr);

    // Should contain the IC file information
    QByteArray mimeBytes = data->data("application/x-wiredpanda-dragdrop");
    QVERIFY(mimeBytes.size() > 0);
}
