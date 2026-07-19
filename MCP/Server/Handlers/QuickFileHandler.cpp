// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/QuickFileHandler.h"

#include <algorithm>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QSvgGenerator>

#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/Simulation/Simulation.h"

namespace {

/// \copydoc kMaxExportPadding in FileHandler.cpp
constexpr int kMaxExportPadding = 2000;

} // namespace

QuickFileHandler::QuickFileHandler(QuickAppController *appController, const MCPValidator *validator)
    : QuickBaseHandler(appController, validator)
{
}

QJsonObject QuickFileHandler::handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId)
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

QJsonObject QuickFileHandler::handleLoadCircuit(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"filename"})) {
        return createErrorResponse("Missing required parameter: filename", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("filename"), "filename", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    QString filename = params.value("filename").toString();

    if (!m_appController) {
        return createErrorResponse("No app controller available", requestId, JsonRpcError::InternalError);
    }

    return tryCommand([&] {
        m_appController->openRecentFile(filename);
        return createSuccessResponse(QJsonObject(), requestId);
    }, "load circuit", requestId);
}

QJsonObject QuickFileHandler::handleSaveCircuit(const QJsonObject &params, const QJsonValue &requestId)
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

    if (!m_appController) {
        return createErrorResponse("No app controller available", requestId, JsonRpcError::InternalError);
    }

    return tryCommand([&] {
        m_appController->saveCurrentTabAs(filename);
        return createSuccessResponse(QJsonObject(), requestId);
    }, "save circuit", requestId);
}

QJsonObject QuickFileHandler::handleNewCircuit(const QJsonObject &, const QJsonValue &requestId)
{
    if (!m_appController) {
        return createErrorResponse("No app controller available", requestId, JsonRpcError::InternalError);
    }

    return tryCommand([&]() -> QJsonObject {
        m_appController->newTab();

        CanvasItem *canvas = currentCanvas();
        if (canvas && canvas->simulation()) {
            canvas->simulation()->stop();
        }

        return createSuccessResponse(QJsonObject(), requestId);
    }, "create new circuit", requestId);
}

QJsonObject QuickFileHandler::handleCloseCircuit(const QJsonObject &, const QJsonValue &requestId)
{
    if (!m_appController) {
        return createErrorResponse("No app controller available", requestId, JsonRpcError::InternalError);
    }

    return tryCommand([&]() -> QJsonObject {
        if (m_appController->tabCount() == 0) {
            return createErrorResponse("No tabs to close", requestId, JsonRpcError::OperationFailed);
        }

        CanvasItem *canvas = currentCanvas();
        if (canvas && canvas->simulation()) {
            canvas->simulation()->stop();
        }

        // removeTabWithoutPrompt(), not closeTab(): MCP mode is non-interactive, and closeTab()
        // would block forever on a save-confirmation dialog with no user to answer it if the
        // tab is modified -- mirrors FileHandler.cpp's original QTabWidget-direct-manipulation
        // bypass of WorkspaceManager's own interactive close path, for the same reason.
        m_appController->removeTabWithoutPrompt(m_appController->currentIndex());

        return createSuccessResponse(QJsonObject(), requestId);
    }, "close circuit", requestId);
}

QJsonObject QuickFileHandler::handleGetTabCount(const QJsonObject &, const QJsonValue &requestId)
{
    if (!m_appController) {
        return createErrorResponse("No app controller available", requestId, JsonRpcError::InternalError);
    }

    return tryCommand([&]() -> QJsonObject {
        QJsonObject result;
        result["tab_count"] = m_appController->tabCount();

        return createSuccessResponse(result, requestId);
    }, "get tab count", requestId);
}

