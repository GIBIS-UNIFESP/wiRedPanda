// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestExerciseTourResources : public QObject
{
    Q_OBJECT

private slots:
    void testScanValidEntries();
    void testScanMissingDescriptionDefaultsToEmpty();
    void testScanSkipsMalformedJson();
    void testScanSkipsMissingIdOrTitle();
    void testScanNonExistentDirectoryReturnsEmpty();

    void testTranslateFromCatalogFoundKey();
    void testTranslateFromCatalogMissingKeyFallsBack();
    void testTranslateFromCatalogMalformedJsonFallsBack();
    void testTranslateFromCatalogMissingFileFallsBack();

    void testTranslateEnglishNeverTouchesDisk();
    void testTranslateEmptyLanguageNeverTouchesDisk();

    void testMergeUniqueMergesNonCollidingEntries();
    void testMergeUniqueSkipsCollidingId();

    void testManagedContentDirCreatesAndReturnsExistingPath();
    void testPreferredContentDirReturnsWritablePathOutsideManagedDir();

    void testResolveWritableDirFallsBackWhenCandidatesUnwritable();
    void testResolveWritableDirEmptyCandidateSkipped();
    void testResolveWritableDirReturnsEmptyWhenNothingWorks();

    void testScanUnreadableFileSkipped();
    void testManagedContentDirCreatesFreshDirectory();

    void testDiscoverMergesInstallRelativeDir();
    void testDiscoverMergesDocumentsFallbackDir();

    void testTranslateFromCatalogNonObjectMidPathFallsBack();
    void testTranslateNonEnglishReadsCatalog();
};
