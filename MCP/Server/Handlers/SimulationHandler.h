// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "MCP/Server/Handlers/BaseHandler.h"

class BewavedDolphin;

/**
 * \class SimulationHandler
 * \brief Handler for simulation operation commands (control, waveform generation/export)
 *
 * Handles simulation control (start/stop/restart/update) and waveform generation/export.
 * IC creation/management moved to ICHandler; undo/redo moved to HistoryHandler.
 */
class SimulationHandler : public BaseHandler
{
public:
    explicit SimulationHandler(MainWindow *mainWindow, const MCPValidator *validator);
    ~SimulationHandler();

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    Q_DISABLE_COPY_MOVE(SimulationHandler)

    QJsonObject handleSimulationControl(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleCreateWaveform(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleExportWaveform(const QJsonObject &params, const QJsonValue &requestId);

    // --- Temporal waveform recorder (live, ns-resolution) ---
    QJsonObject handleWatchSignal(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleClearWatchedSignals(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleGetWaveformTrace(const QJsonObject &params, const QJsonValue &requestId);

    BewavedDolphin *m_persistentDolphin;
};
