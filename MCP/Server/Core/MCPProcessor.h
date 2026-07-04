// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <atomic>
#include <memory>

#include <QBuffer>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTextStream>
#include <QThread>
#include <QTimer>

class BaseHandler;
class ConnectionHandler;
class ElementHandler;
class FileHandler;
class HistoryHandler;
class ICHandler;
class MCPValidator;
class MainWindow;
class QSocketNotifier;
class ServerInfoHandler;
class SimulationHandler;
class ThemeHandler;

#ifdef Q_OS_WIN
/**
 * \class StdinReader
 * \brief Background thread for blocking stdin reading (Windows only).
 *
 * Windows console/pipe stdin cannot be watched by QSocketNotifier, so on
 * Windows a dedicated thread blocks in std::getline and emits a line at a
 * time. On stop, MCPProcessor closes the stdin handle to unblock the parked
 * read so run() returns cleanly (no QThread::terminate()). POSIX uses a
 * main-thread QSocketNotifier instead and does not build this class.
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
    std::atomic<bool> m_stopRequested{false};
};
#endif

/**
 * \class MCPProcessor
 * \brief Model Context Protocol processor for wiRedPanda
 *
 * Provides simple, elegant programmatic control of circuit design and simulation
 * through stdin/stdout JSON command processing. stdin is read event-driven: a
 * main-thread QSocketNotifier on POSIX, a dedicated StdinReader thread on
 * Windows. A closed stdin (EOF) shuts the processor down cleanly.
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
#ifndef Q_OS_WIN
    /// QSocketNotifier::activated handler: drains stdin, dispatches whole lines.
    void onStdinReadable();
#endif

private:
    void processCommand(const QString &line);
    void sendResponse(const QJsonObject &response);

    MainWindow *m_mainWindow;
    std::unique_ptr<MCPValidator> m_validator;
    QTextStream m_stdout;
#ifdef Q_OS_WIN
    StdinReader *m_stdinReader = nullptr;
#else
    QSocketNotifier *m_stdinNotifier = nullptr;
    QByteArray m_stdinBuffer;
#endif

    // Specialized handlers for different command domains
    std::unique_ptr<ServerInfoHandler> m_serverInfoHandler;
    std::unique_ptr<FileHandler> m_fileHandler;
    std::unique_ptr<ElementHandler> m_elementHandler;
    std::unique_ptr<ConnectionHandler> m_connectionHandler;
    std::unique_ptr<SimulationHandler> m_simulationHandler;
    std::unique_ptr<ICHandler> m_icHandler;
    std::unique_ptr<HistoryHandler> m_historyHandler;
    std::unique_ptr<ThemeHandler> m_themeHandler;

    /// Maps each MCP method name to the handler responsible for it.
    QHash<QString, BaseHandler *> m_dispatchMap;
};
