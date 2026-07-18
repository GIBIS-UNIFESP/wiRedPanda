// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Abstract confirm/prompt dialog interface for the Quick chrome -- the
 * QMessageBox/QInputDialog equivalent of App/UI/FileDialogProvider.h.
 */

#pragma once

#include <optional>

#include <QList>
#include <QString>

/// One button a choice() dialog can offer. Sized to the real shapes the ported
/// WorkSpace/WorkspaceManager call sites use (Ok-only notices, Yes/No, and the
/// Yes/YesToAll/No/NoToAll/Cancel "closing multiple tabs" prompt) -- extend as later
/// sub-steps port controllers with other real shapes, not speculatively ahead of them.
enum class DialogButton { Ok, Yes, No, Cancel, YesToAll, NoToAll };

/// Abstract interface for blocking confirm/notice/prompt dialogs.
///
/// Production Quick code uses the global accessor Dialogs::provider(), which returns a
/// QuickDialogProvider (App/QuickShell/Chrome/QuickDialogProvider.h) by default. Tests can
/// swap in a stub via Dialogs::setProvider(), mirroring FileDialogs' pattern exactly so
/// ported controller code (WorkspaceManager, ICController, ...) keeps calling a plain,
/// synchronous, blocking API unchanged from its QMessageBox-based original -- only the
/// concrete provider swaps from a QWidget modal to a QML Dialog.
class DialogProvider
{
public:
    virtual ~DialogProvider() = default;

    /// Shows a modal dialog titled \a title with \a buttons and blocks until one is clicked,
    /// returning it. Returns \a defaultButton if the dialog is dismissed without a button
    /// click (e.g. the window is closed some other way).
    virtual DialogButton choice(const QString &title, const QString &text,
                                 const QList<DialogButton> &buttons, DialogButton defaultButton) = 0;

    /// Shows a single-line text prompt pre-filled with \a initialValue and blocks until
    /// dismissed. Returns std::nullopt if the user cancels.
    virtual std::optional<QString> textPrompt(const QString &title, const QString &label,
                                               const QString &initialValue = {}) = 0;
};

/// Global accessor for the current DialogProvider.
namespace Dialogs {

/// Returns the active provider. Never null.
DialogProvider *provider();

/// Replaces the active provider. Pass nullptr to restore the default.
/// Returns the previous provider (for RAII restoration in tests).
DialogProvider *setProvider(DialogProvider *newProvider);

} // namespace Dialogs
