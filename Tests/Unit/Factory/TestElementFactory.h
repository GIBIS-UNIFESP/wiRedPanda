// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTest>

class TestElementFactory : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // Element Creation (data-driven + 2 additional tests)
    void testBuildElement_data();
    void testBuildElement();
    void testBuildElementInvalidType();
    void testMetadataLogicCreator();

    // Type Conversion (3 tests)
    void testTextToType_data();
    void testTextToType();
    void testTypeToTextBidirectional();

    // Registry Management (3 tests)
    void testItemRegistry();
    void testUniqueIdAssignment();
    void testMetadataRegistry();

    // Completeness Tests (1 test)
    void testAllElementTypesRegistered();
};

