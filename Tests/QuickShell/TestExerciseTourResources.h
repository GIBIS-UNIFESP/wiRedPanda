// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// ExerciseTourResources (App/Core/ExerciseTourResources.h) is pure, portable Core logic with
/// zero Scene/CanvasItem/Widgets dependency; see CLAUDE.md for its
/// discover()/preferredContentDir()/managedContentDir() merge logic. Mirrors
/// Tests/Unit/Core/TestExerciseTourResources.h/.cpp (same class, same tests).
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
    void testTranslateFromCatalogEmptyStringValueFallsBack();
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
    void testDiscoverMergesManagedContentDir();

    void testTranslateFromCatalogNonObjectMidPathFallsBack();
    void testTranslateNonEnglishReadsCatalog();
};
