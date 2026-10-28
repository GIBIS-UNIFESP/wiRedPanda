// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/MCP/TestMCPProcessor.h"

#include <functional>

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QSocketNotifier>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QUndoStack>

#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MainWindow.h"
#include "MCP/Server/Core/JsonRpcError.h"
#include "MCP/Server/Core/MCPProcessor.h"
#include "Tests/Common/TestUtils.h"

namespace {

void sendLine(MCPProcessor &processor, const QString &line)
{
    QMetaObject::invokeMethod(&processor, "processIncomingData", Qt::DirectConnection, Q_ARG(QString, line));
}

// Redirects real fd 0 to the read end of a pipe we control, so startProcessing()'s real
// QSocketNotifier/onStdinReadable() path can be driven deterministically without touching
// the actual, shared process stdin other parallel test *processes* might depend on (each
// test class is its own process, but this still avoids any risk to this process's own stdin
// state after the guard is gone). The write end stays open (not immediately EOF) unless
// closeWriteEnd() is called explicitly, mirroring a real client that hasn't disconnected yet.
struct ScopedStdinPipe {
    int savedFd;
    int writeFd;

    ScopedStdinPipe()
        : savedFd(testUtilsDupFd(0))
    {
        int fds[2];
        if (testUtilsPipeFd(fds) != 0) {
            qFatal("ScopedStdinPipe: pipe() failed");
        }
        testUtilsDup2Fd(fds[0], 0);
        testUtilsCloseFd(fds[0]);
        writeFd = fds[1];
    }

    void writeData(const QByteArray &data)
    {
        if (testUtilsWriteFd(writeFd, data.constData(), static_cast<size_t>(data.size())) != data.size()) {
            qFatal("ScopedStdinPipe::writeData: write() failed or was incomplete");
        }
    }

    void closeWriteEnd()
    {
        if (writeFd != -1) {
            testUtilsCloseFd(writeFd);
            writeFd = -1;
        }
    }

    ~ScopedStdinPipe()
    {
        closeWriteEnd();
        testUtilsDup2Fd(savedFd, 0);
        testUtilsCloseFd(savedFd);
    }
};

// QCoreApplication::quit() does nothing outside the application's own exec(), so a test of what quitting does runs it:
// \a step runs inside it, and exec() returns 0 when quitting ends it, 1 once \a done holds, or 2 from the guard against
// a quit that never ends it
int execUntil(const std::function<void()> &step, const std::function<bool()> &done)
{
    QTimer::singleShot(0, step);
    QTimer poll;
    QObject::connect(&poll, &QTimer::timeout, [&done] {
        if (done()) {
            QCoreApplication::exit(1);
        }
    });
    poll.start(10);
    QTimer guard;
    guard.setSingleShot(true);
    QObject::connect(&guard, &QTimer::timeout, [] { QCoreApplication::exit(2); });
    guard.start(30000);
    return QCoreApplication::exec();
}

} // namespace

void TestMCPProcessor::testExtractStdinLinesSplitsCompleteLines()
{
    QByteArray buffer;
    const QStringList lines = MCPProcessor::extractStdinLines(buffer, "abc\ndef\n");

    QCOMPARE(lines, QStringList({"abc", "def"}));
    QVERIFY(buffer.isEmpty());
}

void TestMCPProcessor::testExtractStdinLinesHandlesPartialLineAcrossCalls()
{
    QByteArray buffer;

    const QStringList firstResult = MCPProcessor::extractStdinLines(buffer, "ab");
    QVERIFY(firstResult.isEmpty());
    QCOMPARE(buffer, QByteArray("ab"));

    const QStringList secondResult = MCPProcessor::extractStdinLines(buffer, "c\n");
    QCOMPARE(secondResult, QStringList({"abc"}));
    QVERIFY(buffer.isEmpty());
}

void TestMCPProcessor::testExtractStdinLinesKeepsIncompleteLineAtExactCap()
{
    // No newline anywhere — the whole thing stays buffered, waiting for more data.
    // Exactly at the cap must NOT be treated as an overflow.
    QByteArray buffer;
    const QByteArray chunk(MCPProcessor::kMaxStdinLineBytes, 'x');

    const QStringList lines = MCPProcessor::extractStdinLines(buffer, chunk);

    QVERIFY(lines.isEmpty());
    QCOMPARE(buffer.size(), MCPProcessor::kMaxStdinLineBytes);
}

void TestMCPProcessor::testExtractStdinLinesDropsIncompleteLineOverCap()
{
    // One byte past the cap, still no newline: a client that never sends '\n' must not be
    // allowed to grow the buffer without bound.
    QByteArray buffer;
    const QByteArray chunk(MCPProcessor::kMaxStdinLineBytes + 1, 'x');

    const QStringList lines = MCPProcessor::extractStdinLines(buffer, chunk);

    QVERIFY(lines.isEmpty());
    QVERIFY2(buffer.isEmpty(), "Buffer must be dropped once it exceeds the cap with no newline");
}

