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

    // Fall-through cascade: each case disables that level and all finer ones.
    // verbosity=5 → nothing disabled (all categories active);
    // verbosity=0 → all six categories disabled (silence everything);
    // negative / default → also disables all (used for production builds).
    switch (verbosity) {
    default:                         [[fallthrough]];
    case 0:  rules += "0 = false\n"; [[fallthrough]];
    case 1:  rules += "1 = false\n"; [[fallthrough]];
    case 2:  rules += "2 = false\n"; [[fallthrough]];
    case 3:  rules += "3 = false\n"; [[fallthrough]];
    case 4:  rules += "4 = false\n"; [[fallthrough]];
    case 5:  rules += "5 = false\n";
    }

    QLoggingCategory::setFilterRules(rules);

    // Include file line and function name in debug output for faster navigation.
    qSetMessagePattern("%{if-debug}%{line}: %{function} => %{endif}%{message}");
}

Pandaception::Pandaception(const QString &translatedMessage, const QString &englishMessage)
    // std::runtime_error carries the translated message so that what() shown in
    // the UI respects the current locale.  The English copy is kept separately
    // for crash-reporting backends (Sentry) that need a language-stable string.
    : std::runtime_error(translatedMessage.toStdString())
    , m_englishMessage(englishMessage)
{
}

QString Pandaception::englishMessage() const
{
    return m_englishMessage;
}

