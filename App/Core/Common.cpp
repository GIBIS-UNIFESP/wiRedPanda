// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/Common.h"

Q_LOGGING_CATEGORY(zero,  "0")
Q_LOGGING_CATEGORY(one,   "1")
Q_LOGGING_CATEGORY(two,   "2")
Q_LOGGING_CATEGORY(three, "3")
Q_LOGGING_CATEGORY(four,  "4")
Q_LOGGING_CATEGORY(five,  "5")

void Comment::setVerbosity(const int verbosity)
{
    QString rules;

    // Category i carries detail level i (0 = coarsest). A given verbosity
    // enables categories 0..verbosity-1; verbosity 5 means "everything"
    // (category five included — the old fall-through cascade appended
    // "5 = false" in every case, so category five was unreachable). Values
    // above 5 behave like 5; negative / 0 (the production default) disable
    // all output.
    for (int i = 0; i <= 5; ++i) {
        const bool enabled = (i < 5) ? (verbosity > i) : (verbosity >= 5);
        rules += QString("%1 = %2\n").arg(i).arg(enabled ? "true" : "false");
    }

    QLoggingCategory::setFilterRules(rules);

    // Include file line and function name in debug output for faster navigation.
    qSetMessagePattern("%{if-debug}%{line}: %{function} => %{endif}%{message}");
}

Pandaception::Pandaception(const QString &translatedMessage, const QString &englishMessage,
                           const char *file, int line)
    // std::runtime_error carries the translated message so that what() shown in
    // the UI respects the current locale.  The English copy is kept separately
    // for crash-reporting backends (Sentry) that need a language-stable string.
    : std::runtime_error(translatedMessage.toStdString())
    , m_englishMessage(englishMessage)
    , m_file(file)
    , m_line(line)
{
}

// Key function — see header for rationale.  Must remain out-of-line and
// non-inline so the Itanium ABI emits Pandaception's typeinfo here only.
// The `= default` body is inlined at every throw's unwind (thousands of
// times across the suite) with no attributable standalone execution.
Pandaception::~Pandaception() = default; // LCOV_EXCL_LINE

QString Pandaception::englishMessage() const
{
    return m_englishMessage;
}

const char *Pandaception::file() const
{
    return m_file;
}

int Pandaception::line() const
{
    return m_line;
}
