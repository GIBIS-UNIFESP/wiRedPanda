// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestElementFactory : public QObject
{
    Q_OBJECT

private slots:
    void testSingletonInstance();
    void testTextToType();
    void testTypeToText();
    void testTypeToTitleText();
    void testTranslatedName();
    void testPixmap();
    void testBuildElement();
    void testBuildLogicElement();
    void testItemManagement();
    void testAddItem();
    void testRemoveItem();
    void testUpdateItemId();
    void testItemById();
    void testContains();
    void testClearCache();
    void testGetStaticProperties();
    void testAllElementTypes();
    void testElementCreationAndDestruction();
    void testLogicElementMapping();
    void testInvalidElementTypes();
    void testPropertyCaching();
};