// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Data-driven regression sweep: loads every bundled example .panda file via QuickWorkSpace and
/// asserts a clean, structurally-valid load plus a save/reload round-trip, walking the result
/// through CanvasItem's elements()/connections() accessors.
class TestFiles : public QObject
{
    Q_OBJECT

private slots:
    void testFiles_data();
    void testFiles();
    void testBackwardCompatibility_data();
    void testBackwardCompatibility();
};
