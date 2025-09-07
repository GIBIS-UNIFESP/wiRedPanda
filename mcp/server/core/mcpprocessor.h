// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTextStream>
#include <QTimer>
#include <QThread>
#include <memory>

class ConnectionHandler;
class ElementHandler;
class FileHandler;
class MCPValidator;
class MainWindow;
class ServerInfoHandler;
class SimulationHandler;

/*!
 * @class StdinReader
 * @brief Background thread for non-blocking stdin reading
 *
 * Reads stdin in a dedicated thread and emits signals when data is available.
 * This replaces the 1ms polling timer with true event-driven I/O.
 */
class StdinReader : public QThread
{
    Q_OBJECT

public:
    explicit StdinReader(QObject *parent = nullptr);
    void requestStop();

protected:
    void run() override;

signals:
    void dataReceived(const QString &line);

private:
    volatile bool m_stopRequested = false;
};

/*!
 * @class MCPProcessor
 * @brief Model Context Protocol processor for wiRedPanda
 *
 * Provides simple, elegant programmatic control of circuit design and simulation
 * through stdin/stdout JSON command processing. Uses QTimer-based polling for
 * reliable cross-platform stdin processing.
 */
class MCPProcessor : public QObject
{
    Q_OBJECT

public:
    explicit MCPProcessor(MainWindow *mainWindow, QObject *parent = nullptr);
    ~MCPProcessor();

    void startProcessing();
    void stopProcessing();

private slots:
    void processIncomingData(const QString &line);

private:
    void processCommand(const QString &line);
    void sendResponse(const QJsonObject &response);

    MainWindow *m_mainWindow;
    std::unique_ptr<MCPValidator> m_validator;
    QTextStream m_stdin;
    QTextStream m_stdout;
    StdinReader *m_stdinReader;

    // Specialized handlers for different command domains
    std::unique_ptr<ServerInfoHandler> m_serverInfoHandler;
    std::unique_ptr<FileHandler> m_fileHandler;
    std::unique_ptr<ElementHandler> m_elementHandler;
    std::unique_ptr<ConnectionHandler> m_connectionHandler;
    std::unique_ptr<SimulationHandler> m_simulationHandler;
};
