// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/ICHandler.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>

#include "App/Core/Constants.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Commands.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MainWindow.h"

namespace {

/// Returns \c true if \a name is a single path component with no directory traversal —
/// safe to use as-is inside a filesystem path built from MCP-client-supplied input, which
/// (unlike the GUI's getSaveFileName()/getOpenFileName() dialogs) has no human confirming
/// the resulting location before a file gets read or written.
bool isBareFileName(const QString &name)
{
    return name != "." && name != ".." && QFileInfo(name).fileName() == name
        && !name.contains('/') && !name.contains('\\');
}

} // namespace

ICHandler::ICHandler(MainWindow *mainWindow, const MCPValidator *validator)
    : BaseHandler(mainWindow, validator)
{
}

QJsonObject ICHandler::handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId)
{
    if (command == "create_ic") {
        return handleCreateIC(params, requestId);
    } else if (command == "instantiate_ic") {
        return handleInstantiateIC(params, requestId);
    } else if (command == "list_ics") {
        return handleListICs(params, requestId);
    } else if (command == "embed_ic") {
        return handleEmbedIC(params, requestId);
    } else if (command == "extract_ic") {
        return handleExtractIC(params, requestId);
    } else {
        return createErrorResponse(QString("Unknown IC command: %1").arg(command),
                                   requestId, JsonRpcError::MethodNotFound);
    }
}

