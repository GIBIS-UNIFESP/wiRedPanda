// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief MainWindowHost: the application context that MainWindow's extracted controllers depend on.
 */

#pragma once

#include <QFileInfo>

class DolphinHost;
class QString;
class QWidget;
class WorkSpace;

/**
 * \class MainWindowHost
 * \brief Interface MainWindow provides to its extracted controllers (export, IC, …).
 *
 * \details Mirrors the DolphinHost pattern: collaborators receive this narrow
 * context instead of a concrete MainWindow, so each cluster can be exercised with a
 * stub host. Exposes the current tab, the current circuit file, a parent widget for
 * dialogs, and a status-bar message sink.
 */
class MainWindowHost
{
public:
    virtual ~MainWindowHost() = default;

    /// Currently visible WorkSpace tab, or nullptr when none is open.
    virtual WorkSpace *currentTab() const = 0;

    /// QFileInfo of the currently active .panda file.
    virtual QFileInfo currentFile() const = 0;

    /// Widget used to parent modal dialogs spawned by a controller.
    virtual QWidget *widget() = 0;

    /// The host's DolphinHost facet, used to drive headless beWavedDolphin exports.
    virtual DolphinHost *dolphinHost() = 0;

    /// Shows \a message in the host's status bar for \a timeout milliseconds.
    virtual void showStatusMessage(const QString &message, int timeout) = 0;
};
