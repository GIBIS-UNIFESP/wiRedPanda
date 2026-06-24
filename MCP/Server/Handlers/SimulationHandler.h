// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "MCP/Server/Handlers/BaseHandler.h"

class BewavedDolphin;

/**
 * \class SimulationHandler
 * \brief Handler for simulation control and waveform commands.
 *
 * Handles simulation control (start/stop/restart/update) and waveform generation
 * and export. IC management and undo/redo history were split out into ICHandler and
 * HistoryHandler respectively.
 */
class SimulationHandler : public BaseHandler
{
public:
    explicit SimulationHandler(MainWindow *mainWindow, MCPValidator *validator);
    ~SimulationHandler();

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    QJsonObject handleSimulationControl(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleCreateWaveform(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleExportWaveform(const QJsonObject &params, const QJsonValue &requestId);

    BewavedDolphin *m_persistentDolphin;
};
