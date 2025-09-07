// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filehandler.h"

#include "mainwindow.h"
#include "scene.h"
#include "workspace.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPixmap>
#include <QSvgGenerator>
#include <QTabWidget>
#include <QPainter>

FileHandler::FileHandler(MainWindow *mainWindow, MCPValidator *validator)
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
    } else {
        return createErrorResponse(QString("Unknown file command: %1").arg(command), requestId);
    }
}

QJsonObject FileHandler::handleLoadCircuit(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"filename"})) {
        return createErrorResponse("Missing required parameter: filename", requestId);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("filename"), "filename", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    QString filename = params.value("filename").toString();

    if (!m_mainWindow) {
        return createErrorResponse("No main window available", requestId);
    }

    try {
        m_mainWindow->loadPandaFile(filename);
        return createSuccessResponse(QJsonObject(), requestId);
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to load circuit: %1").arg(e.what()), requestId);
    }
}

QJsonObject FileHandler::handleSaveCircuit(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"filename"})) {
        return createErrorResponse("Missing required parameter: filename", requestId);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("filename"), "filename", errorMsg)) {
        return createErrorResponse(errorMsg, requestId);
    }

    QString filename = params.value("filename").toString();

    if (!filename.endsWith(".panda", Qt::CaseInsensitive)) {
        return createErrorResponse("Invalid file extension. Use .panda for circuit files", requestId);
    }

    if (!m_mainWindow) {
        return createErrorResponse("No main window available", requestId);
    }

    try {
        m_mainWindow->save(filename);
        return createSuccessResponse(QJsonObject(), requestId);
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to save circuit: %1").arg(e.what()), requestId);
    }
}

QJsonObject FileHandler::handleNewCircuit(const QJsonObject &, const QJsonValue &requestId)
{
    if (!m_mainWindow) {
        return createErrorResponse("No main window available", requestId);
    }

    try {
        m_mainWindow->createNewTab();

        Scene *scene = getCurrentScene();
        if (scene && scene->simulation()) {
            scene->simulation()->stop();
        }

        return createSuccessResponse(QJsonObject(), requestId);
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to create new circuit: %1").arg(e.what()), requestId);
    }
}

QJsonObject FileHandler::handleCloseCircuit(const QJsonObject &, const QJsonValue &requestId)
{
    if (!m_mainWindow) {
        return createErrorResponse("No main window available", requestId);
    }

    try {
        QTabWidget *tabWidget = m_mainWindow->findChild<QTabWidget*>("tab");
        if (!tabWidget) {
            return createErrorResponse("Tab widget not found", requestId);
        }

        if (tabWidget->count() == 0) {
            return createErrorResponse("No tabs to close", requestId);
        }

        Scene *scene = getCurrentScene();
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
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to close circuit: %1").arg(e.what()), requestId);
    }
}

QJsonObject FileHandler::handleGetTabCount(const QJsonObject &, const QJsonValue &requestId)
{
    if (!m_mainWindow) {
        return createErrorResponse("No main window available", requestId);
    }

    try {
        QTabWidget *tabWidget = m_mainWindow->findChild<QTabWidget*>("tab");
        if (!tabWidget) {
            return createErrorResponse("Tab widget not found", requestId);
        }

        QJsonObject result;
        result["tab_count"] = tabWidget->count();

        return createSuccessResponse(result, requestId);
    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to get tab count: %1").arg(e.what()), requestId);
    }
}

QJsonObject FileHandler::handleExportImage(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"filename", "format"})) {
        return createErrorResponse("Missing required parameters: filename, format", requestId);
    }

    QString filename = params.value("filename").toString();
    QString format = params.value("format").toString().toLower();
    int padding = params.value("padding").toInt(20);

    Scene *scene = getCurrentScene();
    if (!scene) {
        return createErrorResponse("No active circuit scene available", requestId);
    }

    if (format != "png" && format != "svg" && format != "pdf") {
        return createErrorResponse("Unsupported format. Use 'png', 'svg', or 'pdf'", requestId);
    }

    try {
        QRectF sceneRect = scene->itemsBoundingRect();
        if (sceneRect.isEmpty()) {
            return createErrorResponse("Scene is empty - nothing to export", requestId);
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
            scene->render(&painter, QRectF(), sceneRect);
            painter.end();
        } else {
            QPixmap pixmap(sceneRect.size().toSize());
            pixmap.fill(Qt::white);

            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            scene->render(&painter, QRectF(), sceneRect);
            painter.end();

            if (format == "png") {
                if (!pixmap.save(filename, "PNG")) {
                    return createErrorResponse("Failed to save PNG file", requestId);
                }
            } else if (format == "pdf") {
                return createErrorResponse("PDF export not yet implemented - use PNG or SVG", requestId);
            }
        }

        QJsonObject result;
        result["exported_file"] = filename;
        result["format"] = format;
        result["size"] = QString("%1x%2").arg(sceneRect.width()).arg(sceneRect.height());

        return createSuccessResponse(result, requestId);

    } catch (const std::exception &e) {
        return createErrorResponse(QString("Failed to export image: %1").arg(e.what()), requestId);
    }
}
