// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestElementLabel : public QObject
{
    Q_OBJECT

private slots:
    // Constructor tests
    void testConstructorWithPixmapPointer();
    void testConstructorWithPixmapReference();

    // Data accessor tests
    void testElementType();
    void testPixmap();
    void testName();
    void testIcFileName();

    // Name management tests
    void testUpdateNameStandardElement();
    void testUpdateNameICElement();
    void testNameForDifferentElementTypes();

    // Theme management tests
    void testUpdateTheme();
    void testPixmapAfterThemeUpdate();

    // MIME data tests
    void testMimeData();
    void testMimeDataContent();
    void testMimeDataWithIC();
};
