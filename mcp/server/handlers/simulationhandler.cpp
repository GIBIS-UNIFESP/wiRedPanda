// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simulationhandler.h"

#include "bewaveddolphin.h"
#include "commands.h"
#include "graphicelementinput.h"
#include "ic.h"
#include "mainwindow.h"
#include "scene.h"
#include "serialization.h"
#include "simulation.h"
#include "workspace.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QTextStream>

SimulationHandler::SimulationHandler(MainWindow *mainWindow, MCPValidator *validator)
    : BaseHandler(mainWindow, validator)
    , m_persistentDolphin(nullptr)
{
}

SimulationHandler::~SimulationHandler()
{
    if (m_persistentDolphin) {
        m_persistentDolphin->deleteLater();
        m_persistentDolphin = nullptr;
    }
}

QJsonObject SimulationHandler::handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId)
{
    if (command == "simulation_control") {
        return handleSimulationControl(params, requestId);
    } else if (command == "create_waveform") {
        return handleCreateWaveform(params, requestId);
    } else if (command == "export_waveform") {
        return handleExportWaveform(params, requestId);
    } else if (command == "create_ic") {
        return handleCreateIC(params, requestId);
    } else if (command == "instantiate_ic") {
        return handleInstantiateIC(params, requestId);
    } else if (command == "list_ics") {
        return handleListICs(params, requestId);
    } else {
        return createErrorResponse(QString("Unknown simulation command: %1").arg(command), requestId);
    }
}

QJsonObject SimulationHandler::handleSimulationControl(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"action"})) {
        return createErrorResponse("Missing required parameter: action", requestId);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("action"), "action", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    QString action = params.value("action").toString();

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    Simulation *simulation = scene->simulation();
    if (!simulation) {
        return createErrorResponse("No simulation available", requestId);
    }

    if (action == "start") {
        simulation->start();
    } else if (action == "stop") {
        simulation->stop();
    } else if (action == "restart") {
        simulation->restart();
    } else if (action == "update") {
        for (int i = 0; i < 4; ++i) {
            simulation->update();
        }
    } else {
        return createErrorResponse(QString("Invalid action: %1").arg(action));
    }

    return createSuccessResponse(QJsonObject(), requestId);
}

