// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Core/MCPProcessor.h"

#include <iostream>
#include <memory>
#include <string>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>

#include "App/UI/MainWindow.h"
#include "MCP/Server/Core/MCPValidator.h"
#include "MCP/Server/Handlers/BaseHandler.h"
#include "MCP/Server/Handlers/ConnectionHandler.h"
#include "MCP/Server/Handlers/ElementHandler.h"
#include "MCP/Server/Handlers/FileHandler.h"
#include "MCP/Server/Handlers/ServerInfoHandler.h"
#include "MCP/Server/Handlers/SimulationHandler.h"

#ifdef _WIN32
#define fileno _fileno
#endif

// StdinReader implementation
StdinReader::StdinReader(QObject *parent)
    : QThread(parent)
{
}

void StdinReader::requestStop()
{
    m_stopRequested = true;
}

void StdinReader::run()
{
    std::string line;
    while (!m_stopRequested && std::getline(std::cin, line)) {
        if (!line.empty()) {
            emit dataReceived(QString::fromStdString(line));
        }

        // Small yield to allow graceful shutdown
        QThread::msleep(1);
    }
}

MCPProcessor::MCPProcessor(MainWindow *mainWindow, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_validator(std::make_unique<MCPValidator>(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("schema-mcp.json")))
    , m_stdin(stdin)
    , m_stdout(stdout)
    , m_stdinReader(new StdinReader(this))
    , m_serverInfoHandler(std::make_unique<ServerInfoHandler>(mainWindow, m_validator.get()))
    , m_fileHandler(std::make_unique<FileHandler>(mainWindow, m_validator.get()))
    , m_elementHandler(std::make_unique<ElementHandler>(mainWindow, m_validator.get()))
    , m_connectionHandler(std::make_unique<ConnectionHandler>(mainWindow, m_validator.get()))
    , m_simulationHandler(std::make_unique<SimulationHandler>(mainWindow, m_validator.get()))
{
    // Build the method → handler dispatch map
    const auto addRoutes = [&](BaseHandler *handler, const QStringList &methods) {
        for (const auto &method : methods) {
            m_dispatchMap.insert(method, handler);
        }
    };
    addRoutes(m_serverInfoHandler.get(), {"get_server_info"});
    addRoutes(m_fileHandler.get(), {
        "load_circuit", "save_circuit", "new_circuit", "close_circuit",
        "get_tab_count", "export_image"
    });
    addRoutes(m_elementHandler.get(), {
        "create_element", "delete_element", "list_elements", "move_element",
        "set_element_properties", "set_input_value", "get_output_value",
        "rotate_element", "flip_element", "update_element",
        "change_input_size", "change_output_size",
        "toggle_truth_table_output", "morph_element"
    });
    addRoutes(m_connectionHandler.get(), {
        "connect_elements", "disconnect_elements", "list_connections", "split_connection"
    });
    addRoutes(m_simulationHandler.get(), {
        "simulation_control", "create_waveform", "export_waveform",
        "create_ic", "instantiate_ic", "list_ics",
        "undo", "redo", "get_undo_stack"
    });

    // Set up event-driven stdin reading
    connect(m_stdinReader, &StdinReader::dataReceived, this, &MCPProcessor::processIncomingData);
}

MCPProcessor::~MCPProcessor()
{
    stopProcessing();
    // Smart pointers automatically clean up handlers and validator
}

void MCPProcessor::startProcessing()
{
    // Start processing without sending automatic messages - follow MCP protocol
    m_stdinReader->start();
}

void MCPProcessor::stopProcessing()
{
    if (m_stdinReader->isRunning()) {
        m_stdinReader->requestStop();
        if (!m_stdinReader->wait(3000)) {
            // Force terminate if graceful shutdown fails
            m_stdinReader->terminate();
            m_stdinReader->wait(1000);
        }
    }
}

void MCPProcessor::processIncomingData(const QString &line)
{
    // Process the line received from the stdin reader thread
    QString trimmedLine = line.trimmed();
    if (!trimmedLine.isEmpty()) {
        processCommand(trimmedLine);
    }
}

void MCPProcessor::processCommand(const QString &line)
{
    // Parse JSON command
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        sendResponse(m_serverInfoHandler->createErrorResponse(QString("Invalid JSON: %1").arg(parseError.errorString())));
        return;
    }

    QJsonObject request = doc.object();

    // Extract request ID early for proper JSON-RPC 2.0 error responses
    QJsonValue requestId = request.value("id");

    // JSON-RPC 2.0 format validation
    QString jsonrpc = request.value("jsonrpc").toString();
    if (jsonrpc != "2.0") {
        sendResponse(m_serverInfoHandler->createErrorResponse("Invalid or missing jsonrpc version", requestId));
        return;
    }

    // Schema validation
    if (m_validator) {
        ValidationResult validationResult = m_validator->validateCommand(request);
        if (!validationResult.isValid) {
            QString detailedError = validationResult.errorMessage;

            if (!validationResult.errorPath.isEmpty()) {
                detailedError += QString(" (at path: %1)").arg(validationResult.errorPath);
            }

            sendResponse(m_serverInfoHandler->createErrorResponse(QString("Schema validation failed: %1").arg(detailedError), requestId));
            return;
        }
    }

    QString method = request.value("method").toString();
    QJsonObject params = request.value("params").toObject();

    // Route command to appropriate handler using delegation pattern
    QJsonObject response;

    try {
        if (auto *handler = m_dispatchMap.value(method)) {
            response = handler->handleCommand(method, params, requestId);
        } else {
            response = m_serverInfoHandler->createErrorResponse(QString("Unknown method: %1").arg(method), requestId);
        }
    } catch (const std::exception &e) {
        response = m_serverInfoHandler->createErrorResponse(QString("Internal error: %1").arg(e.what()), requestId);
    } catch (...) {
        response = m_serverInfoHandler->createErrorResponse("Internal error: unknown exception", requestId);
    }

    // Validate and send response
    if (m_validator) {
        ValidationResult validationResult = m_validator->validateResponse(response, method);
        if (!validationResult.isValid) {
            QString detailedError = validationResult.errorMessage;
            if (!validationResult.errorPath.isEmpty()) {
                detailedError += QString(" (at path: %1)").arg(validationResult.errorPath);
            }
            QJsonObject errorResponse = m_serverInfoHandler->createErrorResponse(QString("Internal validation error: %1").arg(detailedError), requestId);
            sendResponse(errorResponse);
            return;
        }
    }

    sendResponse(response);
}

void MCPProcessor::sendResponse(const QJsonObject &response)
{
    QJsonDocument doc(response);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    m_stdout << QString::fromUtf8(data) << Qt::endl;
    m_stdout.flush();
}

