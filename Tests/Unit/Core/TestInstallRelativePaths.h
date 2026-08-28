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

    /// isCandidate() recognises the bundled content directory so callers can refuse to write
    /// into it even where the filesystem allows it (a dev checkout's Examples/ is writable).
    void testIsCandidateMatchesCwdFallbackByCanonicalPath();
    /// Matching is by canonical path, so an equivalent spelling of the same directory
    /// (trailing "/.", a ".." round trip) still matches -- a plain string compare would not.
    void testIsCandidateMatchesEquivalentPathSpellings();
    /// A directory that isn't a candidate, doesn't exist, or is empty is never a match.
    void testIsCandidateRejectsUnrelatedMissingAndEmptyDirs();
};
