// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QTextStream>

class BaseHandler;
class MCPValidator;
class QSocketNotifier;
class QuickAppController;
class QuickConnectionHandler;
class QuickElementHandler;
class QuickFileHandler;
class QuickHistoryHandler;
class QuickICHandler;
class QuickSimulationHandler;
class ServerInfoHandler;
class ThemeHandler;
#ifdef Q_OS_WIN
class StdinReader;
#endif

/**
 * \class QuickMCPProcessor
 * \brief CanvasItem-side port of MCPProcessor.
 *
 * \details Copy-and-adapt port (same precedent as every other Quick-prefixed/Canvas-prefixed
 * class in this rewrite), not a retrofitted shared interface on MCPProcessor/BaseHandler/MainWindow --
 * see the plan's Phase 5c entry. Constructed with QuickAppController* instead of MainWindow*,
 * and builds Quick*Handler instances instead of the Widgets *Handler family for the 6 handlers
 * that need real Scene/MainWindow access (ConnectionHandler, ElementHandler, FileHandler,
 * HistoryHandler, ICHandler, SimulationHandler) -- ServerInfoHandler/ThemeHandler need neither
 * (app-global, not per-tab), so those two are reused completely as-is, constructed with a null
 * MainWindow* (see BaseHandler's constructor -- never dereferenced by either handler, confirmed
 * by reading both .cpp files in full, not assumed).
 *
 * The stdin-reading/line-buffering machinery itself (extractStdinLines()'s cap logic, the
 * Windows-only StdinReader background thread) is NOT duplicated here -- both are already public,
 * MainWindow-independent members of MCPProcessor (see that header's own doc comments on why),
 * so this class calls/constructs them directly. Only the handler set, dispatch map, and the
 * thin processCommand()/sendResponse() wrapper around them are genuinely new here.
 */
class QuickMCPProcessor : public QObject
{
    Q_OBJECT

public:
    explicit QuickMCPProcessor(QuickAppController *appController, QObject *parent = nullptr);
    ~QuickMCPProcessor();

    void startProcessing();
    void stopProcessing();

private slots:
    void processIncomingData(const QString &line);
#ifndef Q_OS_WIN
    /// QSocketNotifier::activated handler: drains stdin, dispatches whole lines. Mirrors
    /// MCPProcessor::onStdinReadable() exactly (same non-blocking-fd/EAGAIN/EOF handling) --
    /// not reused directly since it's a private, non-static MCPProcessor member.
    void onStdinReadable();
#endif

private:
    void processCommand(const QString &line);
    void sendResponse(const QJsonObject &response);

    QuickAppController *m_appController;
    std::unique_ptr<MCPValidator> m_validator;
    QTextStream m_stdout;
#ifdef Q_OS_WIN
    StdinReader *m_stdinReader = nullptr;
#else
    QSocketNotifier *m_stdinNotifier = nullptr;
    QByteArray m_stdinBuffer;
#endif

    // Specialized handlers for different command domains -- see this class's own doc comment
    // for why ServerInfoHandler/ThemeHandler are reused as-is while the rest are Quick*Handler.
    std::unique_ptr<ServerInfoHandler> m_serverInfoHandler;
    std::unique_ptr<QuickFileHandler> m_fileHandler;
    std::unique_ptr<QuickElementHandler> m_elementHandler;
    std::unique_ptr<QuickConnectionHandler> m_connectionHandler;
    std::unique_ptr<QuickSimulationHandler> m_simulationHandler;
    std::unique_ptr<QuickICHandler> m_icHandler;
    std::unique_ptr<QuickHistoryHandler> m_historyHandler;
    std::unique_ptr<ThemeHandler> m_themeHandler;

    /// Maps each MCP method name to the handler responsible for it.
    QHash<QString, BaseHandler *> m_dispatchMap;
};
