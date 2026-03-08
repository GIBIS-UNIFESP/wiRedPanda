// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/LanguageManager.h"

#include <QDir>
#include <QLocale>
#include <QResource>
#include <QTranslator>

#include "App/Core/Application.h"
#include "App/Core/Settings.h"

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent)
{
}

LanguageManager::~LanguageManager()
{
    // Remove translators from Application::instance() before they are deleted by the QObject parent tree.
    Application::instance()->removeTranslator(m_pandaTranslator);
    Application::instance()->removeTranslator(m_qtTranslator);
}

void LanguageManager::loadTranslation(const QString &language)
{
    if (language.isEmpty()) {
        return;
    }

    Settings::setLanguage(language);

    // Always recreate translators rather than re-loading; Qt does not guarantee
    // that calling load() on an existing translator re-emits languageChanged.
    Application::instance()->removeTranslator(m_pandaTranslator);
    Application::instance()->removeTranslator(m_qtTranslator);

    delete m_pandaTranslator;
    delete m_qtTranslator;

    m_pandaTranslator = nullptr;
    m_qtTranslator    = nullptr;

    if (language == "en") {
        emit translationChanged();
        return;
    }

    const QString pandaFile = QString(":/Translations/wpanda_%1.qm").arg(language);
    const QString qtFile    = QString(":/Translations/qt_%1.qm").arg(language);

    if (QResource(pandaFile).isValid()) {
        m_pandaTranslator = new QTranslator(this);

        if (!m_pandaTranslator->load(pandaFile) || !Application::instance()->installTranslator(m_pandaTranslator)) {
            qWarning() << "Failed to load wiRedPanda translation for" << language << ", falling back to English";
            delete m_pandaTranslator;
            m_pandaTranslator = nullptr;
            loadTranslation("en");
            return;
        }
    }

    if (QResource(qtFile).isValid()) {
        m_qtTranslator = new QTranslator(this);

        if (!m_qtTranslator->load(qtFile) || !Application::instance()->installTranslator(m_qtTranslator)) {
            qWarning() << "Failed to load Qt translation for" << language << ", continuing without Qt translation";
            delete m_qtTranslator;
            m_qtTranslator = nullptr;
        }
    }

    emit translationChanged();
}

QStringList LanguageManager::availableLanguages() const
{
    QStringList languages{"en"};

    // Qt's resource system exposes ":/Translations" as a virtual directory.
    QDir translationsDir(":/Translations");
    if (translationsDir.exists()) {
        const QStringList qmFiles = translationsDir.entryList({"wpanda_*.qm"}, QDir::Files);

        for (const QString &file : qmFiles) {
            QString langCode = file;
            langCode.remove("wpanda_");
            langCode.remove(".qm");

            if (!langCode.isEmpty() && QResource(QString(":/Translations/%1").arg(file)).isValid()) {
                languages << langCode;
            }
        }
    } else {
        // Fallback: probe a known set of language codes when directory listing fails.
        static const QStringList kKnownLanguages = {
            "ar", "bg", "bn", "cs", "da", "de", "el", "es", "et", "fa", "fi", "fr",
            "he", "hi", "hr", "hu", "id", "it", "ja", "ko", "lt", "lv", "ms", "nb",
            "nl", "pl", "pt", "pt_BR", "ro", "ru", "sk", "sv", "th", "tr", "uk",
            "vi", "zh_Hans", "zh_Hant"
        };

        for (const QString &langCode : kKnownLanguages) {
            if (QResource(QString(":/Translations/wpanda_%1.qm").arg(langCode)).isValid()) {
                languages << langCode;
            }
        }
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
#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
        const QString country = locale.nativeCountryName();
#else
        const QString country = locale.nativeTerritoryName();
#endif
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
