// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mcpprocessor.h"

#include "mainwindow.h"
#include "mcpvalidator.h"
#include "../handlers/connectionhandler.h"
#include "../handlers/elementhandler.h"
#include "../handlers/filehandler.h"
#include "../handlers/serverinfohandler.h"
#include "../handlers/simulationhandler.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <iostream>
#include <memory>
#include <string>

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

    // Server info commands
    if (method == "get_server_info") {
        response = m_serverInfoHandler->handleCommand(method, params, requestId);
    }
    // File operation commands
    else if (method == "load_circuit" || method == "save_circuit" || method == "new_circuit" ||
             method == "close_circuit" || method == "get_tab_count" || method == "export_image" ||
             method == "export_arduino") {
        response = m_fileHandler->handleCommand(method, params, requestId);
    }
    // Element operation commands
    else if (method == "create_element" || method == "delete_element" || method == "list_elements" ||
             method == "move_element" || method == "set_element_properties" ||
             method == "set_input_value" || method == "get_output_value") {
        response = m_elementHandler->handleCommand(method, params, requestId);
    }
    // Connection operation commands
    else if (method == "connect_elements" || method == "disconnect_elements" || method == "list_connections") {
        response = m_connectionHandler->handleCommand(method, params, requestId);
    }
    // Simulation operation commands
    else if (method == "simulation_control" || method == "create_waveform" || method == "export_waveform" ||
             method == "create_ic" || method == "instantiate_ic" || method == "list_ics") {
        response = m_simulationHandler->handleCommand(method, params, requestId);
    }
    else {
        response = m_serverInfoHandler->createErrorResponse(QString("Unknown method: %1").arg(method), requestId);
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
