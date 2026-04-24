// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Common logging utilities, the Pandaception error type, and helper macros.
 */

#pragma once

#include <stdexcept>

#include <QCoreApplication>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(zero)
Q_DECLARE_LOGGING_CATEGORY(one)
Q_DECLARE_LOGGING_CATEGORY(two)
Q_DECLARE_LOGGING_CATEGORY(three)
Q_DECLARE_LOGGING_CATEGORY(four)
Q_DECLARE_LOGGING_CATEGORY(five)

#undef qCDebug // to add noquote() and nospace()

#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
#define qCDebug(category) \
    for (bool qt_category_enabled = category().isDebugEnabled(); qt_category_enabled; qt_category_enabled = false) \
        QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC, category().categoryName()).debug().noquote().nospace()
#else
#define qCDebug(category) QT_MESSAGE_LOGGER_COMMON(category, QtDebugMsg).debug().noquote().nospace()
#endif

/**
 * \class Comment
 * \brief Controls the verbosity level of diagnostic logging categories.
 */
class Comment
{
public:
    /**
     * \brief Sets the active logging verbosity level.
     * \param verbosity Level 0–5 corresponds to logging categories zero–five.
     */
    static void setVerbosity(const int verbosity);
};

/**
 * \class Pandaception
 * \brief Exception type for user-facing circuit errors.
 *
 * \details Pandaception carries both a translated message (for display in the UI)
 * and the original English message (for logging and Sentry reports).
 * Throw it via the PANDACEPTION() macro to take advantage of Qt's tr() mechanism.
 */
class Pandaception : public std::runtime_error
{
public:
    // --- Lifecycle ---

    /**
     * \brief Constructs the exception with throw-site location for Sentry.
     * \param translatedMessage Localized message shown to the user.
     * \param englishMessage    English message for logging/Sentry.
     * \param file              Source file where the exception was thrown (__FILE__).
     * \param line              Source line where the exception was thrown (__LINE__).
     */
    explicit Pandaception(const QString &translatedMessage, const QString &englishMessage,
                          const char *file = nullptr, int line = 0);

    // --- Message access ---

    /// Returns the English (non-translated) message for logging and crash reports.
    QString englishMessage() const;

    /// Returns the source file where the exception was thrown, or nullptr.
    const char *file() const;

    /// Returns the source line where the exception was thrown, or 0.
    int line() const;

private:
    QString m_englishMessage; ///< English message preserved for logging.
    const char *m_file = nullptr; ///< Source file at throw site.
    int m_line = 0; ///< Source line at throw site.
};

// Main macro for translatable exceptions
#ifdef USE_KDE_FRAMEWORKS
#  define PANDACEPTION(msg, ...) \
    Pandaception(i18n(msg __VA_OPT__(, __VA_ARGS__)), QString(msg) __VA_OPT__(.arg(__VA_ARGS__)), __FILE__, __LINE__)
#  define PANDACEPTION_WITH_CONTEXT(context, msg, ...) \
    Pandaception(i18nc(context, msg __VA_OPT__(, __VA_ARGS__)), QString(msg) __VA_OPT__(.arg(__VA_ARGS__)), __FILE__, __LINE__)
#else
#  define PANDACEPTION(msg, ...) \
    Pandaception(tr(msg) __VA_OPT__(.arg(__VA_ARGS__)), QString(msg) __VA_OPT__(.arg(__VA_ARGS__)), __FILE__, __LINE__)
#  define PANDACEPTION_WITH_CONTEXT(context, msg, ...) \
    Pandaception(QCoreApplication::translate(context, msg) __VA_OPT__(.arg(__VA_ARGS__)), QString(msg) __VA_OPT__(.arg(__VA_ARGS__)), __FILE__, __LINE__)
#endif

