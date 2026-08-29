// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestUpdateController.h"

#include <QCheckBox>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QPushButton>
#include <QSignalSpy>
#include <QSslError>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUrl>
#include <QWidget>

#include "App/Core/Application.h"
#include "App/Core/Settings.h"
#include "App/UI/UpdateController.h"
#include "Tests/Common/TestUtils.h"

namespace {

// A tiny, real HTTP/1.1 server: replies with a fixed small body to every request. Mirrors
// TestUpdateChecker.cpp's local-server technique so UpdateController's real
// QNetworkAccessManager::get() path is exercised without any live network dependency.
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

// Finds a real top-level QDialog/QMessageBox and clicks the button with the given text.
TestUtils::AutoDismisser dismisserForButton(const QString &text)
{
    return TestUtils::AutoDismisser([text](QWidget *w) {
        auto *dialog = qobject_cast<QDialog *>(w);
        if (!dialog) return false;
        const auto buttons = dialog->findChildren<QAbstractButton *>();
        for (auto *button : buttons) {
            if (button->text().remove(QLatin1Char('&')) == text) {
                button->click();
                return true;
            }
        }
        return false;
    });
}

// QDesktopServices::setUrlHandler() needs a real, moc-processed slot -- a function-local class
// can't have one, so this capturer lives at namespace scope (mirrors TestExportController.cpp's
// identical helper). Requires TestUpdateController.moc at file end.
class UpdateControllerUrlOpenCapturer : public QObject
{
    Q_OBJECT
public:
    QStringList urls;
public slots:
    void handle(const QUrl &url) { urls.append(url.toString()); }
};

} // namespace

void TestUpdateController::init()
{
    Application::interactiveMode = true;
}

void TestUpdateController::testCheckForUpdatesSkippedWhenNotInteractive()
{
    Application::interactiveMode = false;

    QWidget parent;
    UpdateController controller(&parent);

    // No UpdateChecker gets constructed at all -- if it did (and somehow reached the network),
    // a stray dialog would eventually appear. Watch briefly and confirm none does.
    TestUtils::AutoDismisser dismisser([](QWidget *w) {
        auto *dialog = qobject_cast<QDialog *>(w);
        return dialog != nullptr;
    });

    controller.checkForUpdates();
    QTest::qWait(200);

    QCOMPARE(dismisser.dismissCount(), 0);
}

void TestUpdateController::testShowUpdateDialogDirectDownloadAccepted()
{
    QWidget parent;
    UpdateController controller(&parent);

    const QByteArray body = "fake update package contents";
    FakeHttpServer server(body);
    const QUrl downloadUrl = server.url("wiRedPanda-update.bin");

    const QString savePath = QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).filePath("wiRedPanda-update.bin");
    QFile::remove(savePath);

    // First dismisses "Update Available" (click Download), then later "Download Complete".
    auto dismisser = dismisserForButton(QStringLiteral("Download"));
    TestUtils::AutoDismisser infoDismisser = TestUtils::AutoDismisser::acceptMessageBox();

    controller.showUpdateDialog(QStringLiteral("99.9.9"), downloadUrl, QUrl("https://example.invalid/release"));

    QVERIFY2(TestUtils::waitFor([&] { return QFile::exists(savePath); }), "downloadUpdate() must have saved the file");
    QFile saved(savePath);
    QVERIFY(saved.open(QIODevice::ReadOnly));
    QCOMPARE(saved.readAll(), body);
    saved.close();
    QFile::remove(savePath);
}

void TestUpdateController::testShowUpdateDialogReleasePageFallbackAccepted()
{
    QWidget parent;
    UpdateController controller(&parent);

    UpdateControllerUrlOpenCapturer capturer;
    QDesktopServices::setUrlHandler(QStringLiteral("https"), &capturer, "handle");

    const QString savePath = QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).filePath("should-not-exist.bin");
    QFile::remove(savePath);

    // An empty/invalid downloadUrl means hasDirectDownload is false: accepting the dialog must
    // open the release page instead of calling downloadUpdate() at all.
    auto dismisser = dismisserForButton(QStringLiteral("Download"));
    controller.showUpdateDialog(QStringLiteral("99.9.9"), QUrl(), QUrl("https://example.invalid/release"));

    QDesktopServices::unsetUrlHandler(QStringLiteral("https"));

    QCOMPARE(capturer.urls.size(), 1);
    QCOMPARE(capturer.urls.constFirst(), QStringLiteral("https://example.invalid/release"));
    QVERIFY2(!QFile::exists(savePath), "the release-page fallback must not download any file");
}

void TestUpdateController::testShowUpdateDialogRejectedIsNoOp()
{
    QWidget parent;
    UpdateController controller(&parent);

    const QString previousSkipped = Settings::updateCheckSkippedVersion();
    Settings::setUpdateCheckSkippedVersion(QString());

    auto dismisser = dismisserForButton(QStringLiteral("Close"));
    controller.showUpdateDialog(QStringLiteral("42.0.0"), QUrl(), QUrl("https://example.invalid/release"));

    QVERIFY2(dismisser.dismissCount() >= 1, "the \"Close\" button must have been found");
    QVERIFY(Settings::updateCheckSkippedVersion().isEmpty());

    Settings::setUpdateCheckSkippedVersion(previousSkipped);
}

