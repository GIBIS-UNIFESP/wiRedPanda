// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel1JKFlipFlop : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testResetAndSet();
    void testHold();
    void testToggle();
    void testAsyncPreset();
    void testAsyncClear();
    void testPresetClearOverrideClock();
};
