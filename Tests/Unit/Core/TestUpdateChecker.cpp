// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestUpdateChecker.h"

#include <memory>

#include <QDate>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSignalSpy>
#include <QSslError>
#include <QSysInfo>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>

#include "App/Core/Settings.h"
#include "App/Core/UpdateChecker.h"
#include "Tests/Common/TestUtils.h"

namespace {

// Resolves a real QNetworkReply for `body` without any actual network I/O:
// Qt's QNetworkAccessManager has a built-in backend for the "data:" URL
// scheme that hands back the decoded payload asynchronously via the normal
// finished()/error() signals, exactly like a real HTTP response would.
QNetworkReply *fetchDataUrl(QNetworkAccessManager &manager, const QByteArray &body)
{
    const QUrl url(QStringLiteral("data:application/json;base64,") + QString::fromLatin1(body.toBase64()));
    QNetworkReply *reply = manager.get(QNetworkRequest(url));
    QSignalSpy spy(reply, &QNetworkReply::finished);
    Q_UNUSED(spy.wait(2000));
    return reply;
}

} // namespace

void TestUpdateChecker::init()
{
    Settings::setUpdateCheckLastDate("");
}

void TestUpdateChecker::testUpdateAvailable()
{
    // shouldOfferUpdate must offer any release strictly newer than the running
    // version, unless the user suppressed exactly that version.
    const QVersionNumber current(5, 1, 2);

    QVERIFY(shouldOfferUpdate("5.1.3", current, {}));   // newer patch
    QVERIFY(shouldOfferUpdate("5.2", current, {}));     // newer minor
    QVERIFY(shouldOfferUpdate("6.0", current, {}));     // newer major
    QVERIFY(shouldOfferUpdate("5.1.3.1", current, {})); // extra segment, still newer

    // Suppressing a DIFFERENT version must not swallow the new release.
    QVERIFY(shouldOfferUpdate("5.1.3", current, "5.1.1"));
    QVERIFY(shouldOfferUpdate("5.2", current, "5.1.3"));
}

void TestUpdateChecker::testNoUpdate()
{
    const QVersionNumber current(5, 1, 2);

    // Same or older releases are never offered.
    QVERIFY(!shouldOfferUpdate("5.1.2", current, {}));
    QVERIFY(!shouldOfferUpdate("5.1.1", current, {}));
    QVERIFY(!shouldOfferUpdate("5.0", current, {}));
    QVERIFY(!shouldOfferUpdate("4.9.9", current, {}));

    // Trailing zeros normalize away: "5.1.2.0" is the running version.
    QVERIFY(!shouldOfferUpdate("5.1.2.0", current, {}));

    // Malformed or empty tags parse to a null version — never offered.
    QVERIFY(!shouldOfferUpdate({}, current, {}));
    QVERIFY(!shouldOfferUpdate("not-a-version", current, {}));

    // Per-version suppression: the suppressed release itself stays silent.
    // (The suppression is matched against the NORMALIZED version string.)
    QVERIFY(!shouldOfferUpdate("5.1.3", current, "5.1.3"));
    QVERIFY(!shouldOfferUpdate("5.2.0", current, "5.2"));
}

void TestUpdateChecker::testReleaseAssetKey()
{
    // Each platform/arch combination must select its own key in latest-release.json.
    QCOMPARE(releaseAssetKey("Linux", "x86_64"), QStringLiteral("linuxX64"));
    QCOMPARE(releaseAssetKey("Linux", "arm64"), QStringLiteral("linuxArm64"));
    QCOMPARE(releaseAssetKey("Windows", "x86_64"), QStringLiteral("windowsX64"));
    QCOMPARE(releaseAssetKey("Windows", "arm64"), QStringLiteral("windowsArm64"));

    // macOS: single universal DMG, keyed on platform regardless of arch token.
    QCOMPARE(releaseAssetKey("macOS", "x86_64"), QStringLiteral("macosUniversal"));
    QCOMPARE(releaseAssetKey("macOS", "arm64"), QStringLiteral("macosUniversal"));

    // Unknown architecture resolves to no key where one is required — the caller
    // then falls back to the release page rather than a wrong binary.
    QVERIFY(releaseAssetKey("Linux", "ppc64").isEmpty());
    QVERIFY(releaseAssetKey("Windows", "ppc64").isEmpty());

    // Unknown platform resolves to no key at all.
    QVERIFY(releaseAssetKey("BeOS", "x86_64").isEmpty());
}