void TestMCPProcessor::testProcessIncomingDataIgnoresBlankLine()
{
    MainWindow window;
    MCPProcessor processor(&window);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    TestUtils::ScopedStdoutCapture capture(dir.filePath("stdout.txt"));

    sendLine(processor, QStringLiteral("   "));

    QVERIFY2(capture.json().isEmpty(), "a blank/whitespace-only line must never reach processCommand()");
}

void TestMCPProcessor::testProcessCommandRejectsInvalidJson()
{
    MainWindow window;
    MCPProcessor processor(&window);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    TestUtils::ScopedStdoutCapture capture(dir.filePath("stdout.txt"));

    sendLine(processor, QStringLiteral("this is not json"));

    const QJsonObject response = capture.json();
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ParseError);
}

void TestMCPProcessor::testProcessCommandRejectsWrongJsonRpcVersion()
{
    MainWindow window;
    MCPProcessor processor(&window);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    TestUtils::ScopedStdoutCapture capture(dir.filePath("stdout.txt"));

    sendLine(processor, QStringLiteral(R"({"jsonrpc":"1.0","method":"get_server_info","id":1})"));

    const QJsonObject response = capture.json();
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidRequest);
}

void TestMCPProcessor::testProcessCommandRejectsSchemaViolation()
{
    MainWindow window;
    MCPProcessor processor(&window);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    TestUtils::ScopedStdoutCapture capture(dir.filePath("stdout.txt"));

    // create_element requires params.type/x/y; omit x/y entirely.
    sendLine(processor, QStringLiteral(R"({"jsonrpc":"2.0","method":"create_element","params":{"type":"And"},"id":1})"));

    const QJsonObject response = capture.json();
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject error = response["error"].toObject();
    QCOMPARE(error["code"].toInt(), JsonRpcError::InvalidParams);
    QVERIFY2(error["message"].toString().contains("Schema validation failed"), qPrintable(error["message"].toString()));
}

