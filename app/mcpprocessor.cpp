// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mcpprocessor.h"
#include "mainwindow.h"
#include "scene.h"
#include "workspace.h"
#include "graphicelement.h"
#include "elementfactory.h"
#include "simulation.h"
#include "nodes/qneport.h"
#include "nodes/qneconnection.h"
#include "graphicelementinput.h"

#include <QApplication>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QPixmap>
#include <QPainter>
#include <QPdfWriter>
#include <QSvgGenerator>
#include <QPageSize>
#include <cmath>
#include <climits>

MCPProcessor::MCPProcessor(MainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_scene(nullptr)
    , m_stdin(stdin)
    , m_stdout(stdout)
    , m_inputTimer(new QTimer(this))
{
    // Set up input processing timer
    m_inputTimer->setSingleShot(false);
    m_inputTimer->setInterval(10); // Check for input every 10ms
    connect(m_inputTimer, &QTimer::timeout, this, &MCPProcessor::processInput);
    
    // Get current scene
    if (m_mainWindow && m_mainWindow->currentTab()) {
        m_scene = m_mainWindow->currentTab()->scene();
    }
}

MCPProcessor::~MCPProcessor() = default;

void MCPProcessor::startProcessing()
{
    qDebug() << "MCP Processor started - ready for commands";
    fprintf(stderr, "MCP PROCESSOR DEBUG: startProcessing() called\n");
    fflush(stderr);
    m_inputTimer->start();
}

void MCPProcessor::processInput()
{
    if (m_stdin.atEnd()) {
        return;
    }
    
    // Read a line of input
    QString line = m_stdin.readLine();
    if (line.isEmpty()) {
        return;
    }
    
    // Parse JSON command
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        QJsonObject errorResponse = createErrorResponse(QString("Invalid JSON: %1").arg(error.errorString()));
        writeResponse(errorResponse);
        return;
    }
    
    QJsonObject command = doc.object();
    QJsonObject response = processCommand(command);
    writeResponse(response);
}

QJsonObject MCPProcessor::processCommand(const QJsonObject& command)
{
    // Validate command structure first
    if (!command.contains("command")) {
        return createErrorResponse("Missing 'command' field");
    }
    if (!command["command"].isString()) {
        return createErrorResponse("'command' field must be a string");
    }
    if (!command.contains("parameters")) {
        return createErrorResponse("Missing 'parameters' field");
    }
    if (!command["parameters"].isObject()) {
        return createErrorResponse("'parameters' field must be an object");
    }
    
    QString commandType = command["command"].toString();
    QJsonObject params = command["parameters"].toObject();
    
    // Validate command type
    if (commandType.isEmpty() || commandType.length() > 50) {
        return createErrorResponse("Invalid command type");
    }
    
    // Ensure we have a valid scene for commands that need it
    QStringList sceneRequiredCommands = {
        "create_element", "delete_element", "connect_elements", "disconnect_elements",
        "set_input_value", "get_output_value", "list_elements", "get_element_info", "export_image"
    };
    
    if (sceneRequiredCommands.contains(commandType)) {
        if (!m_scene && m_mainWindow && m_mainWindow->currentTab()) {
            m_scene = m_mainWindow->currentTab()->scene();
        }
        
        if (!m_scene) {
            return createErrorResponse("No active circuit scene available");
        }
    }
    
    // Route command to appropriate handler
    if (commandType == "create_element") {
        return handleCreateElement(params);
    } else if (commandType == "delete_element") {
        return handleDeleteElement(params);
    } else if (commandType == "connect_elements") {
        return handleConnectElements(params);
    } else if (commandType == "disconnect_elements") {
        return handleDisconnectElements(params);
    } else if (commandType == "save_circuit") {
        return handleSaveCircuit(params);
    } else if (commandType == "load_circuit") {
        return handleLoadCircuit(params);
    } else if (commandType == "new_circuit") {
        return handleNewCircuit(params);
    } else if (commandType == "simulation_control") {
        return handleSimulationControl(params);
    } else if (commandType == "set_input_value") {
        return handleSetInputValue(params);
    } else if (commandType == "get_output_value") {
        return handleGetOutputValue(params);
    } else if (commandType == "list_elements") {
        return handleListElements(params);
    } else if (commandType == "get_element_info") {
        return handleGetElementInfo(params);
    } else if (commandType == "export_image") {
        return handleExportImage(params);
    } else {
        return createErrorResponse(QString("Unknown command: %1").arg(commandType));
    }
}

