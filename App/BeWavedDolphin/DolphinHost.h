// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief DolphinHost: the host-application context the beWavedDolphin editor depends on.
 */

#pragma once

#include <QDir>
#include <QFileInfo>
#include <QString>

/**
 * \class DolphinHost
 * \brief Interface the host application (MainWindow) provides to BewavedDolphin.
 *
 * \details Gives the waveform editor the current circuit file/directory and the
 * .dolphin-file association, without a hard dependency on MainWindow. Injecting this
 * (rather than a concrete MainWindow) lets the editor's Save-As / load / associate
 * paths be exercised in tests with a stub host.
 */
class DolphinHost
{
public:
    virtual ~DolphinHost() = default;

    /// Path of the host's current circuit (.panda) file.
    virtual QFileInfo currentFile() const = 0;

    /// Working directory of the host's current circuit.
    virtual QDir currentDir() const = 0;

    /// Returns the .dolphin file currently linked to the circuit (empty if none).
    virtual QString dolphinFileName() = 0;

    /// Links \a fileName as the circuit's associated .dolphin file.
    virtual void setDolphinFileName(const QString &fileName) = 0;

    /// Saves the host's current circuit to \a fileName (empty = its current path).
    virtual void save(const QString &fileName) = 0;
};
