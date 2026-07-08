// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestICRegistry : public QObject
{
    Q_OBJECT

private slots:

    void testICRegistration();
    void testICFileWatcher();
    void testRecursiveICLoading();
    void testICValidation();

    // Regression: makeBlobSelfContained() only had cycle detection, no recursion depth
    // cap, unlike ICLoader's identical-purpose kMaxICNestingDepth guard.
    void testMakeBlobSelfContainedRejectsDeepDependencyChain();
};