void TestUpdateChecker::testSafeGitHubUrl()
{
    // Regression: a download URL from the (response-controlled) release data was used for a
    // network download + file write with no scheme/host check. The site's latest-release.json
    // only ever populates that field as https://github.com/... — the CDN redirect happens only
    // after fetching this URL, so its host never appears here.
    QVERIFY(isSafeGitHubUrl(QUrl("https://github.com/gibis-unifesp/wiredpanda/releases/download/5.1.2/wiRedPanda.AppImage")));
    QVERIFY(isSafeGitHubUrl(QUrl("https://github.com/gibis-unifesp/wiredpanda/releases/tag/5.1.2")));

    // Wrong scheme: could otherwise have the app silently copy a local file into Downloads,
    // or hand an unexpected URI scheme to the OS's protocol handler.
    QVERIFY(!isSafeGitHubUrl(QUrl("file:///etc/passwd")));
    QVERIFY(!isSafeGitHubUrl(QUrl("ftp://github.com/some/asset")));

    // Lookalike hosts must not pass a substring/prefix check.
    QVERIFY(!isSafeGitHubUrl(QUrl("https://github.com.evil.com/asset")));
    QVERIFY(!isSafeGitHubUrl(QUrl("https://evil.com/github.com")));

    // Invalid/empty URL.
    QVERIFY(!isSafeGitHubUrl(QUrl()));
    QVERIFY(!isSafeGitHubUrl(QUrl(QString())));
}

void TestUpdateChecker::testOnReplyFinishedOffersNewerVersionWithMatchingAsset()
{
    // Keys for every platform/arch releaseAssetKey() can produce, so this test runs
    // identically regardless of which platform actually builds/runs it.
    const QByteArray body = R"({
        "version": "99.5.3",
        "linuxX64": "https://github.com/gibis-unifesp/wiredpanda/releases/download/99.5.3/wiRedPanda-99.5.3-Linux-x86_64-Qt6.9.3.AppImage",
        "linuxArm64": "https://github.com/gibis-unifesp/wiredpanda/releases/download/99.5.3/wiRedPanda-99.5.3-Linux-arm64-Qt6.9.3.AppImage",
        "windowsX64": "https://github.com/gibis-unifesp/wiredpanda/releases/download/99.5.3/wiRedPanda-99.5.3-Windows-x86_64-Qt6.9.3-Portable.zip",
        "windowsArm64": "https://github.com/gibis-unifesp/wiredpanda/releases/download/99.5.3/wiRedPanda-99.5.3-Windows-arm64-Qt6.9.3-Portable.zip",
        "macosUniversal": "https://github.com/gibis-unifesp/wiredpanda/releases/download/99.5.3/wiRedPanda-99.5.3-macOS-Qt6.9.3.dmg"
    })";

    QNetworkAccessManager manager;
    QNetworkReply *reply = fetchDataUrl(manager, body);

    UpdateChecker checker;
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);
    checker.onReplyFinished(reply);

    QCOMPARE(spy.size(), 1);
    const QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), QStringLiteral("99.5.3"));

    // releaseAssetKey() only maps x86_64/arm64 -- a build/runner architecture outside that
    // set (e.g. 32-bit x86 on the CI's dedicated i386 job) genuinely has no asset key and no
    // published binary, so an empty download URL is the correct behavior there, not a bug.
#if defined(Q_OS_WIN)
    const QString platform = QStringLiteral("Windows");
#elif defined(Q_OS_MACOS)
    const QString platform = QStringLiteral("macOS");
#elif defined(Q_OS_LINUX)
    const QString platform = QStringLiteral("Linux");
#else
    const QString platform;
#endif
    const bool hasAsset = !releaseAssetKey(platform, QSysInfo::buildCpuArchitecture()).isEmpty();
    QCOMPARE(!args.at(1).toUrl().isEmpty(), hasAsset);
    QCOMPARE(args.at(2).toUrl(), QUrl("https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/tag/99.5.3"));

    // A successful, parseable reply must record today's date as the daily check.
    QVERIFY(!Settings::updateCheckLastDate().isEmpty());
}

