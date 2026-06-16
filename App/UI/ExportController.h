// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ExportController: orchestrates exporting the current circuit to its output formats.
 */

#pragma once

#include <QObject>
#include <QString>

class MainWindowHost;

/**
 * \class ExportController
 * \brief Owns the circuit-export workflow: Arduino / SystemVerilog code, PDF / PNG
 * images, and beWavedDolphin waveform output.
 *
 * \details Extracted from MainWindow. The `export*` methods are the headless cores
 * (used by the CLI batch mode and the MCP server); the dialog slots add file-picker
 * and status-bar glue on top of them. All circuit state is reached through a
 * MainWindowHost so the controller never depends on a concrete MainWindow.
 */
class ExportController : public QObject
{
    Q_OBJECT

public:
    explicit ExportController(MainWindowHost &host, QObject *parent = nullptr);

    // --- Headless export cores (called by CLI batch mode and the MCP server) ---

    /// Generates an Arduino sketch for the current circuit at \a fileName.
    void exportToArduino(QString fileName);

    /// Generates SystemVerilog for the current circuit at \a fileName.
    void exportToSystemVerilog(QString fileName);

    /// Writes the beWavedDolphin waveform of the current circuit to \a fileName.
    void exportToWaveFormFile(const QString &fileName);

    /// Prints the beWavedDolphin waveform of the current circuit to the terminal.
    void exportToWaveFormTerminal();

public slots:
    // --- Interactive dialog handlers (wired to the Export menu actions) ---

    void exportArduinoDialog();
    void exportSystemVerilogDialog();
    void exportImageDialog();
    void exportPdfDialog();

private:
    MainWindowHost &m_host;
};
