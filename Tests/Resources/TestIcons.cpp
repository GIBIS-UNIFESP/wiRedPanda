// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Resources/TestIcons.h"

#include <QMetaEnum>

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "Tests/Common/TestUtils.h"

void TestIcons::testIcons_data()
{
    QTest::addColumn<int>("typeValue");
    QTest::addColumn<QString>("typeName");

    // Iterate over all registered ElementType enum values
    // Using QMetaEnum ensures all enum values are automatically tested,
    // eliminating the need for magic numbers and guaranteeing correctness
    // even when new element types are added in the future
    const auto metaEnum = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        auto type = static_cast<ElementType>(metaEnum.value(i));

        // Skip types that should not have icons
        if ((type == ElementType::Unknown) ||
            (type == ElementType::JKLatch) ||  // DEPRECATED - backward compatibility only
            (type == ElementType::IC)) {       // Special element, no icon needed
            continue;
        }

        // Try to get the text representation - if it fails, this type is not valid
        const auto text = ElementFactory::typeToText(type);
        if (text.isEmpty() || text == "Unknown") {
            continue;  // Skip invalid types
        }

        QTest::newRow(qPrintable(text)) << static_cast<int>(type) << text;
    }
}

void TestIcons::testIcons()
{
    QFETCH(int, typeValue);
    QFETCH(QString, typeName);

    auto type = static_cast<ElementType>(typeValue);
    const auto pixmap = ElementFactory::pixmap(type);

    QVERIFY2(!pixmap.isNull(),
             qPrintable(QString("%1 (type=%2) pixmap not found").arg(typeName).arg(typeValue)));
}

