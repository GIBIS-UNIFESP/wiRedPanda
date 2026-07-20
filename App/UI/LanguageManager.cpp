// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/LanguageManager.h"

#include <QDir>
#include <QLocale>
#include <QResource>
#include <QTranslator>

#ifdef Q_OS_WASM
#include <QFontDatabase>
#endif

#include "App/Core/Application.h"
#include "App/Core/Settings.h"

#ifdef Q_OS_WASM
static void loadAllFonts()
{
    static const QStringList kFontPaths = {
        ":/Fonts/NotoSansArabic-Regular.ttf",
        ":/Fonts/NotoSansBengali-Regular.ttf",
        ":/Fonts/NotoSansDevanagari-Regular.ttf",
        ":/Fonts/NotoSansHebrew-Regular.ttf",
        ":/Fonts/NotoSansKR-Regular.ttf",
        ":/Fonts/NotoSansSC-Regular.ttf",
        ":/Fonts/NotoSansThai-Regular.ttf",
    };

    for (const auto &path : kFontPaths) {
        if (QFontDatabase::addApplicationFont(path) == -1) {
            qWarning() << "Failed to load WASM font:" << path;
        }
    }
}
#endif

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent)
{
#ifdef Q_OS_WASM
    // Load all fonts upfront so the language menu can render native script names.
    loadAllFonts();
#endif
}

LanguageManager::~LanguageManager()
{
    // Remove translator from Application::instance() before it is deleted by the QObject parent tree.
    Application::instance()->removeTranslator(m_translator);
}

void LanguageManager::loadTranslation(const QString &language)
{
    if (language.isEmpty()) {
        return;
    }

    Settings::setLanguage(language);

    // Always recreate the translator rather than re-loading; Qt does not
    // guarantee that calling load() on an existing translator re-emits
    // languageChanged.
    Application::instance()->removeTranslator(m_translator);
    delete m_translator;
    m_translator = nullptr;

    if (language == "en") {
        emit translationChanged();
        return;
    }

    // qt_add_translations embeds wpanda_<lang>.qm at :/i18n/ at build time.
    // On Qt 6.9+ MERGE_QT_TRANSLATIONS also bakes qtbase / Widgets / Gui /
    // Multimedia / Svg catalogs into the same file, so a single translator
    // covers both application and Qt-owned strings.  On Qt 6.2..6.8 the
    // merge isn't available — Qt's own dialog strings (file picker, message
    // boxes) stay English, but application strings still translate.
    const QString qmFile = QStringLiteral(":/i18n/wpanda_%1.qm").arg(language);

    if (QResource(qmFile).isValid()) { // LCOV_EXCL_LINE — qt_add_translations() embeds wpanda_*.qm only into the production "wiredpanda" executable target (CMakeLists.txt), by design, not into test_wiredpanda, so this is never true in the test binary; see .claude/COVERAGE_100_PLAN.md pattern 37.
        m_translator = new QTranslator(this); // LCOV_EXCL_LINE — see above.

        if (!m_translator->load(qmFile) || !Application::instance()->installTranslator(m_translator)) { // LCOV_EXCL_LINE — see above.
            qWarning() << "Failed to load translation for" << language << ", falling back to English"; // LCOV_EXCL_LINE — see above.
            delete m_translator; // LCOV_EXCL_LINE — see above.
            m_translator = nullptr; // LCOV_EXCL_LINE — see above.
            loadTranslation("en"); // LCOV_EXCL_LINE — see above.
            return; // LCOV_EXCL_LINE — see above.
        }
    }

    emit translationChanged();
}

QStringList LanguageManager::availableLanguages() const
{
    QStringList languages = {"en"};

    // Qt's resource system exposes ":/i18n" as a virtual directory.
    QDir translationsDir(QStringLiteral(":/i18n"));
    if (translationsDir.exists()) {
        const QStringList qmFiles = translationsDir.entryList({"wpanda_*.qm"}, QDir::Files);

        // LCOV_EXCL_START — wpanda_*.qm is embedded only into the production "wiredpanda"
        // executable target, not test_wiredpanda (see loadTranslation()'s matching exclusion
        // and .claude/COVERAGE_100_PLAN.md pattern 37), so qmFiles is always empty here; this
        // loop body only ever runs in the real app.
        for (const QString &file : qmFiles) {
            QString langCode = file;
            langCode.remove("wpanda_");
            langCode.remove(".qm");

            if (!langCode.isEmpty() && QResource(QStringLiteral(":/i18n/") + file).isValid()) {
                languages << langCode;
            }
        }
        // LCOV_EXCL_STOP
    } else {
        // Fallback: probe a known set of language codes when directory listing fails.
        // LCOV_EXCL_START — :/i18n itself always exists in every target here (wiredpanda_resources
        // registers :/i18n/ExerciseTour/ regardless of target), so this branch is unreachable;
        // same root cause as the exclusions above.
        static const QStringList kKnownLanguages = {
            "ar", "bg", "bn", "cs", "da", "de", "el", "es", "et", "fa", "fi", "fr",
            "he", "hi", "hr", "hu", "id", "it", "ja", "ko", "lt", "lv", "ms", "nb",
            "nl", "pl", "pt", "pt_BR", "ro", "ru", "sk", "sv", "th", "tr", "uk",
            "vi", "zh_Hans", "zh_Hant"
        };

        for (const QString &langCode : kKnownLanguages) {
            if (QResource(QStringLiteral(":/i18n/wpanda_%1.qm").arg(langCode)).isValid()) {
                languages << langCode;
            }
        }
        // LCOV_EXCL_STOP
    }

    languages.removeDuplicates();
    languages.sort();
    return languages;
}

