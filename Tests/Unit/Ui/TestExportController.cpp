// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestExportController.h"

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QWidget>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/ExportController.h"
#include "Tests/Common/StubFileDialogProvider.h"
#include "Tests/Common/StubMainWindowHost.h"
#include "Tests/Common/TestUtils.h"

namespace {

struct ExportControllerFixture {
    StubMainWindowHost host;
    WorkSpace tab;
    QWidget widget;

    ExportControllerFixture()
    {
        host.currentTabValue = &tab;
        host.widgetValue = &widget;
    }
};

// Deletes the file at `path` when it goes out of scope, regardless of an assertion failure
// part-way through the test -- QVERIFY/QCOMPARE return early on failure, so a plain
// end-of-test QFile::remove() call could otherwise be skipped and leak a real file into
// QDir::home() (where ExportController::exportToArduino()/exportToSystemVerilog() write).
struct ScopedFileRemoval {
    QString path;
    ~ScopedFileRemoval() { if (!path.isEmpty()) QFile::remove(path); }
};

} // namespace

// QDesktopServices::setUrlHandler() needs a real, moc-processed slot -- a function-local
// class can't have one (moc doesn't see local classes), so this capturer lives at namespace
// scope instead of inside the tests that use it.
class ExportControllerUrlOpenCapturer : public QObject
{
    Q_OBJECT
public:
    QStringList urls;
public slots:
    void handle(const QUrl &url) { urls.append(url.toLocalFile()); }
};

void TestExportController::testExportToArduinoGuardsMissingTab()
{
    StubMainWindowHost host; // currentTabValue left null
    ExportController controller(host);

    controller.exportToArduino(QStringLiteral("out.ino"));

    QCOMPARE(host.statusMessages.size(), 0);
}

void TestExportController::testExportToArduinoThrowsOnEmptyFileName()
{
    ExportControllerFixture f;
    ExportController controller(f.host);
    QVERIFY_THROWS(std::exception, controller.exportToArduino(QString()));
}

void TestExportController::testExportToArduinoThrowsOnEmptyScene()
{
    ExportControllerFixture f; // brand-new WorkSpace, no elements
    ExportController controller(f.host);
    QVERIFY_THROWS(std::exception, controller.exportToArduino(QStringLiteral("out.ino")));
}

void TestExportController::testExportToArduinoAppendsExtensionAndGenerates()
{
    ExportControllerFixture f;
    f.tab.scene()->addItem(new InputSwitch());
    ExportController controller(f.host);

    ScopedFileRemoval cleanup;
    cleanup.path = QDir::home().absoluteFilePath(QStringLiteral("wiredpanda_test_export_transient.ino"));
    QFile::remove(cleanup.path); // in case a prior interrupted run left it behind

    controller.exportToArduino(QStringLiteral("wiredpanda_test_export_transient")); // no .ino suffix

    QVERIFY2(QFile::exists(cleanup.path), "the .ino extension must have been appended");
    QCOMPARE(f.host.statusMessages.size(), 1);
}

void TestExportController::testExportToSystemVerilogGuardsMissingTab()
{
    StubMainWindowHost host;
    ExportController controller(host);

    controller.exportToSystemVerilog(QStringLiteral("out.sv"));

    QCOMPARE(host.statusMessages.size(), 0);
}

void TestExportController::testExportToSystemVerilogThrowsOnEmptyFileName()
{
    ExportControllerFixture f;
    ExportController controller(f.host);
    QVERIFY_THROWS(std::exception, controller.exportToSystemVerilog(QString()));
}

void TestExportController::testExportToSystemVerilogThrowsOnEmptyScene()
{
    ExportControllerFixture f;
    ExportController controller(f.host);
    QVERIFY_THROWS(std::exception, controller.exportToSystemVerilog(QStringLiteral("out.sv")));
}

void TestExportController::testExportToSystemVerilogAppendsExtensionAndGenerates()
{
    ExportControllerFixture f;
    f.tab.scene()->addItem(new InputSwitch());
    ExportController controller(f.host);

    ScopedFileRemoval cleanup;
    cleanup.path = QDir::home().absoluteFilePath(QStringLiteral("wiredpanda_test_export_transient.sv"));
    QFile::remove(cleanup.path);

    controller.exportToSystemVerilog(QStringLiteral("wiredpanda_test_export_transient")); // no .sv suffix

    QVERIFY2(QFile::exists(cleanup.path), "the .sv extension must have been appended");
    QCOMPARE(f.host.statusMessages.size(), 1);
}

void TestExportController::testExportToWaveFormFileThrowsOnEmptyFileName()
{
    ExportControllerFixture f;
    ExportController controller(f.host);
    QVERIFY_THROWS(std::exception, controller.exportToWaveFormFile(QString()));
}