void TestUpdateController::testShowUpdateDialogSkipCheckboxPersistsEvenWhenRejected()
{
    QWidget parent;
    UpdateController controller(&parent);

    const QString previousSkipped = Settings::updateCheckSkippedVersion();

    TestUtils::AutoDismisser dismisser([](QWidget *w) {
        auto *dialog = qobject_cast<QDialog *>(w);
        if (!dialog) return false;
        auto *checkBox = dialog->findChild<QCheckBox *>();
        if (!checkBox) return false;
        checkBox->setChecked(true);
        for (auto *button : dialog->findChildren<QAbstractButton *>()) {
            if (button->text().remove(QLatin1Char('&')) == QStringLiteral("Close")) {
                button->click();
                return true;
            }
        }
        return false;
    });

    controller.showUpdateDialog(QStringLiteral("42.0.1"), QUrl(), QUrl("https://example.invalid/release"));

    // Suppression is recorded regardless of accept/reject -- only the checkbox state matters.
    QCOMPARE(Settings::updateCheckSkippedVersion(), QStringLiteral("42.0.1"));

    Settings::setUpdateCheckSkippedVersion(previousSkipped);
}

void TestUpdateController::testDownloadUpdateSuccessSavesFile()
{
    QWidget parent;
    UpdateController controller(&parent);

    const QByteArray body = "another fake update payload, slightly different";
    FakeHttpServer server(body);
    const QUrl url = server.url("direct-download.bin");

    const QString savePath = QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).filePath("direct-download.bin");
    QFile::remove(savePath);

    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();
    controller.downloadUpdate(QStringLiteral("7.7.7"), url);

    QVERIFY2(TestUtils::waitFor([&] { return QFile::exists(savePath); }), "the file must have been saved");
    QVERIFY2(TestUtils::waitFor([&] { return dismisser.dismissCount() >= 1; }), "the \"Download Complete\" dialog must have appeared");

    QFile saved(savePath);
    QVERIFY(saved.open(QIODevice::ReadOnly));
    QCOMPARE(saved.readAll(), body);
    saved.close();
    QFile::remove(savePath);
}

void TestUpdateController::testDownloadUpdateNetworkErrorShowsWarning()
{
    QWidget parent;
    UpdateController controller(&parent);

    // Port 1 is a reserved/unassigned TCP port practically guaranteed to have nothing
    // listening -- a real, reproducible network error without any external dependency
    // (same technique as TestUpdateChecker.cpp).
    const QUrl url("http://127.0.0.1:1/unreachable.bin");

    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();
    controller.downloadUpdate(QStringLiteral("1.0.0"), url);

    QVERIFY2(TestUtils::waitFor([&] { return dismisser.dismissCount() >= 1; }), "the \"Download Failed\" dialog must have appeared");
}

void TestUpdateController::testDownloadUpdateCanceledSkipsWarning()
{
    QWidget parent;
    UpdateController controller(&parent);

    // A slow server that never actually finishes the body -- gives the test time to cancel
    // the progress dialog before the reply completes.
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));
    QObject::connect(&server, &QTcpServer::newConnection, &server, [&server] {
        QTcpSocket *socket = server.nextPendingConnection();
        QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket] {
            socket->readAll();
            socket->write(QByteArrayLiteral("HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: 999999\r\n\r\n"));
            // Deliberately never writes the body -- the request stays open until canceled.
        });
        QObject::connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    });
    const QUrl url(QStringLiteral("http://127.0.0.1:%1/slow.bin").arg(server.serverPort()));

    bool sawFailureDialog = false;
    TestUtils::AutoDismisser dismisser([&](QWidget *w) {
        auto *box = qobject_cast<QMessageBox *>(w);
        if (!box) return false;
        sawFailureDialog = true;
        box->accept();
        return true;
    });
    TestUtils::AutoDismisser cancelDismisser([](QWidget *w) {
        auto *progress = qobject_cast<QProgressDialog *>(w);
        if (!progress) return false;
        progress->cancel();
        return true;
    });

    controller.downloadUpdate(QStringLiteral("1.0.0"), url);

    QVERIFY2(TestUtils::waitFor([&] { return cancelDismisser.dismissCount() >= 1; }),
              "the progress dialog must have appeared and been canceled");

    QTest::qWait(500);
    QVERIFY2(!sawFailureDialog, "a user-canceled download must not show the \"Download Failed\" dialog");
}

