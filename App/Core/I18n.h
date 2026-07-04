// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief KDE i18n() / non-KDE compat shim.
 */

#pragma once

#ifdef USE_KDE_FRAMEWORKS
#include <KLocalizedString>
#else
#include <QCoreApplication>
#include <QString>

// Bridge: i18n(text, args...) on non-KDE builds.
// Falls back to QCoreApplication::translate with empty context; .qm translations
// that rely on class context won't resolve, but this is acceptable on the porting branch.
template<typename... Args>
[[nodiscard]] inline QString i18n(const char *text, const Args &... args)
{
    QString s = QCoreApplication::translate("", text);
    if constexpr (sizeof...(args) > 0) {
        (..., (s = s.arg(args)));
    }
    return s;
}
#endif