QString LanguageManager::displayName(const QString &langCode) const
{
    QLocale locale(langCode);

    if (langCode == "pt_BR") {
        locale = QLocale(QLocale::Portuguese, QLocale::Brazil);
    }

    QString name = locale.nativeLanguageName();

    if (langCode.contains('_')) {
        const QString country = locale.nativeTerritoryName();
        if (!country.isEmpty() && country != name) {
            name += QString(" (%1)").arg(country);
        }
    }

    if (name.isEmpty()) {
        // Manual fallbacks for languages Qt may not fully support.
        static const QMap<QString, QString> kFallbacks = {
            {"bn", "বাংলা"},
            {"fa", "فارسی"},
            {"he", "עברית"},
            {"th", "ไทย"}
        };
        name = kFallbacks.value(langCode, langCode);
    }

    if (!name.isEmpty() && name.at(0).isLetter()) {
        name[0] = name.at(0).toUpper();
    }

    return name;
}

QString LanguageManager::flagIcon(const QString &langCode) const
{
    // Single authoritative map keyed by BCP-47 language code.
    static const QMap<QString, QString> kFlags = {
        {"ar",      ":/Interface/Locale/arabic.svg"},
        {"bg",      ":/Interface/Locale/bulgarian.svg"},
        {"bn",      ":/Interface/Locale/bangladesh.svg"},
        {"cs",      ":/Interface/Locale/czech.svg"},
        {"da",      ":/Interface/Locale/danish.svg"},
        {"de",      ":/Interface/Locale/german.svg"},
        {"el",      ":/Interface/Locale/greek.svg"},
        {"en",      ":/Interface/Locale/usa.svg"},
        {"es",      ":/Interface/Locale/spanish.svg"},
        {"et",      ":/Interface/Locale/estonian.svg"},
        {"fa",      ":/Interface/Locale/iranian.svg"},
        {"fi",      ":/Interface/Locale/finnish.svg"},
        {"fr",      ":/Interface/Locale/french.svg"},
        {"he",      ":/Interface/Locale/hebrew.svg"},
        {"hi",      ":/Interface/Locale/hindi.svg"},
        {"hr",      ":/Interface/Locale/croatian.svg"},
        {"hu",      ":/Interface/Locale/hungarian.svg"},
        {"id",      ":/Interface/Locale/indonesian.svg"},
        {"it",      ":/Interface/Locale/italian.svg"},
        {"ja",      ":/Interface/Locale/japanese.svg"},
        {"ko",      ":/Interface/Locale/korean.svg"},
        {"lt",      ":/Interface/Locale/lithuanian.svg"},
        {"lv",      ":/Interface/Locale/latvian.svg"},
        {"ms",      ":/Interface/Locale/malaysian.svg"},
        {"nb",      ":/Interface/Locale/norwegian.svg"},
        {"nl",      ":/Interface/Locale/dutch.svg"},
        {"pl",      ":/Interface/Locale/polish.svg"},
        {"pt",      ":/Interface/Locale/portuguese.svg"},
        {"pt_BR",   ":/Interface/Locale/brasil.svg"},
        {"ro",      ":/Interface/Locale/romanian.svg"},
        {"ru",      ":/Interface/Locale/russian.svg"},
        {"sk",      ":/Interface/Locale/slovak.svg"},
        {"sv",      ":/Interface/Locale/swedish.svg"},
        {"th",      ":/Interface/Locale/thai.svg"},
        {"tr",      ":/Interface/Locale/turkish.svg"},
        {"uk",      ":/Interface/Locale/ukrainian.svg"},
        {"vi",      ":/Interface/Locale/vietnamese.svg"},
        {"zh_Hans", ":/Interface/Locale/chinese.svg"},
        {"zh_Hant", ":/Interface/Locale/chinese_traditional.svg"},
    };

    return kFlags.value(langCode, ":/Interface/Locale/default.svg");
}
