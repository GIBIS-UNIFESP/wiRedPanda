// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "MCP/Server/Handlers/QuickBaseHandler.h"

/**
 * \class QuickSimulationHandler
 * \brief CanvasItem-side port of SimulationHandler.
 *
 * \details simulation_control (start/stop/restart/update) is fully ported -- it only ever
 * touched Scene::simulation(), now CanvasItem::simulation(). create_waveform/export_waveform
 * are NOT ported: both are entirely built on constructing a real BewavedDolphin (a QMainWindow-
 * derived Widgets class -- SimulationHandler.cpp literally does `new BewavedDolphin(scene,
 * false, m_mainWindow, m_mainWindow)`), which doesn't exist in the Quick chrome yet (Phase 6).
 * Both return a clean, discoverable "not yet available" error instead of a crash or silent
 * no-op -- the same graceful-degradation precedent this rewrite already uses for BeWavedDolphin-
 * scoped Tour/Exercise steps (see App/QuickShell/TourOverlay.qml's resolveTarget()) and for
 * QuickExportController's identically-scoped waveform-export gap (see that class's own doc
 * comment).
 */
class QuickSimulationHandler : public QuickBaseHandler
{
public:
    explicit QuickSimulationHandler(QuickAppController *appController, const MCPValidator *validator);

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

private:
    Q_DISABLE_COPY_MOVE(QuickSimulationHandler)

    QJsonObject handleSimulationControl(const QJsonObject &params, const QJsonValue &requestId);
};