void MCPProcessor::writeResponse(const QJsonObject& response)
{
    QJsonDocument doc(response);
    m_stdout << doc.toJson(QJsonDocument::Compact) << Qt::endl;
    m_stdout.flush();
}

QJsonObject MCPProcessor::handleCreateElement(const QJsonObject& params)
{
    qDebug() << "handleCreateElement called with params:" << params;
    
    if (!validateParameters(params, {"type", "x", "y"})) {
        qDebug() << "Parameter validation failed";
        return createErrorResponse("Missing required parameters: type, x, y");
    }
    
    QString type = params["type"].toString();
    double x = params["x"].toDouble();
    double y = params["y"].toDouble();
    QString label = params["label"].toString();
    
    qDebug() << "Attempting to create element:" << type << "at position" << x << "," << y;
    
    GraphicElement* element = createElementByType(type, QPointF(x, y), label);
    if (!element) {
        qDebug() << "Element creation failed - returning error";
        return createErrorResponse(QString("Failed to create element of type: %1").arg(type));
    }
    
    qDebug() << "Adding element to scene";
    // Add element to scene
    m_scene->addItem(element);
    
    QJsonObject result;
    result["element_id"] = static_cast<int>(element->id());
    result["type"] = type;
    result["position"] = QJsonObject{{"x", x}, {"y", y}};
    if (!label.isEmpty()) {
        result["label"] = label;
    }
    
    qDebug() << "Returning success response with element_id:" << element->id();
    return createSuccessResponse(result);
}

QJsonObject MCPProcessor::handleDeleteElement(const QJsonObject& params)
{
    if (!validateParameters(params, {"element_id"})) {
        return createErrorResponse("Missing required parameter: element_id");
    }
    
    int elementId = params["element_id"].toInt();
    GraphicElement* element = findElementById(elementId);
    
    if (!element) {
        return createErrorResponse(QString("Element not found: %1").arg(elementId));
    }
    
    m_scene->removeItem(element);
    delete element;
    
    return createSuccessResponse();
}

QJsonObject MCPProcessor::handleConnectElements(const QJsonObject& params)
{
    if (!validateParameters(params, {"source_id", "source_port", "target_id", "target_port"})) {
        return createErrorResponse("Missing required parameters: source_id, source_port, target_id, target_port");
    }
    
    int sourceId = params["source_id"].toInt();
    int sourcePort = params["source_port"].toInt();
    int targetId = params["target_id"].toInt();
    int targetPort = params["target_port"].toInt();
    
    GraphicElement* source = findElementById(sourceId);
    GraphicElement* target = findElementById(targetId);
    
    if (!source) {
        return createErrorResponse(QString("Source element not found: %1").arg(sourceId));
    }
    if (!target) {
        return createErrorResponse(QString("Target element not found: %1").arg(targetId));
    }
    
    if (!connectElementPorts(source, sourcePort, target, targetPort)) {
        return createErrorResponse("Failed to connect elements");
    }
    
    return createSuccessResponse();
}

