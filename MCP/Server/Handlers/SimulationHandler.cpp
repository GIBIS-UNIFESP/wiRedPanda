// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/SimulationHandler.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QSaveFile>
#include <QTextStream>

#include "App/BeWavedDolphin/BeWavedDolphin.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/IC.h"
#include "App/Element/ICRegistry.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/UI/MainWindow.h"

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
    } else if (command == "embed_ic") {
        return handleEmbedIC(params, requestId);
    } else if (command == "extract_ic") {
        return handleExtractIC(params, requestId);
    } else if (command == "undo") {
        return handleUndo(params, requestId);
    } else if (command == "redo") {
        return handleRedo(params, requestId);
    } else if (command == "get_undo_stack") {
        return handleGetUndoStack(params, requestId);
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
            return createErrorResponse(QString("Invalid action: %1").arg(action), requestId);
        }
        return createSuccessResponse(QJsonObject(), requestId);
    }, "control simulation", requestId);
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
            int outputRowIndex = static_cast<int>(inputs.size()) + row;
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
    }, "create waveform", requestId);
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

    return tryCommand([&]() -> QJsonObject {
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
    }, "export waveform", requestId);
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

    return tryCommand([&]() -> QJsonObject {
        QString icFileName = name + ".panda";
        QString fullPath = m_mainWindow->currentDir().absoluteFilePath(icFileName);

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
    }, "create IC", requestId);
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
    const int snap = Scene::gridSize / 2;
    int x = qRound(params.value("x").toDouble() / snap) * snap;
    int y = qRound(params.value("y").toDouble() / snap) * snap;
    QString label = params.value("label").toString(icName);

    return tryCommand([&]() -> QJsonObject {
        QString icFileName = icName + ".panda";
        QString fullPath = m_mainWindow->currentDir().absoluteFilePath(icFileName);

        if (!QFile::exists(fullPath)) {
            return createErrorResponse(QString("IC file not found: %1").arg(icFileName), requestId);
        }

        auto ic = std::make_unique<IC>();

        const QString icDirectory = QFileInfo(fullPath).absolutePath();
        const bool inlineMode = params.value("inline").toBool(false);

        IC *icPtr = nullptr;

        if (inlineMode) {
            QFile file(fullPath);
            if (!file.open(QIODevice::ReadOnly)) {
                return createErrorResponse(QString("Could not read IC file: %1").arg(file.errorString()), requestId);
            }
            QByteArray fileBytes = file.readAll();
            file.close();

            QString blobName = params.value("blob_name").toString();
            if (blobName.isEmpty()) {
                blobName = QFileInfo(fullPath).baseName();
            }

            auto *reg = scene->icRegistry();
            if (reg->hasBlob(blobName)) {
                return createErrorResponse(QString("Blob name collision: an embedded IC named '%1' already exists. "
                                                   "Use blob_name parameter to specify a different name.").arg(blobName), requestId);
            }

            icPtr = reg->createEmbeddedIC(blobName, fileBytes, icDirectory);
        } else {
            icPtr = ic.release();
            icPtr->loadFile(fullPath, icDirectory);
            scene->receiveCommand(new AddItemsCommand({icPtr}, scene));
        }

        icPtr->setPos(x, y);
        icPtr->setLabel(label);

        QJsonObject result;
        result["element_id"] = icPtr->id();
        result["ic_name"] = icName;
        result["filename"] = icFileName;
        result["label"] = label;
        result["position"] = QJsonObject{{"x", x}, {"y", y}};
        result["input_count"] = icPtr->inputSize();
        result["output_count"] = icPtr->outputSize();
        if (inlineMode) {
            result["inline"] = true;
            result["blob_name"] = icPtr->blobName();
        }
        result["message"] = "IC instantiated successfully";

        return createSuccessResponse(result, requestId);
    }, "instantiate IC", requestId);
}

QJsonObject SimulationHandler::handleListICs(const QJsonObject &, const QJsonValue &requestId)
{
    return tryCommand([&]() -> QJsonObject {
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
                        tempIC.loadFile(fileInfo.absoluteFilePath(), fileInfo.absolutePath());
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
    }, "list ICs", requestId);
}

QJsonObject SimulationHandler::handleUndo(const QJsonObject &params, const QJsonValue &requestId)
{
    (void)params;  // Parameter not used
    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    QUndoStack *undoStack = scene->undoStack();
    if (!undoStack) {
        return createErrorResponse("Undo stack not available", requestId);
    }

    return tryCommand([&]() -> QJsonObject {
        if (!undoStack->canUndo()) {
            QJsonObject result;
            result["success"] = false;
            result["message"] = "Nothing to undo";
            return createSuccessResponse(result, requestId);
        }

        undoStack->undo();

        QJsonObject result;
        result["success"] = true;
        result["message"] = "Undo operation completed";
        result["can_undo"] = undoStack->canUndo();
        result["can_redo"] = undoStack->canRedo();
        result["undo_text"] = undoStack->undoText();
        result["redo_text"] = undoStack->redoText();

        return createSuccessResponse(result, requestId);
    }, "undo", requestId);
}

