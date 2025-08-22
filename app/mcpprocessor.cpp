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
    QString commandType = command["command"].toString();
    QJsonObject params = command["parameters"].toObject();
    
    // Ensure we have a valid scene
    if (!m_scene && m_mainWindow && m_mainWindow->currentTab()) {
        m_scene = m_mainWindow->currentTab()->scene();
    }
    
    if (!m_scene) {
        return createErrorResponse("No active circuit scene available");
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
    if (!validateParameters(params, {"type", "x", "y"})) {
        return createErrorResponse("Missing required parameters: type, x, y");
    }
    
    QString type = params["type"].toString();
    double x = params["x"].toDouble();
    double y = params["y"].toDouble();
    QString label = params["label"].toString();
    
    GraphicElement* element = createElementByType(type, QPointF(x, y), label);
    if (!element) {
        return createErrorResponse(QString("Failed to create element of type: %1").arg(type));
    }
    
    // Add element to scene
    m_scene->addItem(element);
    
    QJsonObject result;
    result["element_id"] = static_cast<int>(element->id());
    result["type"] = type;
    result["position"] = QJsonObject{{"x", x}, {"y", y}};
    if (!label.isEmpty()) {
        result["label"] = label;
    }
    
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
    
    QString filePath = params["file_path"].toString();
    
    // Use MainWindow's save functionality
    try {
        m_mainWindow->save(filePath);
        return createSuccessResponse();
    } catch (const std::exception& e) {
        return createErrorResponse(QString("Failed to save circuit: %1").arg(e.what()));
    }
}

QJsonObject MCPProcessor::handleLoadCircuit(const QJsonObject& params)
{
    if (!validateParameters(params, {"file_path"})) {
        return createErrorResponse("Missing required parameter: file_path");
    }
    
    QString filePath = params["file_path"].toString();
    
    if (!QFile::exists(filePath)) {
        return createErrorResponse(QString("File not found: %1").arg(filePath));
    }
    
    try {
        m_mainWindow->loadPandaFile(filePath);
        // Update scene reference after loading
        if (m_mainWindow->currentTab()) {
            m_scene = m_mainWindow->currentTab()->scene();
        }
        return createSuccessResponse();
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
            return false;
        }
    }
    return true;
}

GraphicElement* MCPProcessor::createElementByType(const QString& type, const QPointF& position, const QString& label)
{
    ElementType elementType = ElementFactory::textToType(type);
    if (elementType == ElementType::Unknown) {
        return nullptr;
    }
    
    GraphicElement* element = ElementFactory::buildElement(elementType);
    if (!element) {
        return nullptr;
    }
    
    element->setPos(position);
    if (!label.isEmpty()) {
        element->setLabel(label);
    }
    
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
    
    QString filePath = params["file_path"].toString();
    QString format = params["format"].toString().toLower();
    
    // Validate format
    if (format != "png" && format != "pdf" && format != "svg") {
        return createErrorResponse("Unsupported format. Supported formats: png, pdf, svg");
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