QJsonObject MCPProcessor::handleDisconnectElements(const QJsonObject& params)
{
    if (!validateParameters(params, {"source_id", "source_port", "target_id", "target_port"})) {
        return createErrorResponse("Missing required parameters: source_id, source_port, target_id, target_port");
    }
    
    int sourceId = params["source_id"].toInt();
    int sourcePort = params["source_port"].toInt();
    int targetId = params["target_id"].toInt(); 
    int targetPort = params["target_port"].toInt();
    
    GraphicElement* source = findElementById(sourceId);
    GraphicElement* target = findElementById(targetId);
    
    if (!source) {
        return createErrorResponse(QString("Source element not found: %1").arg(sourceId));
    }
    if (!target) {
        return createErrorResponse(QString("Target element not found: %1").arg(targetId));
    }
    
    // Find and remove the connection between these specific ports
    if (sourcePort >= source->outputs().size() || targetPort >= target->inputs().size()) {
        return createErrorResponse("Invalid port numbers");
    }
    
    auto* outputPort = source->outputs().at(sourcePort);
    auto* inputPort = target->inputs().at(targetPort);
    
    // Find connection between these ports
    for (auto* connection : outputPort->connections()) {
        if (connection->endPort() == inputPort) {
            m_scene->removeItem(connection);
            delete connection;
            return createSuccessResponse();
        }
    }
    
    return createErrorResponse("No connection found between specified ports");
}

QJsonObject MCPProcessor::handleSaveCircuit(const QJsonObject& params)
{
    if (!validateParameters(params, {"file_path"})) {
        return createErrorResponse("Missing required parameter: file_path");
    }
    
    QString rawPath = params["file_path"].toString();
    QString filePath = sanitizeFilePath(rawPath);
    
    if (filePath.isEmpty()) {
        return createErrorResponse("Invalid file path");
    }
    
    // Additional security check - ensure path has valid circuit extension
    if (!filePath.endsWith(".panda", Qt::CaseInsensitive) && !filePath.endsWith(".pandaproject", Qt::CaseInsensitive)) {
        return createErrorResponse("File path must have .panda or .pandaproject extension for circuit files");
    }
    
    // Use MainWindow's save functionality
    try {
        m_mainWindow->save(filePath);
        QJsonObject result;
        result["saved_path"] = filePath;
        return createSuccessResponse(result);
    } catch (const std::exception& e) {
        return createErrorResponse(QString("Failed to save circuit: %1").arg(e.what()));
    }
}

QJsonObject MCPProcessor::handleLoadCircuit(const QJsonObject& params)
{
    if (!validateParameters(params, {"file_path"})) {
        return createErrorResponse("Missing required parameter: file_path");
    }
    
    QString rawPath = params["file_path"].toString();
    QString filePath = sanitizeFilePath(rawPath);
    
    if (filePath.isEmpty()) {
        return createErrorResponse("Invalid file path");
    }
    
    // Additional security check - ensure path has valid circuit extension
    if (!filePath.endsWith(".panda", Qt::CaseInsensitive) && !filePath.endsWith(".pandaproject", Qt::CaseInsensitive)) {
        return createErrorResponse("File path must have .panda or .pandaproject extension for circuit files");
    }
    
    if (!QFile::exists(filePath)) {
        return createErrorResponse(QString("File not found: %1").arg(filePath));
    }
    
    try {
        m_mainWindow->loadPandaFile(filePath);
        // Update scene reference after loading
        if (m_mainWindow->currentTab()) {
            m_scene = m_mainWindow->currentTab()->scene();
        }
        QJsonObject result;
        result["loaded_path"] = filePath;
        return createSuccessResponse(result);
    } catch (const std::exception& e) {
        return createErrorResponse(QString("Failed to load circuit: %1").arg(e.what()));
    }
}

QJsonObject MCPProcessor::handleNewCircuit(const QJsonObject& /*params*/)
{
    try {
        m_mainWindow->createNewTab();
        // Update scene reference after creating new tab
        if (m_mainWindow->currentTab()) {
            m_scene = m_mainWindow->currentTab()->scene();
        }
        return createSuccessResponse();
    } catch (const std::exception& e) {
        return createErrorResponse(QString("Failed to create new circuit: %1").arg(e.what()));
    }
}

