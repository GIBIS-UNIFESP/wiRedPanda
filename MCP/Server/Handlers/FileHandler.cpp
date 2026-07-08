// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/FileHandler.h"

#include <algorithm>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QSvgGenerator>
#include <QTabWidget>

#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/CircuitExporter.h"
#include "App/UI/MainWindow.h"

namespace {

/// Maximum padding (in scene pixels) export_image will apply around the circuit before
/// rendering. The schema only enforces a minimum of 0 (MCP/schema-mcp.json); an MCP client
/// requesting an absurd padding could otherwise blow up the render regardless of scene size.
constexpr int kMaxExportPadding = 2000;

} // namespace

FileHandler::FileHandler(MainWindow *mainWindow, const MCPValidator *validator)
    : BaseHandler(mainWindow, validator)
{
}

QJsonObject FileHandler::handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId)
{
    if (command == "load_circuit") {
        return handleLoadCircuit(params, requestId);
    } else if (command == "save_circuit") {
        return handleSaveCircuit(params, requestId);
    } else if (command == "new_circuit") {
        return handleNewCircuit(params, requestId);
    } else if (command == "close_circuit") {
        return handleCloseCircuit(params, requestId);
    } else if (command == "get_tab_count") {
        return handleGetTabCount(params, requestId);
    } else if (command == "export_image") {
        return handleExportImage(params, requestId);
    } else if (command == "export_arduino") {
        return handleExportArduino(params, requestId);
    } else if (command == "export_systemverilog") {
        return handleExportSystemVerilog(params, requestId);
    } else {
        return createErrorResponse(QString("Unknown file command: %1").arg(command),
                                   requestId, JsonRpcError::MethodNotFound);
    }
}

