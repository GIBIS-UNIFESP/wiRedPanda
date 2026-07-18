// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickExportController: CanvasItem-side port of App/UI/ExportController.h.
 */

#pragma once

#include <QObject>
#include <QString>

class QuickMainWindowHost;

/**
 * \class QuickExportController
 * \brief Owns the circuit-export workflow for wiredpanda_quick. CanvasItem-side port of
 * ExportController.
 *
 * \details Copy-and-adapt port (same precedent as CanvasCommands/CanvasICRegistry/
 * QuickWorkSpace/QuickWorkspaceManager), not a modification of the production
 * ExportController: MainWindowHost::currentTab() returns a concrete WorkSpace* (a Widgets
 * type), which this class has no use for.
 *
 * Scoped to Arduino/SystemVerilog export only (ArduinoCodeGen/SystemVerilogCodeGen, both
 * confirmed portable Layer-1 logic taking exactly the QVector<GraphicElement *>
 * CanvasItem::elements() already returns). PDF/image export needs a CircuitExporter overload
 * that takes a CanvasItem instead of a Scene -- not built yet, tracked as sub-step 8's job.
 * Waveform export needs BeWavedDolphin, out of scope until Phase 6. Both are real, named
 * deferrals, not oversights.
 */
class QuickExportController : public QObject
{
    Q_OBJECT

public:
    explicit QuickExportController(QuickMainWindowHost &host, QObject *parent = nullptr);

    /// Generates an Arduino sketch for the current tab's circuit at \a fileName.
    void exportToArduino(QString fileName);

    /// Generates SystemVerilog for the current tab's circuit at \a fileName.
    void exportToSystemVerilog(QString fileName);

public slots:
    void exportArduinoDialog();
    void exportSystemVerilogDialog();

private:
    QuickMainWindowHost &m_host;
};