void TestUpdateController::testDownloadUpdateFileOpenFailureShowsWarning()
{
    QWidget parent;
    UpdateController controller(&parent);

    const QByteArray body = "irrelevant -- the save target itself can't be opened";
    FakeHttpServer server(body);
    // A URL with no filename component (bare "/" path) makes downloadUpdate()'s savePath
    // resolve to the Downloads directory itself -- QFile::open(WriteOnly) on a directory fails.
    const QUrl url = server.url(QString());

    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();
    controller.downloadUpdate(QStringLiteral("1.0.0"), url);

    QVERIFY2(TestUtils::waitFor([&] { return dismisser.dismissCount() >= 1; }), "the \"Download Failed\" (can't save) dialog must have appeared");
}

void TestUpdateController::testDownloadUpdateWriteFailureShowsWarning()
{
#ifdef Q_OS_WIN
    QSKIP("RLIMIT_FSIZE (used to force a deferred write failure) has no Windows equivalent");
#else
    QWidget parent;
    UpdateController controller(&parent);

    // QFile buffers writes up to QFILE_WRITEBUFFER_SIZE (16 KiB) in memory, so write() lies
    // about success for anything at or under that -- confirmed empirically (a standalone probe
    // sweeping payload sizes under the same RLIMIT_FSIZE technique showed write() reporting the
    // full size, undetected, for every payload <= 16384 bytes, and only correctly reporting a
    // short write once the payload exceeds the buffer and bypasses it). 32 KiB comfortably
    // clears that threshold so file.write(payload) below is guaranteed to return less than
    // payload.size() -- the FakeHttpServer's own socket writes aren't affected (RLIMIT_FSIZE
    // only caps regular-file writes), only the final QFile::write().
    const QByteArray body(32768, 'x');
    FakeHttpServer server(body);
    const QUrl url = server.url("write-failure.bin");

    const QString savePath = QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).filePath("write-failure.bin");
    QFile::remove(savePath);

    // Checks the dialog's actual text, not just that *some* QMessageBox appeared -- a weaker
    // assertion here previously let the test pass while silently hitting the "Download
    // Complete" dialog instead (the intended failure branch was never reached until the
    // payload was made big enough to defeat QFile's write buffering, see above).
    bool sawWriteFailureDialog = false;
    TestUtils::AutoDismisser dismisser([&](QWidget *w) {
        auto *box = qobject_cast<QMessageBox *>(w);
        if (!box) return false;
        if (box->text().contains(QStringLiteral("Could not write the file"))) {
            sawWriteFailureDialog = true;
        }
        box->accept();
        return true;
    });
    {
        TestUtils::ScopedTinyFsizeLimit tinyLimit;
        controller.downloadUpdate(QStringLiteral("1.0.0"), url);
        QVERIFY2(TestUtils::waitFor([&] { return dismisser.dismissCount() >= 1; }), "a dialog must have appeared");
    }

    QVERIFY2(sawWriteFailureDialog, "the \"Download Failed\" (can't write) dialog must have appeared");
    QFile::remove(savePath);
#endif
}

void TestUpdateController::testDownloadUpdateSslErrorsAbortsReply()
{
    QWidget parent;
    UpdateController controller(&parent);

    // A server that accepts the connection but never replies -- downloadUpdate()'s real
    // request stays in flight, which is all we need: its QNetworkAccessManager gets
    // constructed and the sslErrors handler gets connected before this function returns.
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));
    QObject::connect(&server, &QTcpServer::newConnection, &server, [&server] {
        QTcpSocket *socket = server.nextPendingConnection();
        QObject::connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    });
    const QUrl url(QStringLiteral("http://127.0.0.1:%1/slow.bin").arg(server.serverPort()));

    controller.downloadUpdate(QStringLiteral("1.0.0"), url);

    auto *network = controller.findChild<QNetworkAccessManager *>();
    QVERIFY2(network, "downloadUpdate() must have created its own QNetworkAccessManager as a child of the controller");

    // Invoking QNetworkAccessManager::sslErrors directly (the same mechanism `emit` compiles
    // to) exercises the connected lambda without needing a real TLS handshake failure --
    // same technique as TestUpdateChecker::testSslErrorsAbortsReply(). The "data:" reply
    // just needs to be *some* real QNetworkReply for the lambda's reply->abort() to act on.
    QNetworkReply *dummyReply = network->get(QNetworkRequest(QUrl(QStringLiteral("data:text/plain,x"))));

    const QList<QSslError> errors{QSslError(QSslError::SelfSignedCertificate)};
    QTest::ignoreMessage(QtWarningMsg, "MainWindow::downloadUpdate: SSL errors, aborting reply: QList(\"The certificate is self-signed, and untrusted\")");
    QMetaObject::invokeMethod(network, "sslErrors", Q_ARG(QNetworkReply *, dummyReply), Q_ARG(QList<QSslError>, errors));

    QSignalSpy spy(dummyReply, &QNetworkReply::finished);
    QVERIFY(spy.wait(1000));
    QVERIFY(dummyReply->error() == QNetworkReply::NoError || dummyReply->error() == QNetworkReply::OperationCanceledError);
}

#include "TestUpdateController.moc"
