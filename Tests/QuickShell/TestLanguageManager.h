// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// LanguageManager (App/UI/LanguageManager.h) is pure, portable logic -- zero
/// Scene/CanvasItem/Widgets dependency. Verbatim copy of the Widgets-side
/// Tests/Unit/Ui/TestLanguageManager.h/.cpp -- same class, same tests, only the #include path
/// for its own header changed.
class TestLanguageManager : public QObject
{
    Q_OBJECT

private slots:

    void testAvailableLanguages();
    void testSetLanguage();
    void testQtTranslationsPathExists();
    void testQtTranslationFilesExist();
    void testQtTranslatorIsLoadable();

    void testDisplayNamePtBrUsesPortugueseBrazilWithCountrySuffix();
    void testDisplayNameWithCountrySuffixAppendsTerritory();
    void testDisplayNameFallsBackToRawCodeForUnknownLanguage();

    void testLoadTranslationForNonEnglishLanguageWithoutEmbeddedResource();
    void testFlagIconReturnsMappedIconForKnownLanguage();
    void testFlagIconFallsBackToDefaultForUnknownLanguage();
};