void TestUpdateChecker::testOnReplyFinishedNoMatchingAssetLeavesDownloadUrlEmpty()
{
    // No per-platform key at all: releaseAssetKey() resolves to a real key for whatever
    // platform/arch this test runs on, but the JSON simply doesn't carry it -- regardless
    // of platform, doc.object().value(key) is then null and downloadUrl stays empty.
    const QByteArray body = R"({
        "version": "99.5.3"
    })";

    QNetworkAccessManager manager;
    QNetworkReply *reply = fetchDataUrl(manager, body);

    UpdateChecker checker;
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);
    checker.onReplyFinished(reply);

    QCOMPARE(spy.size(), 1);
    const QList<QVariant> args = spy.takeFirst();
    QVERIFY(args.at(1).toUrl().isEmpty());
}

void TestUpdateChecker::testOnReplyFinishedNetworkErrorSkipsEverything()
{
    // Port 1 is a reserved/unassigned TCP port practically guaranteed to have
    // nothing listening — a real, reproducible network error without any
    // external dependency. On Windows CI, surfacing the refusal takes noticeably
    // longer than on Linux/macOS (confirmed empirically: an equivalent wait using
    // the same port there took ~4s), so this needs a generous margin.
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl("http://127.0.0.1:1/")));
    QSignalSpy errorSpy(reply, &QNetworkReply::errorOccurred);
    QVERIFY(errorSpy.wait(10000));
    QVERIFY(reply->error() != QNetworkReply::NoError);

    UpdateChecker checker;
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);
    checker.onReplyFinished(reply);

    QCOMPARE(spy.size(), 0);
    QVERIFY(Settings::updateCheckLastDate().isEmpty());
}

void TestUpdateChecker::testOnReplyFinishedMalformedJsonSkipsEverything()
{
    QNetworkAccessManager manager;
    QNetworkReply *reply = fetchDataUrl(manager, "this is not valid json at all");

    UpdateChecker checker;
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);
    checker.onReplyFinished(reply);

    QCOMPARE(spy.size(), 0);
    QVERIFY(Settings::updateCheckLastDate().isEmpty());
}

void TestUpdateChecker::testOnReplyFinishedUpToDateRecordsCheckDateButNoSignal()
{
    // Older than any real running version — shouldOfferUpdate() must reject it,
    // but the reply was still successfully parsed, so today's check still counts.
    const QByteArray body = R"({
        "version": "1.0.0"
    })";

    QNetworkAccessManager manager;
    QNetworkReply *reply = fetchDataUrl(manager, body);

    UpdateChecker checker;
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);
    checker.onReplyFinished(reply);

    QCOMPARE(spy.size(), 0);
    QVERIFY(!Settings::updateCheckLastDate().isEmpty());
}

void TestUpdateChecker::testOnReplyFinishedUnsafeDownloadUrlFallsBackToReleasePage()
{
    // The release page URL is always constructed from a validated version number, never
    // sourced from JSON, so it can't be spoofed via the reply -- only the download URL
    // (still JSON-sourced) can be. An unsafe/unexpected download asset URL must still
    // emit updateAvailable (the release page is a safe manual fallback), just with
    // downloadUrl cleared rather than passed through.
    const QByteArray body = R"({
        "version": "99.5.3",
        "linuxX64": "https://evil.example.com/malicious.AppImage",
        "linuxArm64": "https://evil.example.com/malicious.AppImage",
        "windowsX64": "https://evil.example.com/malicious.zip",
        "windowsArm64": "https://evil.example.com/malicious.zip",
        "macosUniversal": "https://evil.example.com/malicious.dmg"
    })";

    QNetworkAccessManager manager;
    QNetworkReply *reply = fetchDataUrl(manager, body);

    UpdateChecker checker;
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);
    checker.onReplyFinished(reply);

    QCOMPARE(spy.size(), 1);
    const QList<QVariant> args = spy.takeFirst();
    QVERIFY(args.at(1).toUrl().isEmpty());
    QCOMPARE(args.at(2).toUrl(), QUrl("https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/tag/99.5.3"));
}

void TestUpdateChecker::testCheckForUpdatesSkippedWhenDisabled()
{
    Settings::setUpdateChecksDisabled(true);

    UpdateChecker checker;
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);
    checker.checkForUpdates();

    QCOMPARE(spy.size(), 0);

    Settings::setUpdateChecksDisabled(false);
}