void TestExportController::testExportToWaveFormFileGeneratesAndPrints()
{
    ExportControllerFixture f;
    f.tab.scene()->addItem(new InputSwitch());
    f.tab.scene()->addItem(new Led()); // BewavedDolphin requires at least one output element
    ExportController controller(f.host);

    QTemporaryDir capDir;
    QVERIFY(capDir.isValid());
    TestUtils::ScopedStdoutCapture capture(capDir.filePath("stdout.txt"));

    // A non-existent .dolphin path is a safe, deterministic no-op inside
    // BewavedDolphin::createWaveform(fileName) (see its own "file does not exist" branch) --
    // it still fully initializes the model, so print() below produces real CSV output.
    controller.exportToWaveFormFile(QStringLiteral("nonexistent_waveform.dolphin"));

    QVERIFY2(!capture.contents().isEmpty(), "print() must have written CSV output to stdout");
}

void TestExportController::testExportToWaveFormTerminalReadsStdinAndPrints()
{
    ExportControllerFixture f;
    f.tab.scene()->addItem(new InputSwitch());
    f.tab.scene()->addItem(new Led()); // BewavedDolphin requires at least one output element
    ExportController controller(f.host);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QFile stdinFile(dir.filePath("stdin.txt"));
    QVERIFY(stdinFile.open(QIODevice::WriteOnly));
    stdinFile.write("1,1\n0\n"); // 1 row, 1 column, single all-zero input row
    stdinFile.close();

    TestUtils::ScopedStdoutCapture capture(dir.filePath("stdout.txt"));
    TestUtils::ScopedStdinRedirect stdinRedirect(dir.filePath("stdin.txt"));

    controller.exportToWaveFormTerminal();

    QVERIFY2(!capture.contents().isEmpty(), "print() must have written CSV output to stdout");
}

void TestExportController::testExportArduinoDialogGuardsMissingTab()
{
    StubMainWindowHost host;
    ExportController controller(host);
    ScopedFileDialogStub dialogStub;

    controller.exportArduinoDialog();

    QCOMPARE(dialogStub.stub.saveCallCount, 0);
}

void TestExportController::testExportSystemVerilogDialogGuardsMissingTab()
{
    StubMainWindowHost host;
    ExportController controller(host);
    ScopedFileDialogStub dialogStub;

    controller.exportSystemVerilogDialog();

    QCOMPARE(dialogStub.stub.saveCallCount, 0);
}

void TestExportController::testExportPdfDialogGuardsMissingTab()
{
    StubMainWindowHost host;
    ExportController controller(host);
    ScopedFileDialogStub dialogStub;

    controller.exportPdfDialog();

    QCOMPARE(dialogStub.stub.saveCallCount, 0);
}

void TestExportController::testExportPdfDialogCancelledIsNoOp()
{
    ExportControllerFixture f;
    ExportController controller(f.host);

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.saveResult.fileName = QString();

    controller.exportPdfDialog();

    QCOMPARE(dialogStub.stub.saveCallCount, 1);
    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestExportController::testExportPdfDialogAppendsExtensionAndOpensFile()
{
    ExportControllerFixture f;
    f.tab.scene()->addItem(new InputSwitch());
    ExportController controller(f.host);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString pathNoExt = dir.filePath("circuit");

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.saveResult.fileName = pathNoExt;

    ExportControllerUrlOpenCapturer capturer;
    QDesktopServices::setUrlHandler(QStringLiteral("file"), &capturer, "handle");

    controller.exportPdfDialog();

    QDesktopServices::unsetUrlHandler(QStringLiteral("file"));

    QVERIFY2(QFile::exists(pathNoExt + ".pdf"), "the .pdf extension must have been appended");
    QCOMPARE(f.host.statusMessages.size(), 1);
    QCOMPARE(capturer.urls.size(), 1);
    QCOMPARE(capturer.urls.constFirst(), pathNoExt + ".pdf");
}

void TestExportController::testExportImageDialogGuardsMissingTab()
{
    StubMainWindowHost host;
    ExportController controller(host);
    ScopedFileDialogStub dialogStub;

    controller.exportImageDialog();

    QCOMPARE(dialogStub.stub.saveCallCount, 0);
}

void TestExportController::testExportImageDialogCancelledIsNoOp()
{
    ExportControllerFixture f;
    ExportController controller(f.host);

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.saveResult.fileName = QString();

    controller.exportImageDialog();

    QCOMPARE(dialogStub.stub.saveCallCount, 1);
    QCOMPARE(f.host.statusMessages.size(), 0);
}

void TestExportController::testExportImageDialogAppendsExtensionAndOpensFile()
{
    ExportControllerFixture f;
    f.tab.scene()->addItem(new InputSwitch());
    ExportController controller(f.host);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString pathNoExt = dir.filePath("circuit");

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.saveResult.fileName = pathNoExt;

    ExportControllerUrlOpenCapturer capturer;
    QDesktopServices::setUrlHandler(QStringLiteral("file"), &capturer, "handle");

    controller.exportImageDialog();

    QDesktopServices::unsetUrlHandler(QStringLiteral("file"));

    QVERIFY2(QFile::exists(pathNoExt + ".png"), "the .png extension must have been appended");
    QCOMPARE(f.host.statusMessages.size(), 1);
    QCOMPARE(capturer.urls.size(), 1);
    QCOMPARE(capturer.urls.constFirst(), pathNoExt + ".png");
}

#include "TestExportController.moc"
