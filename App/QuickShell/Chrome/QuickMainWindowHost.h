// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickMainWindowHost: the application context Quick-side controllers depend on.
 */

#pragma once

#include <QDir>
#include <QFileInfo>

class QString;
class QuickElementPalette;
class QuickWorkSpace;

/**
 * \class QuickMainWindowHost
 * \brief CanvasItem-side port of MainWindowHost's narrow role: the context
 * QuickExportController (and later Quick-side controllers) depend on instead of a concrete
 * app-shell class, so each can be exercised with a stub host.
 *
 * \details Not a literal implementation of the production MainWindowHost interface --
 * MainWindowHost::currentTab() returns a concrete WorkSpace* (a Widgets type), which
 * QuickExportController has no use for and can't satisfy. Copy-and-adapt port (same
 * precedent as CanvasCommands/CanvasICRegistry/QuickWorkSpace/QuickWorkspaceManager). Gained
 * palette()/requestSave() for QuickICController (Phase 4 sub-step 7); still no
 * dolphinHost()/widget()/setICButtonsVisible()/refreshICButtonsEnabled() -- no Quick-side
 * caller needs them yet (no BeWavedDolphin/IC-toolbar-button chrome exists), and no
 * icListFile() -- QuickAppController::bindCurrentTab() already established the precedent of
 * using currentFile() directly instead of icListFile()'s inline-IC-tab parent-chain walk,
 * since inline IC tabs have no UI trigger in the Quick chrome yet; QuickICController follows
 * the same simplification rather than adding a redundant interface method for it.
 */
class QuickMainWindowHost
{
public:
    virtual ~QuickMainWindowHost() = default;

    /// Currently visible tab, or nullptr when none is open.
    virtual QuickWorkSpace *currentTab() const = 0;

    /// QFileInfo of the currently active .panda file.
    virtual QFileInfo currentFile() const = 0;

    /// Directory of the currently active .panda file.
    virtual QDir currentDir() const = 0;

    /// Shows \a message for \a timeout milliseconds -- a no-op until a status bar exists to
    /// display it, matching QuickWorkspaceManager's own "no chrome yet" deferrals.
    virtual void showStatusMessage(const QString &message, int timeout) = 0;

    /// The element palette panel (file-based and embedded IC lists live here). Mirrors
    /// MainWindowHost::palette() -- not const, unlike the production signature: the
    /// implementation stores its QuickElementPalette as a plain value member (see
    /// QuickAppController::elementPalette()'s own identical non-const precedent), so a const
    /// override could only return a const pointer to it.
    virtual QuickElementPalette *palette() = 0;

    /// Saves the current tab (equivalent to triggering the Save action). Mirrors
    /// MainWindowHost::requestSave().
    virtual void requestSave() = 0;
};