QJsonObject QuickFileHandler::handleExportImage(const QJsonObject &params, const QJsonValue &requestId)
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

    CanvasItem *canvas = currentCanvas();
    if (!canvas) {
        return createErrorResponse("No active circuit scene available", requestId, JsonRpcError::SceneNotAvailable);
    }

    if (format != "png" && format != "svg" && format != "pdf") {
        return createErrorResponse("Unsupported format. Use 'png', 'svg', or 'pdf'", requestId, JsonRpcError::ValidationError);
    }

    return tryCommand([&]() -> QJsonObject {
        QRectF contentRect = canvas->elementsBoundingRect();
        if (contentRect.isEmpty()) {
            return createErrorResponse("Scene is empty - nothing to export", requestId, JsonRpcError::OperationFailed);
        }

        contentRect.adjust(-padding, -padding, padding, padding);

        if (format == "svg") {
            QSvgGenerator generator;
            generator.setFileName(filename);
            generator.setSize(contentRect.size().toSize());
            generator.setViewBox(contentRect);
            generator.setTitle("wiRedPanda Circuit");
            generator.setDescription("Circuit exported from wiRedPanda");

            QPainter painter(&generator);
            if (!painter.isActive()) {
                return createErrorResponse(QString("Failed to open SVG file for writing: %1").arg(filename),
                                           requestId, JsonRpcError::FileError);
            }
            // painter.viewport() after the QPainter is constructed against the generator is the
            // generator's own full device rect -- same idiom CanvasItem::exportToPdf() uses for
            // its own QPrinter-backed painter, reproducing QGraphicsScene::render(painter,
            // QRectF(), source)'s "target defaults to the whole device" behavior without needing
            // QGraphicsScene::render() itself.
            canvas->paintElementsInto(&painter, QRectF(painter.viewport()), contentRect);
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
            pdfWriter.setPageSize(QPageSize(contentRect.size(), QPageSize::Point));
            pdfWriter.setPageMargins(QMarginsF(0, 0, 0, 0));

            QPainter painter(&pdfWriter);
            if (!painter.isActive()) {
                return createErrorResponse(QString("Failed to open PDF file for writing: %1").arg(filename),
                                           requestId, JsonRpcError::FileError);
            }
            painter.setRenderHint(QPainter::Antialiasing);
            canvas->paintElementsInto(&painter, QRectF(painter.viewport()), contentRect);
            painter.end();
        } else {  // png
            // Delegates to CanvasItem's own already-hardened renderer (mirrors
            // CircuitExporter::renderScaledImage()'s exact bounding/scale-to-fit/cap contract --
            // see renderExportImage()'s own doc comment) instead of sizing an image directly
            // from contentRect, so that logic can't drift out of sync between the GUI's future
            // "Export to Image" action and this MCP command.
            const QImage image = canvas->renderExportImage(contentRect);

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
        result["size"] = QString("%1x%2").arg(contentRect.width()).arg(contentRect.height());

        return createSuccessResponse(result, requestId);
    }, "export image", requestId);
}

QJsonObject QuickFileHandler::handleExportArduino(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"filename"})) {
        return createErrorResponse("Missing required parameter: filename", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("filename"), "filename", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    QString filename = params.value("filename").toString();

    if (!m_appController) {
        return createErrorResponse("No app controller available", requestId, JsonRpcError::InternalError);
    }

    return tryCommand([&]() -> QJsonObject {
        m_appController->exportArduinoTo(filename);

        QJsonObject result;
        result["exported_file"] = filename;
        result["format"] = "arduino";

        return createSuccessResponse(result, requestId);
    }, "export Arduino code", requestId);
}

QJsonObject QuickFileHandler::handleExportSystemVerilog(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"filename"})) {
        return createErrorResponse("Missing required parameter: filename", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("filename"), "filename", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    QString filename = params.value("filename").toString();

    if (!m_appController) {
        return createErrorResponse("No app controller available", requestId, JsonRpcError::InternalError);
    }

    return tryCommand([&]() -> QJsonObject {
        m_appController->exportSystemVerilogTo(filename);

        QJsonObject result;
        result["exported_file"] = filename;
        result["format"] = "systemverilog";

        return createSuccessResponse(result, requestId);
    }, "export SystemVerilog code", requestId);
}
