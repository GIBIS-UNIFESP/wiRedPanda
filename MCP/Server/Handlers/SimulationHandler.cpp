// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/SimulationHandler.h"

#include <QFile>
#include <QJsonArray>
#include <QTextStream>

#include "App/BeWavedDolphin/BeWavedDolphin.h"
#include "App/BeWavedDolphin/SignalModel.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/UI/MainWindow.h"

SimulationHandler::SimulationHandler(MainWindow *mainWindow, const MCPValidator *validator)
    : BaseHandler(mainWindow, validator)
    , m_persistentDolphin(nullptr)
{
}

SimulationHandler::~SimulationHandler()
{
    // SimulationHandler is destroyed synchronously (not necessarily from within a
    // running event loop), so deleteLater() may never fire — delete directly.
    delete m_persistentDolphin;
    m_persistentDolphin = nullptr;
}

QJsonObject SimulationHandler::handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId)
{
    if (command == "simulation_control") {
        return handleSimulationControl(params, requestId);
    } else if (command == "create_waveform") {
        return handleCreateWaveform(params, requestId);
    } else if (command == "export_waveform") {
        return handleExportWaveform(params, requestId);
    } else {
        return createErrorResponse(QString("Unknown simulation command: %1").arg(command),
                                   requestId, JsonRpcError::MethodNotFound);
    }
}

QJsonObject SimulationHandler::handleSimulationControl(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"action"})) {
        return createErrorResponse("Missing required parameter: action", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("action"), "action", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    QString action = params.value("action").toString();

    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    Simulation *simulation = scene->simulation();
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

QJsonObject SimulationHandler::handleCreateWaveform(const QJsonObject &params, const QJsonValue &requestId)
{
    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    int duration = params.value("duration").toInt(32);
    QJsonObject inputPatterns = params.value("input_patterns").toObject();

    if (duration <= 0 || duration > SignalModel::kMaxColumns) {
        return createErrorResponse(QString("Duration must be between 1 and %1").arg(SignalModel::kMaxColumns), requestId, JsonRpcError::ValidationError);
    }

    return tryCommand([&]() -> QJsonObject {
        if (m_persistentDolphin) {
            m_persistentDolphin->deleteLater();
            m_persistentDolphin = nullptr;
        }
        m_persistentDolphin = new BewavedDolphin(scene, false, m_mainWindow);
        BewavedDolphin *bewavedDolphin = m_persistentDolphin;

        bewavedDolphin->prepare("");

        bewavedDolphin->setLength(duration, false);

        if (!inputPatterns.isEmpty()) {
            for (auto it = inputPatterns.begin(); it != inputPatterns.end(); ++it) {
                QString inputLabel = it.key();
                QJsonArray pattern = it.value().toArray();

                const int rowIndex = bewavedDolphin->inputRow(inputLabel);

                if (rowIndex == -1) {
                    return createErrorResponse(QString("Input element with label '%1' not found").arg(inputLabel),
                                               requestId, JsonRpcError::ElementNotFound);
                }

                if (pattern.size() != duration) {
                    return createErrorResponse(QString("Pattern length for '%1' (%2) doesn't match duration (%3)")
                                               .arg(inputLabel).arg(pattern.size()).arg(duration),
                                               requestId, JsonRpcError::ValidationError);
                }

                for (int col = 0; col < duration; ++col) {
                    int value = pattern[col].toInt();
                    if (value != 0 && value != 1) {
                        return createErrorResponse(QString("Invalid pattern value %1 for '%2' at step %3 (must be 0 or 1)")
                                                   .arg(value).arg(inputLabel).arg(col),
                                                   requestId, JsonRpcError::ValidationError);
                    }
                    bewavedDolphin->setCellValue(rowIndex, col, value);
                }
            }
        }

        bewavedDolphin->run();

        QJsonObject waveformData;
        QJsonArray inputData;
        QJsonArray outputData;

        const auto waveform = bewavedDolphin->snapshot(duration);

        for (const auto &signal : waveform.inputs) {
            QJsonObject inputSignal;
            inputSignal["label"] = signal.label;
            inputSignal["type"] = "input";

            QJsonArray values;
            for (const int value : signal.values) {
                values.append(value);
            }
            inputSignal["values"] = values;
            inputData.append(inputSignal);
        }

        for (const auto &signal : waveform.outputs) {
            QJsonObject outputSignal;
            outputSignal["label"] = signal.label;
            outputSignal["type"] = "output";

            QJsonArray values;
            for (const int value : signal.values) {
                values.append(value);
            }
            outputSignal["values"] = values;
            outputData.append(outputSignal);
        }

        waveformData["inputs"] = inputData;
        waveformData["outputs"] = outputData;
        waveformData["duration"] = duration;

        QJsonObject result;
        result["actual_duration"] = duration;
        result["requested_duration"] = duration;
        result["waveform_data"] = waveformData;
        result["message"] = "Waveform created and analyzed successfully";
        result["status"] = "ready";

        return createSuccessResponse(result, requestId);
    }, "create waveform", requestId);
}

QJsonObject SimulationHandler::handleExportWaveform(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"filename", "format"})) {
        return createErrorResponse("Missing required parameters: filename, format", requestId, JsonRpcError::InvalidParams);
    }

    QString filename = params.value("filename").toString();
    QString format = params.value("format").toString().toLower();

    if (format != "txt" && format != "png") {
        return createErrorResponse("Only 'txt' and 'png' formats are supported for waveform export",
                                   requestId, JsonRpcError::ValidationError);
    }

    if (!m_persistentDolphin) {
        return createErrorResponse("No waveform data available. Call create_waveform first.",
                                   requestId, JsonRpcError::SimulationError);
    }

    return tryCommand([&]() -> QJsonObject {
        QJsonObject result;
        result["filename"] = filename;
        result["format"] = format;

        if (format == "txt") {
            QFile file(filename);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                return createErrorResponse(QString("Failed to create file: %1").arg(filename),
                                           requestId, JsonRpcError::FileError);
            }

            QTextStream stream(&file);
            m_persistentDolphin->saveToTxt(stream);
            stream.flush();
            if (stream.status() != QTextStream::Ok || file.error() != QFileDevice::NoError) {
                return createErrorResponse(QString("Failed to write waveform text: %1").arg(file.errorString()),
                                           requestId, JsonRpcError::FileError);
            }

        } else if (format == "png") {
            if (!m_persistentDolphin->exportToPng(filename)) {
                return createErrorResponse("Failed to export waveform as PNG", requestId, JsonRpcError::FileError);
            }
        }

        result["exported"] = true;
        return createSuccessResponse(result, requestId);
    }, "export waveform", requestId);
}
