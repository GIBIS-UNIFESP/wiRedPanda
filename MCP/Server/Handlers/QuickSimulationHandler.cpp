// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/QuickSimulationHandler.h"

#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/Simulation/Simulation.h"

QuickSimulationHandler::QuickSimulationHandler(QuickAppController *appController, const MCPValidator *validator)
    : QuickBaseHandler(appController, validator)
{
}

QJsonObject QuickSimulationHandler::handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId)
{
    if (command == "simulation_control") {
        return handleSimulationControl(params, requestId);
    } else if (command == "create_waveform" || command == "export_waveform") {
        // See this class's own doc comment: BeWavedDolphin isn't ported to Quick yet (Phase 6).
        return createErrorResponse(
            QString("'%1' is not yet available in wiredpanda_quick: waveform generation needs "
                    "BeWavedDolphin, not yet ported (Phase 6 of the Qt Quick rewrite)").arg(command),
            requestId, JsonRpcError::SimulationError);
    } else {
        return createErrorResponse(QString("Unknown simulation command: %1").arg(command),
                                   requestId, JsonRpcError::MethodNotFound);
    }
}

QJsonObject QuickSimulationHandler::handleSimulationControl(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"action"})) {
        return createErrorResponse("Missing required parameter: action", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("action"), "action", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    QString action = params.value("action").toString();

    CanvasItem *canvas = currentCanvas();
    if (!canvas) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    Simulation *simulation = canvas->simulation();
    if (!simulation) {
        return createErrorResponse("No simulation available", requestId, JsonRpcError::SimulationError);
    }

    return tryCommand([&]() -> QJsonObject {
        if (action == "start") {
            simulation->start();
        } else if (action == "stop") {
            simulation->stop();
        } else if (action == "restart") {
            simulation->restart();
        } else if (action == "update") {
            simulation->update();
        } else {
            return createErrorResponse(QString("Invalid action: %1").arg(action),
                                       requestId, JsonRpcError::ValidationError);
        }
        return createSuccessResponse(QJsonObject(), requestId);
    }, "control simulation", requestId);
}
