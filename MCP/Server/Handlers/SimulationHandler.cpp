// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/SimulationHandler.h"

#include <QFile>
#include <QJsonArray>
#include <QTextStream>

#include "App/BeWavedDolphin/BeWavedDolphin.h"
#include "App/BeWavedDolphin/SignalModel.h"
#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/WaveformRecorder.h"
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
    } else if (command == "watch_signal") {
        return handleWatchSignal(params, requestId);
    } else if (command == "clear_watched_signals") {
        return handleClearWatchedSignals(params, requestId);
    } else if (command == "get_waveform_trace") {
        return handleGetWaveformTrace(params, requestId);
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
        } else if (action == "set_time_per_tick") {
            // Sets the simulation mode: 0 ⇒ functional (zero-delay), >0 ⇒ temporal (advance this
            // many ns of sim-time per update(), spreading per-element propagation delays over time).
            // The schema's if/then already requires this field for a spec-compliant client; this
            // is defense in depth for any client that bypasses client-side schema validation —
            // without it, an omitted field silently defaulted to 0 (functional mode) instead of
            // failing loudly.
            if (!params.contains("time_per_tick")) {
                return createErrorResponse("Missing required parameter: time_per_tick (required when action=set_time_per_tick)",
                                           requestId, JsonRpcError::InvalidParams);
            }
            // Enforce the schema's bounds server-side too (the server is the source of truth for
            // clients that skip client-side validation): negative values would wrap the unsigned
            // SimTime, and anything above 1,000,000 ns exceeds the documented maximum shared with
            // the propagation-delay bound.
            const auto raw = params.value("time_per_tick").toInteger();
            if (raw < 0 || raw > 1'000'000) {
                return createErrorResponse("time_per_tick out of range (0..1000000 ns)",
                                           requestId, JsonRpcError::InvalidParams);
            }
            const auto ns = static_cast<SimTime>(raw);
            simulation->setTimePerTick(ns);
            QJsonObject result;
            result["time_per_tick"] = static_cast<double>(ns);
            return createSuccessResponse(result, requestId);
        } else if (action == "get_state") {
            QJsonObject result;
            result["running"] = simulation->isRunning();
            result["current_time"] = static_cast<double>(simulation->currentTime());
            result["time_per_tick"] = static_cast<double>(simulation->timePerTick());
            return createSuccessResponse(result, requestId);
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
        m_persistentDolphin = new BewavedDolphin(scene, false, m_mainWindow, m_mainWindow);
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

        // Temporal mode: sweep with per-element propagation delays, so outputs lag their cause by
        // whole columns (ns_per_column sim-time advanced per column). Default is functional.
        const bool temporal = params.value("temporal").toBool(false);
        if (temporal) {
            // Enforce the schema's bounds server-side too (the server is the source of truth for
            // clients that skip client-side validation): a negative value would wrap the unsigned
            // SimTime to ~UINT64_MAX, and any huge value makes the sweep's per-column drain window
            // practically unbounded — a delayed-feedback circuit (e.g. a ring oscillator) then
            // keeps self-scheduling at ever-later timestamps and processEvents() never returns.
            const auto raw = params.value("ns_per_column").toInteger(2);
            if (raw < 1 || raw > 1'000'000) {
                return createErrorResponse("ns_per_column out of range (1..1000000 ns)",
                                           requestId, JsonRpcError::InvalidParams);
            }
            bewavedDolphin->setTemporalMode(true, static_cast<SimTime>(raw));
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

// ── Temporal waveform recorder (live, ns-resolution) ─────────────────────────

QJsonObject SimulationHandler::handleWatchSignal(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id"})) {
        return createErrorResponse("Missing required parameter: element_id", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    auto *element = validatedElement(params, "element_id", errorMsg);
    if (!element) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::ElementNotFound);
    }

    Scene *scene = currentScene();
    if (!scene || !scene->simulation()) {
        return createErrorResponse("No simulation available", requestId, JsonRpcError::SimulationError);
    }

    const int port = params.value("port").toInt(0);
    if (port < 0 || port >= element->outputSize()) {
        return createErrorResponse(QString("Invalid port %1 for element with %2 output(s)")
                                       .arg(port).arg(element->outputSize()),
                                   requestId, JsonRpcError::ValidationError);
    }

    QString name = params.value("name").toString();
    if (name.isEmpty()) {
        name = element->label().isEmpty() ? ElementFactory::translatedName(element->elementType())
                                          : element->label();
        if (element->outputSize() > 1) {
            name += QStringLiteral(" [%1]").arg(port);
        }
    }

    auto &recorder = scene->simulation()->waveformRecorder();
    const int idx = recorder.watchSignal(name, element, port);
    recorder.setRecording(true);

    QJsonObject result;
    result["trace_index"] = idx;
    result["name"] = name;
    return createSuccessResponse(result, requestId);
}

QJsonObject SimulationHandler::handleClearWatchedSignals(const QJsonObject &params, const QJsonValue &requestId)
{
    Q_UNUSED(params)

    Scene *scene = currentScene();
    if (!scene || !scene->simulation()) {
        return createErrorResponse("No simulation available", requestId, JsonRpcError::SimulationError);
    }

    auto &recorder = scene->simulation()->waveformRecorder();
    recorder.setRecording(false);
    recorder.clear();
    return createSuccessResponse(QJsonObject(), requestId);
}

QJsonObject SimulationHandler::handleGetWaveformTrace(const QJsonObject &params, const QJsonValue &requestId)
{
    Scene *scene = currentScene();
    if (!scene || !scene->simulation()) {
        return createErrorResponse("No simulation available", requestId, JsonRpcError::SimulationError);
    }

    const bool filterById = params.contains("element_id");
    const int filterId = params.value("element_id").toInt(-1);
    const int filterPort = params.contains("port") ? params.value("port").toInt(-1) : -1;

    // Temporal feedback can canonicalize a node to Unknown, so the trace value is 3-state
    // ("low"/"high"/"unknown") rather than the plain bool that get_output_value returns.
    const auto statusText = [](Status status) -> QString {
        switch (status) {
        case Status::Active:   return QStringLiteral("high");
        case Status::Inactive: return QStringLiteral("low");
        default:               return QStringLiteral("unknown");
        }
    };

    auto &recorder = scene->simulation()->waveformRecorder();
    QJsonArray traces;
    for (const auto &trace : recorder.traces()) {
        GraphicElement *logic = trace.logic;
        if (!logic) {
            continue; // watched element was deleted (QPointer went null)
        }
        if (filterById && logic->id() != filterId) {
            continue;
        }
        if (filterPort >= 0 && trace.portIndex != filterPort) {
            continue;
        }

        QJsonArray transitions;
        for (const auto &[time, status] : trace.transitions) {
            QJsonObject transitionObj;
            transitionObj["time"] = static_cast<double>(time);
            transitionObj["value"] = statusText(status);
            transitions.append(transitionObj);
        }

        QJsonObject traceObj;
        traceObj["name"] = trace.name;
        traceObj["element_id"] = logic->id();
        traceObj["port"] = trace.portIndex;
        traceObj["transitions"] = transitions;
        traces.append(traceObj);
    }

    QJsonObject result;
    result["traces"] = traces;
    return createSuccessResponse(result, requestId);
}