QJsonObject MCPProcessor::handleSimulationControl(const QJsonObject& params)
{
    if (!validateParameters(params, {"action"})) {
        return createErrorResponse("Missing required parameter: action");
    }
    
    QString action = params["action"].toString();
    
    try {
        if (m_mainWindow->currentTab() && m_mainWindow->currentTab()->simulation()) {
            Simulation* sim = m_mainWindow->currentTab()->simulation();
            
            if (action == "start") {
                sim->start();
            } else if (action == "stop") {
                sim->stop();
            } else if (action == "reset") {
                sim->restart();
            } else if (action == "update") {
                // Force a single simulation update (like QtTest does)
                // CRITICAL: Don't restart simulation - this preserves circuit state/memory
                if (!sim->isRunning()) {
                    // If simulation is stopped, just do a single update
                    sim->update();
                } else {
                    // If simulation is running, temporarily stop it, update once, then restart
                    sim->stop();
                    sim->update();
                    sim->start();
                }
            } else {
                return createErrorResponse(QString("Unknown simulation action: %1").arg(action));
            }
            
            return createSuccessResponse();
        } else {
            return createErrorResponse("No simulation available");
        }
    } catch (const std::exception& e) {
        return createErrorResponse(QString("Simulation control failed: %1").arg(e.what()));
    }
}

QJsonObject MCPProcessor::handleSetInputValue(const QJsonObject& params)
{
    if (!validateParameters(params, {"element_id", "value"})) {
        return createErrorResponse("Missing required parameters: element_id, value");
    }
    
    int elementId = params["element_id"].toInt();
    bool value = params["value"].toBool();
    
    GraphicElement* element = findElementById(elementId);
    if (!element) {
        return createErrorResponse(QString("Element not found: %1").arg(elementId));
    }
    
    // Check if element is an input element
    auto* inputElement = dynamic_cast<GraphicElementInput*>(element);
    if (!inputElement) {
        return createErrorResponse("Element is not an input element");
    }
    
    try {
        inputElement->setOn(value);
        return createSuccessResponse();
    } catch (const std::exception& e) {
        return createErrorResponse(QString("Failed to set input value: %1").arg(e.what()));
    }
}

QJsonObject MCPProcessor::handleGetOutputValue(const QJsonObject& params)
{
    if (!validateParameters(params, {"element_id"})) {
        return createErrorResponse("Missing required parameter: element_id");
    }
    
    int elementId = params["element_id"].toInt();
    GraphicElement* element = findElementById(elementId);
    
    if (!element) {
        return createErrorResponse(QString("Element not found: %1").arg(elementId));
    }
    
    try {
        bool value = false;
        
        // If element has output ports, check first output port
        if (!element->outputs().isEmpty()) {
            auto* outputPort = element->outputs().first();
            value = (outputPort->status() == Status::Active);
        }
        // If element has no output ports but has input ports (like LED), check input port status
        else if (!element->inputs().isEmpty()) {
            auto* inputPort = element->inputs().first();
            value = (inputPort->status() == Status::Active);
        }
        else {
            return createErrorResponse("Element has no ports to check");
        }
        
        QJsonObject result;
        result["value"] = value;
        return createSuccessResponse(result);
    } catch (const std::exception& e) {
        return createErrorResponse(QString("Failed to get output value: %1").arg(e.what()));
    }
}

QJsonObject MCPProcessor::handleListElements(const QJsonObject& /*params*/)
{
    QJsonArray elements;
    
    if (m_scene) {
        auto items = m_scene->items();
        for (auto* item : items) {
            auto* element = dynamic_cast<GraphicElement*>(item);
            if (element) {
                elements.append(elementToJson(element));
            }
        }
    }
    
    QJsonObject result;
    result["elements"] = elements;
    return createSuccessResponse(result);
}

QJsonObject MCPProcessor::handleGetElementInfo(const QJsonObject& params)
{
    if (!validateParameters(params, {"element_id"})) {
        return createErrorResponse("Missing required parameter: element_id");
    }
    
    int elementId = params["element_id"].toInt();
    GraphicElement* element = findElementById(elementId);
    
    if (!element) {
        return createErrorResponse(QString("Element not found: %1").arg(elementId));
    }
    
    return createSuccessResponse(elementToJson(element));
}