QJsonObject SimulationHandler::handleCreateWaveform(const QJsonObject &params, const QJsonValue &requestId)
{
    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    int duration = params.value("duration").toInt(32);
    QJsonObject inputPatterns = params.value("input_patterns").toObject();

    if (duration <= 0 || duration > 1024) {
        return createErrorResponse("Duration must be between 1 and 1024", requestId);
    }

    try {
        if (m_persistentDolphin) {
            m_persistentDolphin->deleteLater();
            m_persistentDolphin = nullptr;
        }
        m_persistentDolphin = new BewavedDolphin(scene, false, m_mainWindow);
        BewavedDolphin *bewavedDolphin = m_persistentDolphin;

        bewavedDolphin->prepare("");

        bewavedDolphin->setLength(duration, false);

        if (!inputPatterns.isEmpty()) {
            const auto inputElements = bewavedDolphin->getInputElements();

            for (auto it = inputPatterns.begin(); it != inputPatterns.end(); ++it) {
                QString inputLabel = it.key();
                QJsonArray pattern = it.value().toArray();

                int rowIndex = -1;
                for (int i = 0; i < inputElements.size(); ++i) {
                    if (inputElements[i]->label() == inputLabel) {
                        rowIndex = i;
                        break;
                    }
                }

                if (rowIndex == -1) {
                    return createErrorResponse(QString("Input element with label '%1' not found").arg(inputLabel), requestId);
                }

                if (pattern.size() != duration) {
                    return createErrorResponse(QString("Pattern length for '%1' (%2) doesn't match duration (%3)")
                                               .arg(inputLabel).arg(pattern.size()).arg(duration), requestId);
                }

                for (int col = 0; col < duration; ++col) {
                    int value = pattern[col].toInt();
                    if (value != 0 && value != 1) {
                        return createErrorResponse(QString("Invalid pattern value %1 for '%2' at step %3 (must be 0 or 1)")
                                                   .arg(value).arg(inputLabel).arg(col), requestId);
                    }
                    bewavedDolphin->createElement(rowIndex, col, value, true, false);
                }
            }
        }

        bewavedDolphin->run();

        QJsonObject waveformData;
        QJsonArray inputData;
        QJsonArray outputData;

        const auto inputs = bewavedDolphin->getInputElements();
        const auto outputs = bewavedDolphin->getOutputElements();
        const auto model = bewavedDolphin->getModel();

        for (int row = 0; row < inputs.size(); ++row) {
            QJsonObject inputSignal;
            inputSignal["label"] = inputs[row]->label();
            inputSignal["type"] = "input";

            QJsonArray values;
            for (int col = 0; col < duration; ++col) {
                values.append(model->index(row, col).data().toInt());
            }
            inputSignal["values"] = values;
            inputData.append(inputSignal);
        }

        for (int row = 0; row < outputs.size(); ++row) {
            QJsonObject outputSignal;
            outputSignal["label"] = outputs[row]->label();
            outputSignal["type"] = "output";

            QJsonArray values;
            int outputRowIndex = inputs.size() + row;
            for (int col = 0; col < duration; ++col) {
                values.append(model->index(outputRowIndex, col).data().toInt());
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

    } catch (const std::exception &e) {
        return createErrorResponse(QString("Waveform creation failed: %1").arg(e.what()), requestId);
    } catch (...) {
        return createErrorResponse("Waveform creation failed: Unknown error", requestId);
    }
}

QJsonObject SimulationHandler::handleExportWaveform(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"filename", "format"})) {
        return createErrorResponse("Missing required parameters: filename, format", requestId);
    }

    QString filename = params.value("filename").toString();
    QString format = params.value("format").toString().toLower();

    if (format != "txt" && format != "png") {
        return createErrorResponse("Only 'txt' and 'png' formats are supported for waveform export", requestId);
    }

    if (!m_persistentDolphin) {
        return createErrorResponse("No waveform data available. Call create_waveform first.", requestId);
    }

    try {
        QJsonObject result;
        result["filename"] = filename;
        result["format"] = format;

        if (format == "txt") {
            QFile file(filename);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                return createErrorResponse(QString("Failed to create file: %1").arg(filename), requestId);
            }

            QTextStream stream(&file);
            m_persistentDolphin->saveToTxt(stream);

        } else if (format == "png") {
            if (!m_persistentDolphin->exportToPng(filename)) {
                return createErrorResponse("Failed to export waveform as PNG", requestId);
            }
        }

        result["exported"] = true;
        return createSuccessResponse(result, requestId);

    } catch (const std::exception &e) {
        return createErrorResponse(QString("Waveform export failed: %1").arg(e.what()), requestId);
    } catch (...) {
        return createErrorResponse("Waveform export failed: Unknown error", requestId);
    }
}

