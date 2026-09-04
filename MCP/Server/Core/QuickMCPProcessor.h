// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QStringList>
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
#include <atomic>

#include <QThread>

/**
 * \class StdinReader
 * \brief Background thread for blocking stdin reading (Windows only).
 *
 * Windows console/pipe stdin cannot be watched by QSocketNotifier, so on
 * Windows a dedicated thread blocks in std::getline and emits a line at a
 * time. On stop, QuickMCPProcessor closes the stdin handle to unblock the
 * parked read so run() returns cleanly (no QThread::terminate()). POSIX uses
 * a main-thread QSocketNotifier instead and does not build this class.
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
 * \class QuickMCPProcessor
 * \brief CanvasItem-side port of MCPProcessor.
 *
 * \details Copy-and-adapt port (same precedent as every other Quick-prefixed/Canvas-prefixed
 * class in this rewrite), not a retrofitted shared interface on MCPProcessor/BaseHandler/MainWindow --
 * see the plan's Phase 12a entry. Constructed with QuickAppController* instead of MainWindow*,
 * and builds Quick*Handler instances instead of the Widgets *Handler family for the 6 handlers
 * that need real Scene/MainWindow access (ConnectionHandler, ElementHandler, FileHandler,
 * HistoryHandler, ICHandler, SimulationHandler) -- ServerInfoHandler/ThemeHandler need neither
 * (app-global, not per-tab), so those two are reused completely as-is, constructed with a null
 * MainWindow* (see BaseHandler's constructor -- never dereferenced by either handler, confirmed
 * by reading both .cpp files in full, not assumed).
 *
 * The stdin-reading/line-buffering machinery (extractStdinLines()'s cap logic, the Windows-only
 * StdinReader background thread above) originated on the now-removed Widgets-only MCPProcessor
 * and moved here once it was that class's last surviving dependent -- kMaxStdinLineBytes/
 * extractStdinLines() are pure static/stateless (no MainWindow or QuickAppController state), so
 * moving them was a rename, not a behavior change.
 */
class QuickMCPProcessor : public QObject
{
    Q_OBJECT

public:
    explicit QuickMCPProcessor(QuickAppController *appController, QObject *parent = nullptr);
    ~QuickMCPProcessor();

    void startProcessing();
    void stopProcessing();

    /// Maximum bytes buffered while waiting for a newline-terminated JSON-RPC command — see
    /// extractStdinLines(). Public so tests can reference the real limit instead of a
    /// duplicated magic number.
    static constexpr qint64 kMaxStdinLineBytes = 16 * 1024 * 1024; // 16 MB

    /// Appends \a data to \a buffer and returns every complete ('\n'-terminated) line found,
    /// removing them from \a buffer. If more than kMaxStdinLineBytes accumulates with no
    /// newline, clears \a buffer instead of growing it without bound — stdin is not a trusted
    /// channel in --mcp/--mcp-gui mode (any local process that can pipe into wiRedPanda's
    /// stdin can write it). A pure static method, independent of any live file descriptor, so
    /// the buffering/cap logic is directly unit-testable.
    static QStringList extractStdinLines(QByteArray &buffer, const QByteArray &data);

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
