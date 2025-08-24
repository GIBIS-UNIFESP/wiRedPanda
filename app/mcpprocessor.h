// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTextStream>
#include <QTimer>

class MainWindow;
class GraphicElement;
class Scene;

/**
 * @brief MCP (Model Context Protocol) processor for wiRedPanda
 * 
 * Processes JSON commands from stdin and writes responses to stdout.
 * Enables programmatic control of wiRedPanda for circuit creation,
 * manipulation, and simulation.
 */
class MCPProcessor : public QObject
{
    Q_OBJECT

public:
    explicit MCPProcessor(MainWindow* mainWindow, QObject* parent = nullptr);
    ~MCPProcessor() override;

    /**
     * @brief Start processing commands from stdin
     */
    void startProcessing();

private slots:
    /**
     * @brief Read and process commands from stdin
     */
    void processInput();

private:
    MainWindow* m_mainWindow;
    Scene* m_scene;
    QTextStream m_stdin;
    QTextStream m_stdout;
    QTimer* m_inputTimer;
    
    // Command handlers
    QJsonObject handleCreateElement(const QJsonObject& params);
    QJsonObject handleDeleteElement(const QJsonObject& params);
    QJsonObject handleConnectElements(const QJsonObject& params);
    QJsonObject handleDisconnectElements(const QJsonObject& params);
    QJsonObject handleSaveCircuit(const QJsonObject& params);
    QJsonObject handleLoadCircuit(const QJsonObject& params);
    QJsonObject handleNewCircuit(const QJsonObject& params);
    QJsonObject handleSimulationControl(const QJsonObject& params);
    QJsonObject handleSetInputValue(const QJsonObject& params);
    QJsonObject handleGetOutputValue(const QJsonObject& params);
    QJsonObject handleListElements(const QJsonObject& params);
    QJsonObject handleGetElementInfo(const QJsonObject& params);
    QJsonObject handleExportImage(const QJsonObject& params);
    
    // Helper methods
    GraphicElement* findElementById(int elementId);
    QJsonObject elementToJson(GraphicElement* element);
    QJsonObject createErrorResponse(const QString& message) const;
    QJsonObject createSuccessResponse(const QJsonObject& result = QJsonObject()) const;
    bool validateParameters(const QJsonObject& params, const QStringList& required);
    
    // Element creation helpers
    GraphicElement* createElementByType(const QString& type, const QPointF& position, const QString& label);
    
    // Connection helpers
    bool connectElementPorts(GraphicElement* source, int sourcePort, GraphicElement* target, int targetPort);
    
    // Command processing
    QJsonObject processCommand(const QJsonObject& command);
    void writeResponse(const QJsonObject& response);
};