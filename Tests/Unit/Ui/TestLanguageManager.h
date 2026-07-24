// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

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
