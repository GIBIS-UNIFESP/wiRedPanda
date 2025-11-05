// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "basehandler.h"

class BewavedDolphin;

/*!
 * @class SimulationHandler
 * @brief Handler for simulation operation commands (control, waveforms, ICs)
 *
 * Handles all simulation-related MCP commands including simulation control,
 * waveform generation and export, and IC creation and management.
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
    QJsonObject handleCreateIC(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleInstantiateIC(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleListICs(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleUndo(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleRedo(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleGetUndoStack(const QJsonObject &params, const QJsonValue &requestId);

    BewavedDolphin *m_persistentDolphin;
};
