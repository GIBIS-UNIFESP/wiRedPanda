// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestFileUtils : public QObject
{
    Q_OBJECT

private slots:

    // copyToDir tests (4 tests)
    void testCopyToDirEmptyPath();
    void testCopyToDirValidFile();
    void testCopyToDirFileAlreadyExists();
    void testCopyToDirResourcePath();

    // Regression: D17 — copy failures must throw
    void testCopyToDirThrowsOnFailure();

    // filesHaveSameContent — same-file vs name-collision detection
    void testFilesHaveSameContent();
};