// Helper methods implementation

GraphicElement* MCPProcessor::findElementById(int elementId)
{
    if (!m_scene) return nullptr;
    
    // Basic bounds checking for element ID
    if (elementId < 0 || elementId > 100000) {
        qDebug() << "Element ID out of reasonable bounds:" << elementId;
        return nullptr;
    }
    
    auto items = m_scene->items();
    for (auto* item : items) {
        auto* element = dynamic_cast<GraphicElement*>(item);
        if (element && static_cast<int>(element->id()) == elementId) {
            return element;
        }
    }
    return nullptr;
}

QJsonObject MCPProcessor::elementToJson(GraphicElement* element)
{
    QJsonObject obj;
    obj["id"] = static_cast<int>(element->id());
    obj["type"] = element->objectName();
    obj["position"] = QJsonObject{
        {"x", element->pos().x()},
        {"y", element->pos().y()}
    };
    obj["label"] = element->label();
    return obj;
}

QJsonObject MCPProcessor::createErrorResponse(const QString& message) const
{
    QJsonObject response;
    response["status"] = "error";
    response["error"] = message;
    return response;
}

QJsonObject MCPProcessor::createSuccessResponse(const QJsonObject& result) const
{
    QJsonObject response;
    response["status"] = "success";
    if (!result.isEmpty()) {
        response["result"] = result;
    }
    return response;
}

bool MCPProcessor::validateParameters(const QJsonObject& params, const QStringList& required)
{
    for (const QString& param : required) {
        if (!params.contains(param)) {
            qDebug() << "Missing required parameter:" << param;
            return false;
        }
        
        QJsonValue value = params[param];
        
        // Type-specific validation
        if (param.endsWith("_id") || param == "element_id" || param == "source_id" || param == "target_id") {
            if (!value.isDouble()) {
                qDebug() << "Parameter" << param << "must be numeric, got:" << value.type();
                return false;
            }
            double numValue = value.toDouble();
            if (std::isnan(numValue) || std::isinf(numValue) || numValue < 0) {
                qDebug() << "Parameter" << param << "must be a positive number, got:" << numValue;
                return false;
            }
        }
        else if (param == "x" || param == "y") {
            if (!value.isDouble()) {
                qDebug() << "Coordinate parameter" << param << "must be numeric, got:" << value.type();
                return false;
            }
            double coord = value.toDouble();
            if (std::isnan(coord) || std::isinf(coord)) {
                qDebug() << "Coordinate parameter" << param << "cannot be NaN or infinite, got:" << coord;
                return false;
            }
            // Reasonable coordinate bounds to prevent extreme values
            if (coord < -10000.0 || coord > 10000.0) {
                qDebug() << "Coordinate parameter" << param << "out of reasonable bounds:" << coord;
                return false;
            }
        }
        else if (param.endsWith("_port") || param == "source_port" || param == "target_port") {
            if (!value.isDouble()) {
                qDebug() << "Port parameter" << param << "must be numeric, got:" << value.type();
                return false;
            }
            double portValue = value.toDouble();
            if (std::isnan(portValue) || std::isinf(portValue) || portValue < 0 || portValue > 100) {
                qDebug() << "Port parameter" << param << "must be 0-100, got:" << portValue;
                return false;
            }
        }
        else if (param == "value") {
            if (!value.isBool()) {
                qDebug() << "Boolean parameter" << param << "must be true/false, got:" << value.type();
                return false;
            }
        }
        else if (param == "type" || param == "action" || param == "format") {
            if (!value.isString()) {
                qDebug() << "String parameter" << param << "must be a string, got:" << value.type();
                return false;
            }
            QString strValue = value.toString();
            if (strValue.isEmpty() || strValue.length() > 100) {
                qDebug() << "String parameter" << param << "invalid length:" << strValue.length();
                return false;
            }
        }
        else if (param == "file_path") {
            if (!value.isString()) {
                qDebug() << "File path parameter must be a string, got:" << value.type();
                return false;
            }
            QString path = value.toString();
            if (path.isEmpty() || path.length() > 1000) {
                qDebug() << "File path invalid length:" << path.length();
                return false;
            }
        }
    }
    return true;
}

