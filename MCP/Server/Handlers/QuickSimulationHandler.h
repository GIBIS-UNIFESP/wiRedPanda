// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "MCP/Server/Handlers/QuickBaseHandler.h"

class QuickDolphinController;

/**
 * \class QuickSimulationHandler
 * \brief CanvasItem-side port of SimulationHandler.
 *
 * \details simulation_control (start/stop/restart/update) only ever touched
 * Scene::simulation(), now CanvasItem::simulation() -- a mechanical repoint. create_waveform/
 * export_waveform (Phase 12b) mirror SimulationHandler's own m_persistentDolphin design: a
 * dedicated, hidden QuickDolphinController owned by this handler, entirely separate from
 * AppController.dolphin (the interactive one DolphinWindow.qml shows) -- an MCP script calling
 * create_waveform must never silently reset/mutate whatever waveform the user currently has open.
 */
class QuickSimulationHandler : public QuickBaseHandler
{
public:
    explicit QuickSimulationHandler(QuickAppController *appController, const MCPValidator *validator);
    ~QuickSimulationHandler() override;

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    friend class TestQuickSimulationHandler; // testCreateWaveformReplacesExistingDolphin's m_persistentDolphin identity check, mirrors SimulationHandler.h's identical friend declaration.

    Q_DISABLE_COPY_MOVE(QuickSimulationHandler)

    QJsonObject handleSimulationControl(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleCreateWaveform(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleExportWaveform(const QJsonObject &params, const QJsonValue &requestId);

    QuickDolphinController *m_persistentDolphin = nullptr; // owned; see class doc comment
};
