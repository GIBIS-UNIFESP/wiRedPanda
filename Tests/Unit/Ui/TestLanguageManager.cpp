// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestLanguageManager.h"

#include <QDir>
#include <QFile>
#include <QLibraryInfo>
#include <QSignalSpy>
#include <QTranslator>

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
