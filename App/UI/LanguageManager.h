// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief LanguageManager: Qt translation loading and language metadata.
 */

#pragma once

#include <QObject>
#include <QStringList>

class QTranslator;

/**
 * \class LanguageManager
 * \brief Owns the application QTranslator instances and provides language metadata.
 *
 * \details Encapsulates all translation concerns: discovering available .qm files,
 * loading/unloading QTranslator objects, and mapping language codes to display
 * names and flag icon resource paths. Emits translationChanged() after each
 * successful load so the UI can retranslate itself.
 */
class LanguageManager : public QObject
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit LanguageManager(QObject *parent = nullptr);
    ~LanguageManager() override;

    // --- Translation ---

    /**
     * \brief Loads and installs the Qt translation for \a language.
     * \details Falls back to English if the .qm file is missing or fails to load.
     *          Emits translationChanged() after the translators are installed.
     * \param language BCP-47 language code (e.g. "en", "pt_BR").
     */
    void loadTranslation(const QString &language);

    /// Returns all available translation language codes, sorted, with "en" first.
    QStringList availableLanguages() const;

    /**
     * \brief Returns the native display name for \a langCode (e.g. "Deutsch" for "de").
     * \param langCode BCP-47 language code.
     */
    QString displayName(const QString &langCode) const;

    /**
     * \brief Returns the flag icon resource path for \a langCode.
     * \param langCode BCP-47 language code.
     */
    QString flagIcon(const QString &langCode) const;

signals:
    /// Emitted after a translation has been successfully loaded (or reset to English).
    void translationChanged();

private:
    // --- Members ---

    QTranslator *m_pandaTranslator = nullptr; ///< Translator for the application's own .qm strings.
    QTranslator *m_qtTranslator    = nullptr; ///< Translator for Qt's own translatable strings.
};
