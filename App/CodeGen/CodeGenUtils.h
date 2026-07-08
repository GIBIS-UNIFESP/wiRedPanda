// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Shared string utilities used by all code generators.
 */

#pragma once

#include <algorithm>

#include <QRegularExpression>
#include <QString>

namespace CodeGenUtils {

/// Strips Unicode diacritic marks (accents) from \a input using NFC → NFD decomposition.
inline QString stripAccents(const QString &input)
{
    QString normalized = input.normalized(QString::NormalizationForm_D);
    static const QRegularExpression diacriticMarks("[\\p{Mn}]");
    return normalized.remove(diacriticMarks);
}

/**
 * \brief Converts \a input into a legal language identifier.
 *
 * Lowercases, trims whitespace, replaces spaces and dashes with underscores,
 * removes all remaining non-word characters, prepends '_' if the result starts
 * with a digit, and returns "_unnamed" for empty results.
 *
 * \param input       The raw string to sanitise.
 * \param stripFirst  If \c true, accent-strips \a input before processing
 *                    (required for Arduino C++ identifiers; not needed for SystemVerilog).
 */
inline QString removeForbiddenChars(const QString &input, const bool stripFirst = false)
{
    QString result = (stripFirst ? stripAccents(input) : input).toLower().trimmed().replace(' ', '_').replace('-', '_');
    result.erase(std::remove_if(result.begin(), result.end(),
                                [](const QChar c) { return !QChar::isLetterOrNumber(c.unicode()) && c != QLatin1Char('_'); }),
                 result.end());

    if (result.isEmpty()) {
        result = "_unnamed";
    } else if (result[0].isDigit()) {
        result.prepend('_');
    }

    return result;
}

/**
 * \brief Makes \a input safe to embed in a single-line "//" comment.
 *
 * Neutralizes embedded line breaks. Without this, a label containing "\n" (settable via a
 * crafted .panda file's label field, or the MCP create_element/set_element_properties "label"
 * parameter — neither restricts content beyond length) could break out of the comment and
 * inject an arbitrary line into the generated Arduino/SystemVerilog file.
 */
inline QString sanitizeComment(const QString &input)
{
    QString result = input;
    result.replace(QLatin1Char('\r'), QLatin1Char(' '));
    result.replace(QLatin1Char('\n'), QLatin1Char(' '));
    return result;
}

}  // namespace CodeGenUtils
