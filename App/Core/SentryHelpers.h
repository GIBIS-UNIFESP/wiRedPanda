// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Lightweight Sentry helpers gated behind HAVE_SENTRY.
 */

#pragma once

#include <QString>

#ifdef HAVE_SENTRY
#include "App/Core/Settings.h"
#include "thirdparty/sentry/include/sentry.h"
#endif

/// Adds a Sentry breadcrumb. No-op when Sentry is not available, and no-op when the
/// user has switched crash reporting off -- in that case `sentry_init()` was never
/// called, so we return before allocating anything rather than handing values to an
/// uninitialised SDK.
/// \param category Short category string (e.g. "file", "simulation", "ui").
/// \param message  Human-readable description of the action.
inline void sentryBreadcrumb(const char *category, const QString &message)
{
#ifdef HAVE_SENTRY
    if (!Settings::crashReportingEnabled()) {
        return;
    }

    sentry_value_t crumb = sentry_value_new_breadcrumb("default", message.toStdString().c_str());
    sentry_value_set_by_key(crumb, "category", sentry_value_new_string(category));
    sentry_add_breadcrumb(crumb);
#else
    Q_UNUSED(category)
    Q_UNUSED(message)
#endif
}
