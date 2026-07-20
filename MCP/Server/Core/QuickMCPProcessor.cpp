// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Core/QuickMCPProcessor.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

#include "App/QuickShell/Chrome/QuickAppController.h"
#include "MCP/Server/Core/JsonRpcError.h"
#include "MCP/Server/Core/MCPValidator.h"
#include "MCP/Server/Core/StdinLineReader.h"
#include "MCP/Server/Handlers/QuickConnectionHandler.h"
#include "MCP/Server/Handlers/QuickElementHandler.h"
#include "MCP/Server/Handlers/QuickFileHandler.h"
#include "MCP/Server/Handlers/QuickHistoryHandler.h"
#include "MCP/Server/Handlers/QuickICHandler.h"
#include "MCP/Server/Handlers/QuickSimulationHandler.h"
#include "MCP/Server/Handlers/ServerInfoHandler.h"
#include "MCP/Server/Handlers/ThemeHandler.h"

#ifdef Q_OS_WIN
#  include <windows.h>
#else
#  include <cerrno>
#  include <fcntl.h>
#  include <unistd.h>

#  include <QSocketNotifier>
#endif

QuickMCPProcessor::QuickMCPProcessor(QuickAppController *appController, QObject *parent)
    : QObject(parent)
    , m_appController(appController)
    , m_validator(std::make_unique<MCPValidator>(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("schema-mcp.json")))
    , m_stdout(stdout)
    // ServerInfoHandler/ThemeHandler need no MainWindow/Scene access at all (confirmed by
    // reading both .cpp files in full) -- constructed with a null MainWindow*, same as every
    // other BaseHandler-derived class's inherited constructor accepts.
    , m_serverInfoHandler(std::make_unique<ServerInfoHandler>(nullptr, m_validator.get()))
    , m_fileHandler(std::make_unique<QuickFileHandler>(appController, m_validator.get()))
    , m_elementHandler(std::make_unique<QuickElementHandler>(appController, m_validator.get()))
    , m_connectionHandler(std::make_unique<QuickConnectionHandler>(appController, m_validator.get()))
    , m_simulationHandler(std::make_unique<QuickSimulationHandler>(appController, m_validator.get()))
    , m_icHandler(std::make_unique<QuickICHandler>(appController, m_validator.get()))
    , m_historyHandler(std::make_unique<QuickHistoryHandler>(appController, m_validator.get()))
    , m_themeHandler(std::make_unique<ThemeHandler>(nullptr, m_validator.get()))
{
    // Build the method → handler dispatch map -- same method-name/handler groupings as
    // MCPProcessor's own constructor (the MCP command vocabulary is shared, not per-app).
    const auto addRoutes = [&](BaseHandler *handler, const QStringList &methods) {
        for (const auto &method : methods) {
            m_dispatchMap.insert(method, handler);
        }
    };
    addRoutes(m_serverInfoHandler.get(), {"get_server_info", "list_commands", "describe_command"});
    addRoutes(m_fileHandler.get(), {
        "load_circuit", "save_circuit", "new_circuit", "close_circuit",
        "get_tab_count", "export_image",
        "export_arduino", "export_systemverilog"
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
        "simulation_control", "create_waveform", "export_waveform"
    });
    addRoutes(m_icHandler.get(), {
        "create_ic", "instantiate_ic", "list_ics",
        "embed_ic", "extract_ic"
    });
    addRoutes(m_historyHandler.get(), {
        "undo", "redo", "get_undo_stack"
    });
    addRoutes(m_themeHandler.get(), {
        "get_theme", "set_theme", "get_effective_theme"
    });

    if (!m_validator->isSchemaLoaded()) {
        qWarning() << "QuickMCPProcessor: MCP schema not loaded — all requests will be rejected";
    }
}

QuickMCPProcessor::~QuickMCPProcessor()
{
    stopProcessing();
    // Smart pointers automatically clean up handlers and validator
}

void QuickMCPProcessor::startProcessing()
{
    // Start processing without sending automatic messages - follow MCP protocol. Mirrors
    // MCPProcessor::startProcessing() exactly, reusing the same StdinReader class on Windows
    // (a public, MainWindow-independent member of MCPProcessor.h).
#ifdef Q_OS_WIN
    m_stdinReader = new StdinReader(this);
    connect(m_stdinReader, &StdinReader::dataReceived, this, &QuickMCPProcessor::processIncomingData);
    // EOF (client closed stdin) ends the thread — shut the processor down too.
    connect(m_stdinReader, &StdinReader::finished, qApp, &QCoreApplication::quit);
    m_stdinReader->start();
#else
    // Event-driven, no thread: watch stdin in the main event loop. The fd is
    // made non-blocking so onStdinReadable() can drain it without ever parking.
    const int fd = ::fileno(stdin);
    const int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags != -1) {
        ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
    m_stdinNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_stdinNotifier, &QSocketNotifier::activated, this, &QuickMCPProcessor::onStdinReadable);
    m_stdinNotifier->setEnabled(true);