QJsonObject ICHandler::handleCreateIC(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"name"})) {
        return createErrorResponse("Missing required parameter: name", requestId, JsonRpcError::InvalidParams);
    }

    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    QString name = params.value("name").toString();
    QString description = params.value("description").toString("");

    if (name.isEmpty()) {
        return createErrorResponse("IC name cannot be empty", requestId, JsonRpcError::InvalidParams);
    }

    if (!isBareFileName(name)) {
        return createErrorResponse("IC name must not contain path separators or directory components",
                                   requestId, JsonRpcError::InvalidParams);
    }

    return tryCommand([&]() -> QJsonObject {
        QString icFileName = name + ".panda";
        QString fullPath = m_mainWindow->currentDir().absoluteFilePath(icFileName);

        if (QFile::exists(fullPath)) {
            return createErrorResponse(QString("IC file already exists: %1").arg(icFileName),
                                       requestId, JsonRpcError::IcError);
        }

        const auto elements = scene->elements();
        if (elements.isEmpty()) {
            return createErrorResponse("Cannot create IC from empty circuit", requestId, JsonRpcError::IcError);
        }

        auto *workspace = m_mainWindow->currentTab();
        if (!workspace) {
            return createErrorResponse("No active workspace available", requestId, JsonRpcError::InternalError);
        }

        if (workspace->save(fullPath) == WorkSpace::SaveOutcome::ReadOnlyTarget) {
            return createErrorResponse(QString("Cannot write IC file (target location is read-only): %1").arg(fullPath),
                                       requestId, JsonRpcError::IcError);
        }

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

QJsonObject ICHandler::handleInstantiateIC(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"ic_name", "x", "y"})) {
        return createErrorResponse("Missing required parameters: ic_name, x, y", requestId, JsonRpcError::InvalidParams);
    }

    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    QString icName = params.value("ic_name").toString();
    if (icName.isEmpty()) {
        return createErrorResponse("Parameter 'ic_name' cannot be empty", requestId, JsonRpcError::InvalidParams);
    }

    // Two ways to reference a dependency: a bare name (interactive/GUI use, resolved
    // against the current tab's directory) or an absolute path (automation clients,
    // e.g. the Python IC generators, which run one headless process per script with
    // no reliable "current directory" to resolve a bare name against). Absolute paths
    // still can't contain '..' components, so this doesn't relax path-traversal safety.
    const bool isAbsolute = QFileInfo(icName).isAbsolute();
    if (isAbsolute) {
        if (icName.contains("..")) {
            return createErrorResponse("IC path must not contain '..' components",
                                       requestId, JsonRpcError::InvalidParams);
        }
    } else if (!isBareFileName(icName)) {
        return createErrorResponse("IC name must not contain path separators or directory components",
                                   requestId, JsonRpcError::InvalidParams);
    }

    const int snap = Constants::gridSize / 2;
    int x = qRound(params.value("x").toDouble() / snap) * snap;
    int y = qRound(params.value("y").toDouble() / snap) * snap;
    QString label = params.value("label").toString(icName);

    return tryCommand([&]() -> QJsonObject {
        QString icFileName = icName + ".panda";
        QString fullPath = isAbsolute ? icFileName : m_mainWindow->currentDir().absoluteFilePath(icFileName);

        if (!QFile::exists(fullPath)) {
            return createErrorResponse(QString("IC file not found: %1").arg(icFileName),
                                       requestId, JsonRpcError::IcError);
        }

        auto ic = std::make_unique<IC>();

        const QString icDirectory = QFileInfo(fullPath).absolutePath();
        const bool inlineMode = params.value("inline").toBool(false);

        IC *icPtr = nullptr;

        if (inlineMode) {
            QFile file(fullPath);
            if (!file.open(QIODevice::ReadOnly)) {
                return createErrorResponse(QString("Could not read IC file: %1").arg(file.errorString()),
                                           requestId, JsonRpcError::FileError);
            }
            QByteArray fileBytes = file.readAll();
            file.close();

            QString blobName = params.value("blob_name").toString();
            if (blobName.isEmpty()) {
                blobName = QFileInfo(fullPath).baseName();
            } else if (!isBareFileName(blobName)) {
                // Same reasoning as handleEmbedIC: this blob_name is stored verbatim as a
                // registry key and can later be used as a file-name fallback by extract_ic.
                return createErrorResponse("blob_name must not contain path separators or directory components",
                                           requestId, JsonRpcError::InvalidParams);
            }

            auto *reg = scene->icRegistry();
            if (reg->hasBlob(blobName)) {
                return createErrorResponse(QString("Blob name collision: an embedded IC named '%1' already exists. "
                                                   "Use blob_name parameter to specify a different name.").arg(blobName),
                                           requestId, JsonRpcError::IcError);
            }

            icPtr = reg->createEmbeddedIC(blobName, fileBytes, icDirectory);
        } else {
            // loadFile() can throw (nesting depth, circular reference, bad file); release()
            // only after it succeeds so the unique_ptr cleans up on the throwing path instead
            // of leaking, mirroring SceneDropHandler.cpp's identical loadFromDrop() sequencing.
            ic->loadFile(fullPath, icDirectory);
            icPtr = ic.release();
            scene->receiveCommand(new AddItemsCommand({icPtr}, scene));
        }

        icPtr->setPos(x, y);
        icPtr->setLabel(label);

        const QRectF bounds = icPtr->boundingRect();

        QJsonObject result;
        result["element_id"] = icPtr->id();
        result["ic_name"] = icName;
        result["filename"] = icFileName;
        result["label"] = label;
        result["position"] = QJsonObject{{"x", x}, {"y", y}};
        result["input_count"] = icPtr->inputSize();
        result["output_count"] = icPtr->outputSize();
        result["width"] = bounds.width();
        result["height"] = bounds.height();
        if (inlineMode) {
            result["inline"] = true;
            result["blob_name"] = icPtr->blobName();
        }
        result["message"] = "IC instantiated successfully";

        return createSuccessResponse(result, requestId);
    }, "instantiate IC", requestId);
}

QJsonObject ICHandler::handleListICs(const QJsonObject &, const QJsonValue &requestId)
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