QJsonObject SimulationHandler::handleRedo(const QJsonObject &params, const QJsonValue &requestId)
{
    (void)params;  // Parameter not used
    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    QUndoStack *undoStack = scene->undoStack();
    if (!undoStack) {
        return createErrorResponse("Undo stack not available", requestId);
    }

    return tryCommand([&]() -> QJsonObject {
        if (!undoStack->canRedo()) {
            QJsonObject result;
            result["success"] = false;
            result["message"] = "Nothing to redo";
            return createSuccessResponse(result, requestId);
        }

        undoStack->redo();

        QJsonObject result;
        result["success"] = true;
        result["message"] = "Redo operation completed";
        result["can_undo"] = undoStack->canUndo();
        result["can_redo"] = undoStack->canRedo();
        result["undo_text"] = undoStack->undoText();
        result["redo_text"] = undoStack->redoText();

        return createSuccessResponse(result, requestId);
    }, "redo", requestId);
}

QJsonObject SimulationHandler::handleGetUndoStack(const QJsonObject &params, const QJsonValue &requestId)
{
    (void)params;  // Parameter not used
    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    QUndoStack *undoStack = scene->undoStack();
    if (!undoStack) {
        return createErrorResponse("Undo stack not available", requestId);
    }

    return tryCommand([&]() -> QJsonObject {
        QJsonObject result;
        result["can_undo"] = undoStack->canUndo();
        result["can_redo"] = undoStack->canRedo();
        result["undo_text"] = undoStack->undoText();
        result["redo_text"] = undoStack->redoText();
        result["undo_limit"] = undoStack->undoLimit();
        result["undo_count"] = undoStack->count();
        result["undo_index"] = undoStack->index();

        return createSuccessResponse(result, requestId);
    }, "get undo stack info", requestId);
}

QJsonObject SimulationHandler::handleEmbedIC(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id"})) {
        return createErrorResponse("Missing required parameter: element_id", requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    int elementId = params.value("element_id").toInt();
    if (elementId <= 0) {
        return createErrorResponse("element_id must be a positive integer", requestId);
    }

    return tryCommand([&]() -> QJsonObject {
        auto *item = scene->itemById(elementId);
        if (!item) {
            return createErrorResponse(QString("Element with ID %1 not found").arg(elementId), requestId);
        }

        auto *elm = dynamic_cast<GraphicElement *>(item);
        if (!elm || elm->elementType() != ElementType::IC) {
            return createErrorResponse("Element is not an IC", requestId);
        }

        auto *ic = static_cast<IC *>(elm);

        if (ic->isEmbedded()) {
            return createErrorResponse("IC is already embedded", requestId);
        }

        if (ic->file().isEmpty()) {
            return createErrorResponse("IC has no referenced file", requestId);
        }

        const QString contextDir = scene->contextDir();
        if (contextDir.isEmpty()) {
            return createErrorResponse("Project must be saved before embedding ICs", requestId);
        }

        const QString resolvedPath = QDir(contextDir).absoluteFilePath(ic->file());
        QFile file(resolvedPath);
        if (!file.open(QIODevice::ReadOnly)) {
            return createErrorResponse(QString("Cannot read IC file: %1").arg(file.errorString()), requestId);
        }
        QByteArray fileBytes = file.readAll();
        file.close();

        QString blobName = params.value("blob_name").toString();
        if (blobName.isEmpty()) {
            blobName = QFileInfo(resolvedPath).baseName();
        }

        auto *reg = scene->icRegistry();
        if (reg->hasBlob(blobName)) {
            return createErrorResponse(QString("Blob name collision: an embedded IC named '%1' already exists").arg(blobName), requestId);
        }

        const int count = reg->embedICsByFile(ic->file(), fileBytes, blobName);

        QJsonObject result;
        result["blob_name"] = blobName;
        result["converted_count"] = count;
        result["input_count"] = ic->inputSize();
        result["output_count"] = ic->outputSize();
        result["blob_size"] = fileBytes.size();
        result["message"] = QString("Embedded %1 IC(s) as '%2'").arg(count).arg(blobName);

        return createSuccessResponse(result, requestId);
    }, "embed IC", requestId);
}

QJsonObject SimulationHandler::handleExtractIC(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"blob_name"})) {
        return createErrorResponse("Missing required parameter: blob_name", requestId);
    }

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    const QString contextDir = scene->contextDir();
    if (contextDir.isEmpty()) {
        return createErrorResponse("Project must be saved before extracting ICs", requestId);
    }

    const QString blobName = params.value("blob_name").toString();
    if (blobName.isEmpty()) {
        return createErrorResponse("blob_name must not be empty", requestId);
    }

    return tryCommand([&]() -> QJsonObject {
        auto *reg = scene->icRegistry();
        if (!reg->hasBlob(blobName)) {
            return createErrorResponse(QString("No embedded IC with blob name '%1' found").arg(blobName), requestId);
        }

        QString fileName = params.value("file_name").toString();
        if (fileName.isEmpty()) {
            fileName = QDir(contextDir).absoluteFilePath(blobName + ".panda");
        } else if (QFileInfo(fileName).isRelative()) {
            fileName = QDir(contextDir).absoluteFilePath(fileName);
        }
        if (!fileName.endsWith(".panda")) {
            fileName.append(".panda");
        }

        if (QFile::exists(fileName) && !params.value("overwrite").toBool()) {
            return createErrorResponse(QString("File '%1' already exists. Set overwrite=true to replace it.").arg(fileName), requestId);
        }

        const int count = reg->extractToFile(blobName, fileName);

        QJsonObject result;
        result["blob_name"] = blobName;
        result["file_name"] = fileName;
        result["converted_count"] = count;
        result["message"] = QString("Extracted %1 IC(s) to '%2'").arg(count).arg(fileName);

        return createSuccessResponse(result, requestId);
    }, "extract IC", requestId);
}

