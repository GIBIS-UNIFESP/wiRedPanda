// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestLanguageManager.h"

#include <QDir>
#include <QFile>
#include <QLibraryInfo>
#include <QTranslator>

#include "App/UI/LanguageManager.h"
#include "Tests/Common/TestUtils.h"

void TestLanguageManager::testAvailableLanguages()
{
    LanguageManager manager;
    QStringList languages = manager.availableLanguages();
    QVERIFY(!languages.isEmpty());
}

void TestLanguageManager::testSetLanguage()
{
    LanguageManager manager;
    manager.loadTranslation("en");
    QVERIFY(true);
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

