// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Shared string utilities used by all code generators.
 */

#pragma once

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

}  // namespace CodeGenUtils