QJsonObject SimulationHandler::handleCreateIC(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"name"})) {
        return createErrorResponse("Missing required parameter: name", requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    QString name = params.value("name").toString();
    QString description = params.value("description").toString("");

    if (name.isEmpty()) {
        return createErrorResponse("IC name cannot be empty", requestId);
    }

    try {
        QString icFileName = name + ".panda";
        QString fullPath = QDir(m_mainWindow->currentDir()).absoluteFilePath(icFileName);

        if (QFile::exists(fullPath)) {
            return createErrorResponse(QString("IC file already exists: %1").arg(icFileName), requestId);
        }

        const auto elements = scene->elements();
        if (elements.isEmpty()) {
            return createErrorResponse("Cannot create IC from empty circuit", requestId);
        }

        auto *workspace = m_mainWindow->currentTab();
        if (!workspace) {
            return createErrorResponse("No active workspace available", requestId);
        }

        workspace->save(fullPath);

        QJsonObject result;
        result["name"] = name;
        result["filename"] = icFileName;
        result["path"] = fullPath;
        result["elements_count"] = elements.size();
        result["description"] = description;
        result["message"] = "IC created successfully from current circuit";

        return createSuccessResponse(result, requestId);

    } catch (const std::exception &e) {
        return createErrorResponse(QString("IC creation failed: %1").arg(e.what()), requestId);
    } catch (...) {
        return createErrorResponse("IC creation failed: Unknown error", requestId);
    }
}

QJsonObject SimulationHandler::handleInstantiateIC(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"ic_name", "x", "y"})) {
        return createErrorResponse("Missing required parameters: ic_name, x, y", requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    QString icName = params.value("ic_name").toString();
    int x = params.value("x").toInt();
    int y = params.value("y").toInt();
    QString label = params.value("label").toString(icName);

    try {
        QString icFileName = icName + ".panda";
        QString fullPath = QDir(m_mainWindow->currentDir()).absoluteFilePath(icFileName);

        if (!QFile::exists(fullPath)) {
            return createErrorResponse(QString("IC file not found: %1").arg(icFileName), requestId);
        }

        auto *ic = new IC();

        ic->loadFile(fullPath);

        ic->setPos(x, y);
        ic->setLabel(label);

        QList<QGraphicsItem *> items;
        items.append(ic);
        scene->receiveCommand(new AddItemsCommand(items, scene));

        QJsonObject result;
        result["element_id"] = ic->id();
        result["ic_name"] = icName;
        result["filename"] = icFileName;
        result["label"] = label;
        result["position"] = QJsonObject{{"x", x}, {"y", y}};
        result["input_count"] = ic->inputSize();
        result["output_count"] = ic->outputSize();
        result["message"] = "IC instantiated successfully";

        return createSuccessResponse(result, requestId);

    } catch (const std::exception &e) {
        return createErrorResponse(QString("IC instantiation failed: %1").arg(e.what()), requestId);
    } catch (...) {
        return createErrorResponse("IC instantiation failed: Unknown error", requestId);
    }
}

QJsonObject SimulationHandler::handleListICs(const QJsonObject &, const QJsonValue &requestId)
{
    try {
        QJsonArray icsArray;

        QDir currentDir(m_mainWindow->currentDir());
        QStringList filters;
        filters << "*.panda";

        const QFileInfoList pandaFiles = currentDir.entryInfoList(filters, QDir::Files);

        for (const QFileInfo &fileInfo : pandaFiles) {
            try {
                QFile file(fileInfo.absoluteFilePath());
                if (!file.open(QIODevice::ReadOnly)) {
                    continue;
                }

                QDataStream stream(&file);
                QVersionNumber version = Serialization::readPandaHeader(stream);

                if (!version.isNull()) {
                    QJsonObject icInfo;
                    icInfo["name"] = fileInfo.baseName();
                    icInfo["filename"] = fileInfo.fileName();
                    icInfo["path"] = fileInfo.absoluteFilePath();
                    icInfo["size"] = fileInfo.size();
                    icInfo["modified"] = fileInfo.lastModified().toString(Qt::ISODate);

                    try {
                        IC tempIC;
                        tempIC.loadFile(fileInfo.absoluteFilePath());
                        icInfo["input_count"] = tempIC.inputSize();
                        icInfo["output_count"] = tempIC.outputSize();
                        icInfo["has_valid_definition"] = true;
                    } catch (...) {
                        icInfo["has_valid_definition"] = false;
                        icInfo["input_count"] = 0;
                        icInfo["output_count"] = 0;
                    }

                    icsArray.append(icInfo);
                }

                file.close();

            } catch (...) {
                continue;
            }
        }

        QJsonObject result;
        result["ics"] = icsArray;
        result["directory"] = currentDir.absolutePath();
        result["count"] = icsArray.size();

        return createSuccessResponse(result, requestId);

    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to list ICs: %1").arg(e.what()), requestId);
    } catch (...) {
        return createErrorResponse("Failed to list ICs: Unknown error", requestId);
    }
}
