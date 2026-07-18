// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickMainWindowHost: the application context Quick-side controllers depend on.
 */

#pragma once

#include <QDir>
#include <QFileInfo>

class QString;
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
 * precedent as CanvasCommands/CanvasICRegistry/QuickWorkSpace/QuickWorkspaceManager), sized
 * to what's actually needed so far: no palette()/dolphinHost()/setICButtonsVisible()/
 * refreshICButtonsEnabled() yet, since nothing on the Quick side needs them until the
 * palette (sub-step 4) and IC controller port exist.
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
};
