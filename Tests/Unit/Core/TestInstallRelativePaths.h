// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestInstallRelativePaths : public QObject
{
    Q_OBJECT

private slots:
    void testCandidatesFirstEntryIsAppDirCategory();
    void testCandidatesLastEntryIsBareCategoryFallback();
    void testCandidatesLinuxIncludesNativeFhsEntryInOrder();

    void testResolveReturnsEmptyWhenNoCandidateExists();
    void testResolveFindsCwdFallbackCandidate();
};