QJsonObject ICHandler::handleEmbedIC(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"element_id"})) {
        return createErrorResponse("Missing required parameter: element_id", requestId, JsonRpcError::InvalidParams);
    }

    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    int elementId = params.value("element_id").toInt();
    if (elementId <= 0) {
        return createErrorResponse("element_id must be a positive integer", requestId, JsonRpcError::InvalidParams);
    }

    return tryCommand([&]() -> QJsonObject {
        auto *item = scene->itemById(elementId);
        if (!item) {
            return createErrorResponse(QString("Element with ID %1 not found").arg(elementId),
                                       requestId, JsonRpcError::ElementNotFound);
        }

        auto *elm = dynamic_cast<GraphicElement *>(item);
        if (!elm || elm->elementType() != ElementType::IC) {
            return createErrorResponse("Element is not an IC", requestId, JsonRpcError::ValidationError);
        }

        auto *ic = static_cast<IC *>(elm);

        if (ic->isEmbedded()) {
            return createErrorResponse("IC is already embedded", requestId, JsonRpcError::IcError);
        }

        if (ic->file().isEmpty()) {
            return createErrorResponse("IC has no referenced file", requestId, JsonRpcError::IcError);
        }

        const QString contextDir = scene->contextDir();
        if (contextDir.isEmpty()) {
            return createErrorResponse("Project must be saved before embedding ICs", requestId, JsonRpcError::IcError);
        }

        const QString resolvedPath = QDir(contextDir).absoluteFilePath(ic->file());
        QFile file(resolvedPath);
        if (!file.open(QIODevice::ReadOnly)) {
            return createErrorResponse(QString("Cannot read IC file: %1").arg(file.errorString()),
                                       requestId, JsonRpcError::FileError);
        }
        QByteArray fileBytes = file.readAll();
        file.close();

        QString blobName = params.value("blob_name").toString();
        if (blobName.isEmpty()) {
            blobName = QFileInfo(resolvedPath).baseName();
        } else if (!isBareFileName(blobName)) {
            // blob_name is later used verbatim as a file-name fallback by extract_ic
            // (ICHandler::handleExtractIC) — reject path traversal here too, not just there,
            // so a malicious name can't ride through the registry as a stored blob key.
            return createErrorResponse("blob_name must not contain path separators or directory components",
                                       requestId, JsonRpcError::InvalidParams);
        }

        auto *reg = scene->icRegistry();
        if (reg->hasBlob(blobName)) {
            return createErrorResponse(QString("Blob name collision: an embedded IC named '%1' already exists").arg(blobName),
                                       requestId, JsonRpcError::IcError);
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

QJsonObject ICHandler::handleExtractIC(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"blob_name"})) {
        return createErrorResponse("Missing required parameter: blob_name", requestId, JsonRpcError::InvalidParams);
    }

    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    const QString contextDir = scene->contextDir();
    if (contextDir.isEmpty()) {
        return createErrorResponse("Project must be saved before extracting ICs", requestId, JsonRpcError::IcError);
    }

    const QString blobName = params.value("blob_name").toString();
    if (blobName.isEmpty()) {
        return createErrorResponse("blob_name must not be empty", requestId, JsonRpcError::InvalidParams);
    }

    return tryCommand([&]() -> QJsonObject {
        auto *reg = scene->icRegistry();
        if (!reg->hasBlob(blobName)) {
            return createErrorResponse(QString("No embedded IC with blob name '%1' found").arg(blobName),
                                       requestId, JsonRpcError::IcError);
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

        // file_name is an MCP-client-supplied identifier, not a GUI-picked path (unlike
        // ICController::extractSelectedIC, which always routes through an interactive
        // getSaveFileName dialog a human must approve) — an absolute file_name or one with
        // ".." components would otherwise let the write above land anywhere on disk. Validate
        // the final resolved path rather than the individual inputs that built it, so this
        // can't be bypassed by some other future parameter combination; subdirectories of
        // contextDir remain allowed, only escapes are rejected.
        const QString cleanContextDir = QDir::cleanPath(QDir(contextDir).absolutePath());
        const QString cleanFileName = QDir::cleanPath(fileName);
        if (cleanFileName != cleanContextDir && !cleanFileName.startsWith(cleanContextDir + '/')) {
            return createErrorResponse("file_name must resolve to a location inside the project directory",
                                       requestId, JsonRpcError::InvalidParams);
        }
        fileName = cleanFileName;

        if (QFile::exists(fileName) && !params.value("overwrite").toBool()) {
            return createErrorResponse(QString("File '%1' already exists. Set overwrite=true to replace it.").arg(fileName),
                                       requestId, JsonRpcError::FileError);
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
