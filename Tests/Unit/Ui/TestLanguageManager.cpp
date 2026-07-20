// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestLanguageManager.h"

#include <QDir>
#include <QFile>
#include <QLibraryInfo>
#include <QSignalSpy>
#include <QTranslator>

#include "App/Core/Settings.h"
#include "App/UI/LanguageManager.h"
#include "Tests/Common/TestUtils.h"

void TestLanguageManager::testAvailableLanguages()
{
    // availableLanguages() always includes the hardcoded "en" fallback even when the ":/i18n"
    // resource directory is empty (LanguageManager.cpp), so !isEmpty() alone can never catch a
    // regression that silently drops the ~39 real wpanda_*.qm translations from the embedded
    // resources. Those are embedded only via qt_add_translations(wiredpanda ...)
    // (CMakeLists.txt), scoped to the "wiredpanda" executable target specifically — not
    // available in this test binary — so skip rather than fail when only the fallback is
    // found, the same pattern the Qt-translations tests below already use.
    LanguageManager manager;
    QStringList languages = manager.availableLanguages();
    QVERIFY(languages.contains("en"));
    if (languages.size() == 1) {
        QSKIP("Only the hardcoded \"en\" fallback was found — wpanda_*.qm translations are "
              "embedded only in the \"wiredpanda\" executable target, not in this test binary");
    }
    QVERIFY(languages.contains("de"));
    QVERIFY(languages.contains("pt_BR"));
}

void TestLanguageManager::testSetLanguage()
{
    // loadTranslation() documents that it "Emits translationChanged() after the translators
    // are installed" (LanguageManager.h) — verify that actually happens.
    LanguageManager manager;
    QSignalSpy translationChangedSpy(&manager, &LanguageManager::translationChanged);

    manager.loadTranslation("en");

    // loadTranslation persists the choice and notifies listeners so the UI retranslates.
    QCOMPARE(Settings::language(), QStringLiteral("en"));
    QCOMPARE(translationChangedSpy.count(), 1);
}

void TestLanguageManager::testQtTranslationsPathExists()
{
    // QLibraryInfo::TranslationsPath resolves relative to the running executable
    // via qt.conf. In a deployed app (windeployqt / macdeployqt / AppImage) this
    // directory is populated with qt_*.qm files; in a plain build tree it is not.
    // Skip rather than fail so regular CI builds stay green.
    const QString path = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if (!QDir(path).exists()) {
        QSKIP(qPrintable(QStringLiteral("Qt translations path not present in this build (expected in deployed app): ") + path));
    }
}

void TestLanguageManager::testQtTranslationFilesExist()
{
    // Qt 6 ships qtbase_<lang>.qm with real content; qt_<lang>.qm are stub wrappers
    // that windeployqt merges, or that our deploy script replaces with real content.
    // Spot-check two representative languages to confirm the deployed package is complete.
    const QString path = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if (!QDir(path).exists()) {
        QSKIP(qPrintable(QStringLiteral("Qt translations path not present in this build (expected in deployed app): ") + path));
    }
    QVERIFY2(QFile::exists(path + QStringLiteral("/qtbase_de.qm")),
             qPrintable(QStringLiteral("qtbase_de.qm not found in Qt translations path: ") + path));
    QVERIFY2(QFile::exists(path + QStringLiteral("/qtbase_pt_BR.qm")),
             qPrintable(QStringLiteral("qtbase_pt_BR.qm not found in Qt translations path: ") + path));
}

void TestLanguageManager::testQtTranslatorIsLoadable()
{
    // Load qtbase_de directly (not the stub qt_de) to verify the path contains
    // real translation content and that QTranslator can read it.
    const QString path = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if (!QDir(path).exists()) {
        QSKIP(qPrintable(QStringLiteral("Qt translations path not present in this build (expected in deployed app): ") + path));
    }
    QTranslator translator;
    QVERIFY2(translator.load(QStringLiteral("qtbase_de"), path),
             qPrintable(QStringLiteral("QTranslator::load(qtbase_de) failed from: ") + path));
    QVERIFY2(!translator.isEmpty(),
             "Loaded Qt German translator is empty — translation file may be a stub");
}

void TestLanguageManager::testDisplayNamePtBrUsesPortugueseBrazilWithCountrySuffix()
{
    // pt_BR gets its own QLocale(Portuguese, Brazil) override (LanguageManager.cpp) rather
    // than the generic QLocale(langCode) construction, and its underscore also triggers the
    // "(territory)" suffix -- covers both branches at once.
    LanguageManager manager;
    QCOMPARE(manager.displayName("pt_BR"), QStringLiteral("Português (Brasil)"));
}

void TestLanguageManager::testDisplayNameWithCountrySuffixAppendsTerritory()
{
    // A non-pt_BR code with an underscore still gets the "(territory)" suffix, proving that
    // branch isn't special-cased to pt_BR specifically.
    LanguageManager manager;
    QCOMPARE(manager.displayName("zh_Hans"), QStringLiteral("简体中文 (中国)"));
}

void TestLanguageManager::testDisplayNameFallsBackToRawCodeForUnknownLanguage()
{
    // "zz" isn't a real language QLocale recognises (nativeLanguageName() degrades to empty,
    // confirmed empirically), exercising the manual-fallback map's default: langCode itself,
    // capitalized. The four hand-picked fallback entries (bn/fa/he/th) are all now fully
    // supported by this Qt version's own locale data (also confirmed empirically), so they
    // can't be reached this way any more -- this proves the fallback mechanism itself still
    // works for whatever QLocale gap prompted it originally.
    LanguageManager manager;
    QCOMPARE(manager.displayName("zz"), QStringLiteral("Zz"));
}

void TestLanguageManager::testLoadTranslationForNonEnglishLanguageWithoutEmbeddedResource()
{
    // wpanda_de.qm isn't embedded in this test binary (see testAvailableLanguages()), so
    // QResource(qmFile).isValid() is false here and loadTranslation() falls straight through
    // to translationChanged() without installing anything -- still real behavior worth
    // pinning, since every previous test only ever passed "en" or an empty string.
    LanguageManager manager;
    QSignalSpy translationChangedSpy(&manager, &LanguageManager::translationChanged);

    manager.loadTranslation("de");

    QCOMPARE(Settings::language(), QStringLiteral("de"));
    QCOMPARE(translationChangedSpy.count(), 1);
}

void TestLanguageManager::testFlagIconReturnsMappedIconForKnownLanguage()
{
    LanguageManager manager;
    QCOMPARE(manager.flagIcon("de"), QStringLiteral(":/Interface/Locale/german.svg"));
    QCOMPARE(manager.flagIcon("pt_BR"), QStringLiteral(":/Interface/Locale/brasil.svg"));
}

void TestLanguageManager::testFlagIconFallsBackToDefaultForUnknownLanguage()
{
    LanguageManager manager;
    QCOMPARE(manager.flagIcon("zz"), QStringLiteral(":/Interface/Locale/default.svg"));
}