QJsonObject FileHandler::handleLoadCircuit(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"filename"})) {
        return createErrorResponse("Missing required parameter: filename", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("filename"), "filename", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    QString filename = params.value("filename").toString();

    if (!m_mainWindow) {
        return createErrorResponse("No main window available", requestId, JsonRpcError::InternalError);
    }

    return tryCommand([&] {
        m_mainWindow->loadPandaFile(filename);
        return createSuccessResponse(QJsonObject(), requestId);
    }, "load circuit", requestId);
}

QJsonObject FileHandler::handleSaveCircuit(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"filename"})) {
        return createErrorResponse("Missing required parameter: filename", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("filename"), "filename", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    QString filename = params.value("filename").toString();

    if (!filename.endsWith(".panda", Qt::CaseInsensitive)) {
        return createErrorResponse("Invalid file extension. Use .panda for circuit files", requestId, JsonRpcError::ValidationError);
    }

    if (!m_mainWindow) {
        return createErrorResponse("No main window available", requestId, JsonRpcError::InternalError);
    }

    return tryCommand([&] {
        m_mainWindow->save(filename);
        return createSuccessResponse(QJsonObject(), requestId);
    }, "save circuit", requestId);
}

QJsonObject FileHandler::handleNewCircuit(const QJsonObject &, const QJsonValue &requestId)
{
    if (!m_mainWindow) {
        return createErrorResponse("No main window available", requestId, JsonRpcError::InternalError);
    }

    return tryCommand([&]() -> QJsonObject {
        m_mainWindow->createNewTab();

        Scene *scene = currentScene();
        if (scene && scene->simulation()) {
            scene->simulation()->stop();
        }

        return createSuccessResponse(QJsonObject(), requestId);
    }, "create new circuit", requestId);
}

QJsonObject FileHandler::handleCloseCircuit(const QJsonObject &, const QJsonValue &requestId)
{
    if (!m_mainWindow) {
        return createErrorResponse("No main window available", requestId, JsonRpcError::InternalError);
    }

    return tryCommand([&]() -> QJsonObject {
        QTabWidget *tabWidget = m_mainWindow->findChild<QTabWidget *>("tab");
        if (!tabWidget) {
            return createErrorResponse("Tab widget not found", requestId, JsonRpcError::InternalError);
        }

        if (tabWidget->count() == 0) {
            return createErrorResponse("No tabs to close", requestId, JsonRpcError::OperationFailed);
        }

        Scene *scene = currentScene();
        if (scene && scene->simulation()) {
            scene->simulation()->stop();
        }

        int currentIndex = tabWidget->currentIndex();

        QWidget *tabWidget_widget = tabWidget->widget(currentIndex);
        tabWidget->removeTab(currentIndex);
        if (tabWidget_widget) {
            tabWidget_widget->deleteLater();
        }

        return createSuccessResponse(QJsonObject(), requestId);
    }, "close circuit", requestId);
}

QJsonObject FileHandler::handleGetTabCount(const QJsonObject &, const QJsonValue &requestId)
{
    if (!m_mainWindow) {
        return createErrorResponse("No main window available", requestId, JsonRpcError::InternalError);
    }

    return tryCommand([&]() -> QJsonObject {
        QTabWidget *tabWidget = m_mainWindow->findChild<QTabWidget *>("tab");
        if (!tabWidget) {
            return createErrorResponse("Tab widget not found", requestId, JsonRpcError::InternalError);
        }

        QJsonObject result;
        result["tab_count"] = tabWidget->count();

        return createSuccessResponse(result, requestId);
    }, "get tab count", requestId);
}

QJsonObject FileHandler::handleExportImage(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"filename", "format"})) {
        return createErrorResponse("Missing required parameters: filename, format", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("filename"), "filename", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    if (!validateNonEmptyString(params.value("format"), "format", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    QString filename = params.value("filename").toString();
    QString format = params.value("format").toString().toLower();
    const int padding = std::clamp(params.value("padding").toInt(20), 0, kMaxExportPadding);

    Scene *scene = currentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    if (format != "png" && format != "svg" && format != "pdf") {
        return createErrorResponse("Unsupported format. Use 'png', 'svg', or 'pdf'", requestId, JsonRpcError::ValidationError);
    }

    return tryCommand([&]() -> QJsonObject {
        QRectF sceneRect = scene->itemsBoundingRect();
        if (sceneRect.isEmpty()) {
            return createErrorResponse("Scene is empty - nothing to export", requestId, JsonRpcError::OperationFailed);
        }

        sceneRect.adjust(-padding, -padding, padding, padding);

        if (format == "svg") {
            QSvgGenerator generator;
            generator.setFileName(filename);
            generator.setSize(sceneRect.size().toSize());
            generator.setViewBox(sceneRect);
            generator.setTitle("wiRedPanda Circuit");
            generator.setDescription("Circuit exported from wiRedPanda");

            QPainter painter(&generator);
            if (!painter.isActive()) {
                return createErrorResponse(QString("Failed to open SVG file for writing: %1").arg(filename),
                                           requestId, JsonRpcError::FileError);
            }
            scene->render(&painter, QRectF(), sceneRect);
            painter.end();

            const QFileInfo svgInfo(filename);
            if (!svgInfo.exists() || svgInfo.size() == 0) {
                return createErrorResponse("SVG export failed: file was not written", requestId, JsonRpcError::FileError);
            }
        } else if (format == "pdf") {
            QPdfWriter pdfWriter(filename);
            pdfWriter.setTitle("wiRedPanda Circuit");
            pdfWriter.setCreator("wiRedPanda");
            // Map scene pixels 1:1 to PDF points (72 DPI) and size the page to the bounding box.
            pdfWriter.setResolution(72);
            pdfWriter.setPageSize(QPageSize(sceneRect.size(), QPageSize::Point));
            pdfWriter.setPageMargins(QMarginsF(0, 0, 0, 0));

            QPainter painter(&pdfWriter);
            if (!painter.isActive()) {
                return createErrorResponse(QString("Failed to open PDF file for writing: %1").arg(filename),
                                           requestId, JsonRpcError::FileError);
            }
            painter.setRenderHint(QPainter::Antialiasing);
            scene->render(&painter, QRectF(), sceneRect);
            painter.end();
        } else {  // png
            // Delegates to CircuitExporter's already-hardened renderer instead of sizing an
            // image directly from sceneRect: that bounding/scale-to-fit logic (capped at
            // CircuitExporter::kMaxImageDimension) exists in exactly one place so it can't drift
            // out of sync between the GUI's "Export to Image" action and this MCP command.
            const QImage image = CircuitExporter::renderScaledImage(scene, sceneRect);

            if (!image.save(filename, "PNG")) {
                return createErrorResponse("Failed to save PNG file", requestId, JsonRpcError::FileError);
            }

            QJsonObject result;
            result["exported_file"] = filename;
            result["format"] = format;
            result["size"] = QString("%1x%2").arg(image.width()).arg(image.height());

            return createSuccessResponse(result, requestId);
        }

        QJsonObject result;
        result["exported_file"] = filename;
        result["format"] = format;
        result["size"] = QString("%1x%2").arg(sceneRect.width()).arg(sceneRect.height());

        return createSuccessResponse(result, requestId);
    }, "export image", requestId);
}

QJsonObject FileHandler::handleExportArduino(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"filename"})) {
        return createErrorResponse("Missing required parameter: filename", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("filename"), "filename", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    QString filename = params.value("filename").toString();

    if (!m_mainWindow) {
        return createErrorResponse("No main window available", requestId, JsonRpcError::InternalError);
    }

    return tryCommand([&]() -> QJsonObject {
        m_mainWindow->exportToArduino(filename);

        QJsonObject result;
        result["exported_file"] = filename;
        result["format"] = "arduino";

        return createSuccessResponse(result, requestId);
    }, "export Arduino code", requestId);
}

QJsonObject FileHandler::handleExportSystemVerilog(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"filename"})) {
        return createErrorResponse("Missing required parameter: filename", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("filename"), "filename", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    QString filename = params.value("filename").toString();

    if (!m_mainWindow) {
        return createErrorResponse("No main window available", requestId, JsonRpcError::InternalError);
    }

    return tryCommand([&]() -> QJsonObject {
        m_mainWindow->exportToSystemVerilog(filename);

        QJsonObject result;
        result["exported_file"] = filename;
        result["format"] = "systemverilog";

        return createSuccessResponse(result, requestId);
    }, "export SystemVerilog code", requestId);
}