#endif
}

void QuickMCPProcessor::stopProcessing()
{
#ifdef Q_OS_WIN
    if (m_stdinReader && m_stdinReader->isRunning()) {
        m_stdinReader->requestStop();
        // Close the stdin handle to unblock the thread parked in getline so it
        // returns on EOF — no QThread::terminate() (which leaks locks/state).
        CloseHandle(GetStdHandle(STD_INPUT_HANDLE));
        m_stdinReader->wait();
    }
#else
    if (m_stdinNotifier) {
        m_stdinNotifier->setEnabled(false);
    }
#endif
}

#ifndef Q_OS_WIN
void QuickMCPProcessor::onStdinReadable()
{
    char buffer[4096];
    for (;;) {
        const ssize_t bytesRead = ::read(::fileno(stdin), buffer, sizeof(buffer));

        if (bytesRead > 0) {
            const QByteArray chunk(buffer, static_cast<qsizetype>(bytesRead));
            // extractStdinLines() is a pure static method, independent of any live file
            // descriptor -- reused directly from MCPProcessor rather than duplicated.
            const QStringList lines = StdinLineReader::extractLines(m_stdinBuffer, chunk);
            for (const QString &line : lines) {
                processIncomingData(line);
            }
            continue; // drain whatever else is buffered before yielding
        }

        if (bytesRead == 0) {
            // EOF: the client closed stdin — shut the processor down cleanly.
            m_stdinNotifier->setEnabled(false);
            QCoreApplication::quit();
            return;
        }

        // bytesRead < 0
#if defined(EWOULDBLOCK) && (EWOULDBLOCK != EAGAIN)
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
#else
        if (errno == EAGAIN) { // EWOULDBLOCK == EAGAIN on this platform
#endif
            return; // no more data right now; wait for the next activation
        }
        if (errno == EINTR) {
            continue; // interrupted by a signal; retry
        }
        // Unrecoverable read error: stop watching and quit.
        m_stdinNotifier->setEnabled(false);
        QCoreApplication::quit();
        return;
    }
}
#endif

void QuickMCPProcessor::processIncomingData(const QString &line)
{
    QString trimmedLine = line.trimmed();
    if (!trimmedLine.isEmpty()) {
        processCommand(trimmedLine);
    }
}

void QuickMCPProcessor::processCommand(const QString &line)
{
    // Parse JSON command
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        sendResponse(m_serverInfoHandler->createErrorResponse(QString("Invalid JSON: %1").arg(parseError.errorString()),
                                                              QJsonValue(), JsonRpcError::ParseError));
        return;
    }

    QJsonObject request = doc.object();

    // Extract request ID early for proper JSON-RPC 2.0 error responses
    QJsonValue requestId = request.value("id");

    // JSON-RPC 2.0 format validation
    QString jsonrpc = request.value("jsonrpc").toString();
    if (jsonrpc != "2.0") {
        sendResponse(m_serverInfoHandler->createErrorResponse("Invalid or missing jsonrpc version",
                                                              requestId, JsonRpcError::InvalidRequest));
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

            sendResponse(m_serverInfoHandler->createErrorResponse(QString("Schema validation failed: %1").arg(detailedError),
                                                                  requestId, JsonRpcError::InvalidParams));
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
            response = m_serverInfoHandler->createErrorResponse(QString("Unknown method: %1").arg(method),
                                                                requestId, JsonRpcError::MethodNotFound);
        }
    } catch (const std::exception &e) {
        response = m_serverInfoHandler->createErrorResponse(QString("Internal error: %1").arg(e.what()),
                                                            requestId, JsonRpcError::InternalError);
    } catch (...) {
        response = m_serverInfoHandler->createErrorResponse("Internal error: unknown exception",
                                                            requestId, JsonRpcError::InternalError);
    }

    // Validate and send response
    if (m_validator) {
        ValidationResult validationResult = m_validator->validateResponse(response, method);
        if (!validationResult.isValid) {
            QString detailedError = validationResult.errorMessage;
            if (!validationResult.errorPath.isEmpty()) {
                detailedError += QString(" (at path: %1)").arg(validationResult.errorPath);
            }
            QJsonObject errorResponse = m_serverInfoHandler->createErrorResponse(QString("Internal validation error: %1").arg(detailedError),
                                                                                  requestId, JsonRpcError::InternalError);
            sendResponse(errorResponse);
            return;
        }
    }

    sendResponse(response);
}

void QuickMCPProcessor::sendResponse(const QJsonObject &response)
{
    QJsonDocument doc(response);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    m_stdout << QString::fromUtf8(data) << Qt::endl;
    m_stdout.flush();
}