QString MCPProcessor::sanitizeFilePath(const QString& path) const
{
    if (path.isEmpty()) {
        return QString();
    }
    
    // Remove potentially dangerous characters and sequences
    QString sanitized = path;
    
    // Remove directory traversal sequences
    sanitized.remove("../");
    sanitized.remove("..\\");
    sanitized.replace("\\", "/"); // Normalize path separators
    
    // Remove null bytes and control characters
    for (int i = 0; i < sanitized.length(); i++) {
        QChar c = sanitized.at(i);
        if (c.unicode() < 32 || c.unicode() == 127) {
            sanitized.remove(i, 1);
            i--;
        }
    }
    
    // Ensure path doesn't start with / (absolute path) unless explicitly allowed
    if (sanitized.startsWith("/")) {
        qDebug() << "Warning: Absolute path provided, allowing for now:" << sanitized;
        // In production, you might want to restrict this more
    }
    
    // Basic length check
    if (sanitized.length() > 1000) {
        qDebug() << "Path too long, truncating";
        sanitized = sanitized.left(1000);
    }
    
    // Check for valid file extensions for circuit files
    QStringList allowedExtensions = {".panda", ".pandaproject", ".png", ".pdf", ".svg"};
    bool hasValidExtension = false;
    for (const QString& ext : allowedExtensions) {
        if (sanitized.endsWith(ext, Qt::CaseInsensitive)) {
            hasValidExtension = true;
            break;
        }
    }
    
    if (!hasValidExtension) {
        qDebug() << "Warning: File extension not in allowed list:" << sanitized;
        // In production, you might want to reject this
    }
    
    return sanitized;
}

bool MCPProcessor::safeIntFromJson(const QJsonObject& params, const QString& key, int& result) const
{
    if (!params.contains(key)) {
        return false;
    }
    
    QJsonValue value = params[key];
    if (!value.isDouble()) {
        return false;
    }
    
    double doubleValue = value.toDouble();
    
    // Check for NaN, infinity
    if (std::isnan(doubleValue) || std::isinf(doubleValue)) {
        return false;
    }
    
    // Check if it's within reasonable integer bounds
    if (doubleValue < INT_MIN || doubleValue > INT_MAX) {
        return false;
    }
    
    // Check if it's actually an integer (no fractional part)
    if (doubleValue != std::floor(doubleValue)) {
        return false;
    }
    
    result = static_cast<int>(doubleValue);
    return true;
}

GraphicElement* MCPProcessor::createElementByType(const QString& type, const QPointF& position, const QString& label)
{
    qDebug() << "Creating element of type:" << type;
    ElementType elementType = ElementFactory::textToType(type);
    qDebug() << "ElementType resolved to:" << static_cast<int>(elementType);
    
    // Check for both Unknown (0) and invalid enum values (negative)
    if (elementType == ElementType::Unknown || static_cast<int>(elementType) < 0) {
        qDebug() << "Element type is Unknown or invalid - returning nullptr";
        return nullptr;
    }
    
    GraphicElement* element = ElementFactory::buildElement(elementType);
    if (!element) {
        qDebug() << "ElementFactory::buildElement failed - returning nullptr";
        return nullptr;
    }
    
    element->setPos(position);
    if (!label.isEmpty()) {
        element->setLabel(label);
    }
    
    qDebug() << "Element created successfully with ID:" << element->id();
    return element;
}