void TestMCPProcessor::testProcessCommandRejectsUnknownMethod()
{
    MainWindow window;
    MCPProcessor processor(&window);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    TestUtils::ScopedStdoutCapture capture(dir.filePath("stdout.txt"));

    sendLine(processor, QStringLiteral(R"({"jsonrpc":"2.0","method":"totally_made_up_method","params":{},"id":1})"));

    const QJsonObject response = capture.json();
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject error = response["error"].toObject();
    // "totally_made_up_method" fails schema validation before the dispatch map is even
    // consulted (no command schema exists for it either) -- MethodNotFound is reserved for a
    // method that passes schema validation but has no registered handler, which no current
    // method name can trigger (every schema-known command is also a routed one).
    QCOMPARE(error["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestMCPProcessor::testProcessCommandDispatchesKnownMethodSuccessfully()
{
    MainWindow window;
    MCPProcessor processor(&window);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    TestUtils::ScopedStdoutCapture capture(dir.filePath("stdout.txt"));

    sendLine(processor, QStringLiteral(R"({"jsonrpc":"2.0","method":"get_server_info","params":{},"id":1})"));

    const QJsonObject response = capture.json();
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(!response.contains("error"));
    QCOMPARE(response["id"].toInt(), 1);
}

void TestMCPProcessor::testEachRequestOfABatchRunsInItsOwnPassOfTheEventLoop()
{
    // Requests a client wrote at once run one per pass of the event loop, so the work queued while the first runs, as
    // Qt queues an added element's polish, runs before the second: here a queued call posted as the first request's
    // command is pushed
    MainWindow window;
    MCPProcessor processor(&window);
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    TestUtils::ScopedStdoutCapture capture(dir.filePath("stdout.txt"));

    int responsesWhenQueuedWorkRan = -1;
    // Destroyed first, so neither the window's teardown nor a call it queued reaches this test's locals
    QObject context;
    connect(window.currentTab()->scene()->undoStack(), &QUndoStack::indexChanged, &context, [&] {
        QMetaObject::invokeMethod(&context, [&] {
            if (responsesWhenQueuedWorkRan < 0) {
                responsesWhenQueuedWorkRan = static_cast<int>(capture.contents().count('\n'));
            }
        }, Qt::QueuedConnection);
    });
    processor.enqueueLines({QStringLiteral(R"({"jsonrpc":"2.0","method":"create_element","params":{"type":"And","x":0,"y":0},"id":1})"),
                            QStringLiteral(R"({"jsonrpc":"2.0","method":"get_server_info","params":{},"id":2})")});
    QCOMPARE(capture.contents().count('\n'), 0);
    QVERIFY(TestUtils::waitFor([&] { return capture.contents().count('\n') == 2; }));
    QCOMPARE(responsesWhenQueuedWorkRan, 1);
}

// These drive the real QSocketNotifier -- MCPProcessor uses a dedicated StdinReader thread
// on Windows instead (see MCPProcessor.h), so there's no m_stdinNotifier there.
#ifndef Q_OS_WIN
void TestMCPProcessor::testStartStopProcessingTogglesNotifier()
{
    ScopedStdinPipe pipe;
    MainWindow window;
    MCPProcessor processor(&window);

    QVERIFY(!processor.m_stdinNotifier);
    processor.startProcessing();
    QVERIFY(processor.m_stdinNotifier);
    QVERIFY(processor.m_stdinNotifier->isEnabled());

    processor.stopProcessing();
    QVERIFY(!processor.m_stdinNotifier->isEnabled());
}

void TestMCPProcessor::testOnStdinReadableDispatchesRealData()
{
    ScopedStdinPipe pipe;
    MainWindow window;
    MCPProcessor processor(&window);
    processor.startProcessing();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    TestUtils::ScopedStdoutCapture capture(dir.filePath("stdout.txt"));

    pipe.writeData(QByteArrayLiteral(R"({"jsonrpc":"2.0","method":"get_server_info","params":{},"id":1})") + '\n');

    // Drives the real QSocketNotifier -> onStdinReadable() -> extractStdinLines() ->
    // processIncomingData() chain end to end: the event loop must actually run for the
    // notifier's activated signal to fire.
    QVERIFY(TestUtils::waitFor([&] { return !capture.json().isEmpty(); }));

    const QJsonObject response = capture.json();
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));

    processor.stopProcessing();
}

void TestMCPProcessor::testOnStdinReadableQuitsOnEof()
{
    ScopedStdinPipe pipe;
    MainWindow window;
    MCPProcessor processor(&window);
    processor.startProcessing();
    QVERIFY(processor.m_stdinNotifier->isEnabled());

    pipe.closeWriteEnd(); // the client closed its end -- a real EOF

    QVERIFY(TestUtils::waitFor([&] { return !processor.m_stdinNotifier->isEnabled(); }));
}

void TestMCPProcessor::testEndOfInputQuitsAfterTheRequestsBeforeIt()
{
    // A client that writes its requests and closes its end at once has each answered before the processor quits
    ScopedStdinPipe pipe;
    MainWindow window;
    MCPProcessor processor(&window);
    processor.startProcessing();
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    TestUtils::ScopedStdoutCapture capture(dir.filePath("stdout.txt"));

    const QByteArray request = QByteArrayLiteral(R"({"jsonrpc":"2.0","method":"get_server_info","params":{},"id":1})") + '\n';
    QCOMPARE(execUntil([&pipe, &request] {
        pipe.writeData(request + request + request);
        pipe.closeWriteEnd();
    }, [] { return false; }), 0);
    QCOMPARE(capture.contents().count('\n'), 3);
}

void TestMCPProcessor::testClientClosingTheConnectionKeepsTheSavePromptOfAModifiedTab()
{
    // Unsaved work still gets its prompt when the client goes: the prompt is cancelled, so the window stays
    ScopedStdinPipe pipe;
    MainWindow window;
    window.show();
    MCPProcessor processor(&window);
    processor.startProcessing();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    {
        TestUtils::ScopedStdoutCapture capture(dir.filePath("stdout.txt"));
        sendLine(processor, QStringLiteral(R"({"jsonrpc":"2.0","method":"create_element","params":{"type":"And","x":0,"y":0},"id":1})"));
        QVERIFY2(capture.json().contains("result"), qPrintable(QJsonDocument(capture.json()).toJson()));
    }

    auto dismisser = TestUtils::AutoDismisser::answerMessageBox(QMessageBox::Cancel);
    QCOMPARE(execUntil([&pipe] { pipe.closeWriteEnd(); }, [&dismisser] { return dismisser.dismissCount() > 0; }), 1);
    QCOMPARE(dismisser.dismissCount(), 1);
    QVERIFY(window.isVisible());
}

void TestMCPProcessor::testClientClosingTheConnectionClosesACleanWindowWithoutAsking()
{
    // The client's end of stdin closes, the processor quits, and a window with nothing to save closes: nobody is left
    // to answer an exit question, so none is asked
    ScopedStdinPipe pipe;
    MainWindow window;
    window.show();
    MCPProcessor processor(&window);
    processor.startProcessing();

    auto dismisser = TestUtils::AutoDismisser::answerMessageBox(QMessageBox::Cancel);
    QCOMPARE(execUntil([&pipe] { pipe.closeWriteEnd(); }, [&dismisser] { return dismisser.dismissCount() > 0; }), 0);
    QCOMPARE(dismisser.dismissCount(), 0);
    QVERIFY(!window.isVisible());
}
#endif

void TestMCPProcessor::testHeapAllocationDeletesCleanly()
{
    // Every other test stack-allocates MCPProcessor, which only ever invokes the
    // base-object destructor. `delete` through a pointer instead dispatches through the
    // virtual-destructor's separate "deleting destructor" entry point (pattern 35).
    MainWindow window;
    auto *processor = new MCPProcessor(&window);
    delete processor;
}
