// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestRecentFiles : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // ============================================================
    // RecentFiles Tests (5 tests)
    // ============================================================
    void testAddFile();
    void testDuplicateHandling();
    void testSizeLimit();
    void testNonExistentFile();
    void testPersistence();
};
