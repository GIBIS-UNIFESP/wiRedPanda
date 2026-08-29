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

/// Records the shape of the circuit currently being edited, so a report says what was on
/// screen instead of leaving it to be inferred from breadcrumbs. Deliberately counts and
/// flags only -- no names, labels or paths, nothing derived from the user's content.
///
/// Unlike a fingerprint, this belongs on the GLOBAL scope: it describes current state and
/// must still be attached to a crash minidump assembled long after this call returns.
inline void sentryCircuitContext(int elements, int ics, bool simulationRunning)
{
#ifdef HAVE_SENTRY
    if (!Settings::crashReportingEnabled()) {
        return;
    }

    sentry_value_t circuit = sentry_value_new_object();
    sentry_value_set_by_key(circuit, "elements", sentry_value_new_int32(elements));
    sentry_value_set_by_key(circuit, "ics", sentry_value_new_int32(ics));
    sentry_value_set_by_key(circuit, "simulation_running", sentry_value_new_bool(simulationRunning));
    sentry_set_context("circuit", circuit);
#else
    Q_UNUSED(elements)
    Q_UNUSED(ics)
    Q_UNUSED(simulationRunning)
#endif
}

#ifdef HAVE_SENTRY
namespace SentryDetail {
/// The single circuit attachment currently registered, and the path it points at.
/// One attachment rather than one per tab: the most recently autosaved circuit is the one
/// being actively edited, and it keeps the upload bounded against the project's attachment
/// quota. The path is kept so a workspace tearing down can only detach *its own*.
inline sentry_attachment_t *circuitAttachment = nullptr;
inline QString circuitAttachmentPath;
} // namespace SentryDetail
#endif

/// Detaches the circuit attachment if it currently points at \a absolutePath. Passing an
/// empty path detaches unconditionally, which is what consent withdrawal needs.
inline void sentryDetachCircuit(const QString &absolutePath = QString())
{
#ifdef HAVE_SENTRY
    if (SentryDetail::circuitAttachment == nullptr) {
        return;
    }
    if (!absolutePath.isEmpty() && SentryDetail::circuitAttachmentPath != absolutePath) {
        return; // Belongs to a different workspace; leave it alone.
    }

    sentry_remove_attachment(SentryDetail::circuitAttachment);
    SentryDetail::circuitAttachment = nullptr;
    SentryDetail::circuitAttachmentPath.clear();
#else
    Q_UNUSED(absolutePath)
#endif
}

/// Attaches the circuit at \a absolutePath to future crash reports, replacing any previous
/// one. No-op when the user has switched crash reporting off -- this uploads their work.
///
/// The SDK reads attachments lazily, at crash time, from the path. That is why the
/// autosave is the right target rather than the user's saved .panda: the autosave tracks
/// the in-memory circuit, whereas the saved file would upload the last *saved* state,
/// which is not what crashed, and would be missing entirely for a never-saved circuit.
inline void sentryAttachCircuit(const QString &absolutePath)
{
#ifdef HAVE_SENTRY
    if (!Settings::crashReportingEnabled() || absolutePath.isEmpty()) {
        return;
    }
    if (SentryDetail::circuitAttachmentPath == absolutePath) {
        return; // Already attached; the SDK re-reads the file at crash time anyway.
    }

    sentryDetachCircuit();

    sentry_attachment_t *attachment = sentry_attach_file(absolutePath.toStdString().c_str());
    if (attachment == nullptr) {
        return;
    }

    // Sentry derives the displayed name from the path, and an autosave lives either in
    // AppData or beside the user's project -- both of which carry their home directory,
    // their real name on Windows. Override it so the upload identifies the file without
    // describing where it came from.
    sentry_attachment_set_filename(attachment, "circuit.panda");

    SentryDetail::circuitAttachment = attachment;
    SentryDetail::circuitAttachmentPath = absolutePath;
#else
    Q_UNUSED(absolutePath)
#endif
}

/// Records how many tabs are open and which one is active. Kept separate from the circuit
/// context because it is owned by a different layer and changes on a different cadence.
inline void sentryWorkspaceContext(int tabCount, int currentTab)
{
#ifdef HAVE_SENTRY
    if (!Settings::crashReportingEnabled()) {
        return;
    }

    sentry_value_t workspace = sentry_value_new_object();
    sentry_value_set_by_key(workspace, "tabs", sentry_value_new_int32(tabCount));
    sentry_value_set_by_key(workspace, "current_tab", sentry_value_new_int32(currentTab));
    sentry_set_context("workspace", workspace);
#else
    Q_UNUSED(tabCount)
    Q_UNUSED(currentTab)
#endif
}