void TestUpdateChecker::testCheckForUpdatesSkippedWhenAlreadyCheckedToday()
{
    Settings::setUpdateChecksDisabled(false);
    Settings::setUpdateCheckLastDate(QDate::currentDate().toString(Qt::ISODate));

    UpdateChecker checker;
    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);
    checker.checkForUpdates();

    QCOMPARE(spy.size(), 0);
}

void TestUpdateChecker::testCheckForUpdatesBackpressuresOversizedDownload()
{
    // Defense-in-depth: reply->setReadBufferSize() bounds how much of a
    // hostile/corrupted endpoint's response Qt will admit into memory before
    // the application reads it — confirmed here against a real, incrementally
    // -delivered local HTTP response streaming well past the 1 MiB cap:
    // received plateaus at the cap and never grows further, regardless of how
    // much more the server sends, because nothing ever reads from the reply.
    Settings::setUpdateChecksDisabled(false);
    Settings::setUpdateCheckLastDate("");

    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));

    const QByteArray chunk(64 * 1024, 'x'); // 64 KiB filler
    // Chunks are written one per timer tick (not in a tight loop) so the
    // client's event loop gets to process each one as its own readyRead
    // cycle instead of coalescing the whole 2.5 MiB into a single OS-level
    // read on a fast loopback connection.
    auto *sendTimer = new QTimer(&server);
    auto chunksSent = std::make_shared<int>(0);
    QObject::connect(&server, &QTcpServer::newConnection, &server, [&server, &chunk, sendTimer, chunksSent]() {
        QTcpSocket *socket = server.nextPendingConnection();
        QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket, &chunk, sendTimer, chunksSent]() {
            socket->readAll(); // drain the request line/headers
            const qint64 totalBodySize = chunk.size() * 40;
            socket->write(QStringLiteral("HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: %1\r\n\r\n")
                              .arg(totalBodySize).toUtf8());
            // Stream well past the 1 MiB threshold (40 * 64 KiB = 2.5 MiB).
            QObject::connect(sendTimer, &QTimer::timeout, socket, [socket, &chunk, sendTimer, chunksSent]() {
                if (*chunksSent >= 40 || socket->state() != QAbstractSocket::ConnectedState) {
                    sendTimer->stop();
                    return;
                }
                socket->write(chunk);
                ++*chunksSent;
            });
            sendTimer->start(20);
        });
        QObject::connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    });

    UpdateChecker checker;
    checker.setApiUrlForTesting(QUrl(QStringLiteral("http://127.0.0.1:%1/").arg(server.serverPort())));

    QSignalSpy spy(&checker, &UpdateChecker::updateAvailable);
    checker.checkForUpdates();
    QTest::qWait(3000); // let the transfer stream well past the cap

    // The reply never finishes (nothing drains it past the buffered cap), so
    // onReplyFinished() never runs: no signal, no recorded check date.
    QCOMPARE(spy.size(), 0);
    QVERIFY(Settings::updateCheckLastDate().isEmpty());
    // The server tried to send far more than 1 MiB; if Qt's backpressure had
    // failed to cap admission, this would be a much larger number.
    QVERIFY2(*chunksSent > 16, "server should still be actively streaming past the buffer cap");
}

void TestUpdateChecker::testSslErrorsAbortsReply()
{
    // Invoking QNetworkAccessManager::sslErrors directly (the same mechanism
    // `emit` compiles to) exercises UpdateChecker's connected lambda without
    // needing a real TLS handshake failure.
    UpdateChecker checker;
    QNetworkReply *reply = checker.m_network.get(QNetworkRequest(QUrl(QStringLiteral("data:application/json,{}"))));

    const QList<QSslError> errors{QSslError(QSslError::SelfSignedCertificate)};
    QTest::ignoreMessage(QtWarningMsg, "UpdateChecker: SSL errors, aborting reply: QList(\"The certificate is self-signed, and untrusted\")");
    QMetaObject::invokeMethod(&checker.m_network, "sslErrors", Q_ARG(QNetworkReply *, reply), Q_ARG(QList<QSslError>, errors));

    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait(1000));
    // A "data:" reply may already be internally complete by the time abort()
    // runs, in which case it's a no-op and the reply still reports NoError —
    // either outcome proves the lambda ran (the ignored qWarning() above is
    // the real assertion that abort() was actually invoked).
    QVERIFY(reply->error() == QNetworkReply::NoError || reply->error() == QNetworkReply::OperationCanceledError);
}