bool MCPProcessor::connectElementPorts(GraphicElement* source, int sourcePort, GraphicElement* target, int targetPort)
{
    if (!source || !target) {
        return false;
    }
    
    // Get output port from source
    if (sourcePort >= source->outputs().size()) {
        return false;
    }
    auto* outputPort = dynamic_cast<QNEOutputPort*>(source->outputs().at(sourcePort));
    
    // Get input port from target
    if (targetPort >= target->inputs().size()) {
        return false;
    }
    auto* inputPort = dynamic_cast<QNEInputPort*>(target->inputs().at(targetPort));
    
    if (!outputPort || !inputPort) {
        return false;
    }
    
    // Create connection
    try {
        auto* connection = new QNEConnection();
        connection->setStartPort(outputPort);
        connection->setEndPort(inputPort);
        
        // Add connection to scene
        if (m_scene) {
            m_scene->addItem(connection);
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

QJsonObject MCPProcessor::handleExportImage(const QJsonObject& params)
{
    if (!validateParameters(params, {"file_path", "format"})) {
        return createErrorResponse("Missing required parameters: file_path, format");
    }
    
    QString rawPath = params["file_path"].toString();
    QString filePath = sanitizeFilePath(rawPath);
    QString format = params["format"].toString().toLower();
    
    if (filePath.isEmpty()) {
        return createErrorResponse("Invalid file path");
    }
    
    // Validate format
    if (format != "png" && format != "pdf" && format != "svg") {
        return createErrorResponse("Unsupported format. Supported formats: png, pdf, svg");
    }
    
    // Ensure file path has correct extension for format
    QString expectedExt = "." + format;
    if (!filePath.endsWith(expectedExt, Qt::CaseInsensitive)) {
        return createErrorResponse(QString("File path must have %1 extension for %2 format").arg(expectedExt, format));
    }
    
    if (!m_scene) {
        return createErrorResponse("No active scene to export");
    }
    
    try {
        // Get scene rectangle
        QRectF sceneRect = m_scene->itemsBoundingRect();
        if (sceneRect.isEmpty()) {
            sceneRect = QRectF(0, 0, 800, 600); // Default size for empty scene
        }
        
        // Add some padding around the content
        sceneRect = sceneRect.adjusted(-50, -50, 50, 50);
        
        if (format == "png") {
            // Create pixmap and render scene
            QPixmap pixmap(sceneRect.size().toSize());
            pixmap.fill(Qt::white);
            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            m_scene->render(&painter, QRectF(), sceneRect);
            painter.end();
            
            // Save as PNG
            if (!pixmap.save(filePath, "PNG")) {
                return createErrorResponse(QString("Failed to save PNG file: %1").arg(filePath));
            }
        } else if (format == "pdf") {
            // Create PDF printer
            QPdfWriter pdfWriter(filePath);
            pdfWriter.setPageSize(QPageSize(sceneRect.size(), QPageSize::Point));
            pdfWriter.setPageMargins(QMarginsF(0, 0, 0, 0));
            
            QPainter painter(&pdfWriter);
            painter.setRenderHint(QPainter::Antialiasing);
            m_scene->render(&painter, QRectF(), sceneRect);
            painter.end();
        } else if (format == "svg") {
            // Create SVG generator
            QSvgGenerator svgGen;
            svgGen.setFileName(filePath);
            svgGen.setSize(sceneRect.size().toSize());
            svgGen.setViewBox(sceneRect);
            svgGen.setTitle("wiRedPanda Circuit");
            svgGen.setDescription("Digital logic circuit created with wiRedPanda");
            
            QPainter painter(&svgGen);
            painter.setRenderHint(QPainter::Antialiasing);
            m_scene->render(&painter, QRectF(), sceneRect);
            painter.end();
        }
        
        QJsonObject result;
        result["file_path"] = filePath;
        result["format"] = format;
        result["exported"] = true;
        
        return createSuccessResponse(result);
        
    } catch (const std::exception& e) {
        return createErrorResponse(QString("Export failed: %1").arg(e.what()));
    } catch (...) {
        return createErrorResponse("Unknown error occurred during export");
    }
}