// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickUpdateController.h"

#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>

#include "App/Core/Application.h"
#include "App/Core/Settings.h"
#include "App/QuickShell/Chrome/QuickUpdateController.h"

namespace {

// A tiny, real HTTP/1.1 server: replies with a fixed small body to every request. Mirrors
// Tests/Unit/Ui/TestUpdateController.cpp's identical FakeHttpServer technique, so
// QuickUpdateController::downloadUpdate()'s real QNetworkAccessManager::get() path is
// exercised without any live network dependency.
class FakeHttpServer : public QObject
{
public:
    explicit FakeHttpServer(const QByteArray &body, QObject *parent = nullptr)
        : QObject(parent)
        , m_body(body)
    {
        m_server.listen(QHostAddress::LocalHost);
        connect(&m_server, &QTcpServer::newConnection, this, [this] {
            QTcpSocket *socket = m_server.nextPendingConnection();
            connect(socket, &QTcpSocket::readyRead, this, [this, socket] {
                socket->readAll(); // drain the request line/headers
                socket->write(QStringLiteral("HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: %1\r\n\r\n")
                                  .arg(m_body.size()).toUtf8());
                socket->write(m_body);
            });
            connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
        });
    }

    QUrl url(const QString &fileName) const
    {
        return QUrl(QStringLiteral("http://127.0.0.1:%1/%2").arg(m_server.serverPort()).arg(fileName));
    }

private:
    QTcpServer m_server;
    QByteArray m_body;
};

} // namespace

void TestQuickUpdateController::testAutoCheckEnabledDefaultsToTrueAndRoundTrips()
{
    Settings::setUpdateChecksDisabled(false);
    QuickUpdateController controller;
    QVERIFY(controller.isAutoCheckEnabled());

    QSignalSpy spy(&controller, &QuickUpdateController::autoCheckEnabledChanged);
    controller.setAutoCheckEnabled(false);
    QCOMPARE(spy.size(), 1);
    QVERIFY(!controller.isAutoCheckEnabled());
    QVERIFY(Settings::updateChecksDisabled());

    controller.setAutoCheckEnabled(true);
    QVERIFY(controller.isAutoCheckEnabled());
    QVERIFY(!Settings::updateChecksDisabled());
}

void TestQuickUpdateController::testSkipVersionPersists()
{
    QuickUpdateController controller;
    controller.skipVersion("9.9.9");
    QCOMPARE(Settings::updateCheckSkippedVersion(), QString("9.9.9"));
}

void TestQuickUpdateController::testCheckForUpdatesSkippedWhenNotInteractive()
{
    const bool wasInteractive = Application::interactiveMode;
    Application::interactiveMode = false;

    QuickUpdateController controller;
    QSignalSpy spy(&controller, &QuickUpdateController::updateAvailable);
    controller.checkForUpdates(); // must not construct/start an UpdateChecker at all

    Application::interactiveMode = wasInteractive;
    QCOMPARE(spy.size(), 0);
}

void TestQuickUpdateController::testDownloadUpdateSuccessEmitsProgressAndFinished()
{
    const QByteArray body(4096, 'x');
    FakeHttpServer server(body);

    // Writes to (and cleans up from) the real Downloads folder -- same as
    // Tests/Unit/Ui/TestUpdateController.cpp's identical download tests; no HOME/XDG
    // redirection exists for QStandardPaths in this test environment either.
    QuickUpdateController controller;
    QSignalSpy progressSpy(&controller, &QuickUpdateController::downloadProgress);
    QSignalSpy finishedSpy(&controller, &QuickUpdateController::downloadFinished);

    controller.downloadUpdate("1.2.3", server.url("wiredpanda-1.2.3.bin"));
    QVERIFY(finishedSpy.wait(5000));

    QCOMPARE(finishedSpy.size(), 1);
    QVERIFY(finishedSpy.at(0).at(0).toBool());
    QVERIFY(progressSpy.size() >= 1);

    const QString savePath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/wiredpanda-1.2.3.bin";
    QVERIFY2(QFile::exists(savePath), qPrintable(savePath));
    QFile file(savePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), body);
    file.close();
    QFile::remove(savePath);
}

void TestQuickUpdateController::testDownloadUpdateNoOpWhileAlreadyInFlight()
{
    QTcpServer server; // listens but never replies -- keeps the first download "in flight"
    QVERIFY(server.listen(QHostAddress::LocalHost));
    QObject::connect(&server, &QTcpServer::newConnection, &server, [&server] {
        server.nextPendingConnection(); // accept, then just hold the connection open
    });
    const QUrl slowUrl(QStringLiteral("http://127.0.0.1:%1/slow.bin").arg(server.serverPort()));

    QuickUpdateController controller;
    controller.downloadUpdate("1.0.0", slowUrl);

    QSignalSpy progressSpy(&controller, &QuickUpdateController::downloadProgress);
    controller.downloadUpdate("1.0.0", slowUrl); // second call while the first is still in flight
    QTest::qWait(50);
    QCOMPARE(progressSpy.size(), 0); // no second request was ever started

    controller.cancelDownload();
}

void TestQuickUpdateController::testCancelDownloadAbortsWithoutFinishedSignal()
{
    QTcpServer server; // listens but never replies
    QVERIFY(server.listen(QHostAddress::LocalHost));
    QObject::connect(&server, &QTcpServer::newConnection, &server, [&server] {
        server.nextPendingConnection();
    });
    const QUrl slowUrl(QStringLiteral("http://127.0.0.1:%1/slow.bin").arg(server.serverPort()));

    QuickUpdateController controller;
    QSignalSpy finishedSpy(&controller, &QuickUpdateController::downloadFinished);

    controller.downloadUpdate("1.0.0", slowUrl);
    controller.cancelDownload();
    QTest::qWait(200);

    QCOMPARE(finishedSpy.size(), 0); // a user-initiated cancel is not reported as a failure

    // A fresh download must be startable again -- confirms cancelDownload() actually cleared
    // the in-flight guard, not just aborted the reply object.
    const QByteArray body("ok");
    FakeHttpServer okServer(body);
    QSignalSpy secondFinishedSpy(&controller, &QuickUpdateController::downloadFinished);
    controller.downloadUpdate("1.0.0", okServer.url("retry.bin"));
    QVERIFY(secondFinishedSpy.wait(5000));
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/retry.bin");
}
