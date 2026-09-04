// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickDialogProvider.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QTimer>
#include <QUrl>

#include "App/QuickShell/Chrome/QuickDialogProvider.h"

void TestQuickDialogProvider::init()
{
    m_engine = new QQmlEngine();

    QQmlComponent component(m_engine);
    component.setData(QByteArrayLiteral("import QtQuick.Window\nWindow {}"), QUrl());
    QObject *root = component.create();
    QVERIFY2(root, qPrintable(component.errorString()));

    // Own the window explicitly -- QML/JS ownership rules can otherwise garbage-collect a
    // parentless top-level Window created from a QQmlComponent.
    QQmlEngine::setObjectOwnership(root, QQmlEngine::CppOwnership);

    m_window = qobject_cast<QQuickWindow *>(root);
    QVERIFY(m_window);

    // Shown (not just constructed): TextPromptDialog.qml is a QtQuick.Controls Dialog
    // (Popup/Overlay-based), unlike the other two QQuickAbstractDialog-based dialogs -- matches
    // this project's own Widgets-side dialog-test convention
    // (Tests/Integration/TestFileDialogProvider.cpp's createWindow()) rather than risk a
    // Popup/Overlay edge case on an unshown window under offscreen QPA.
    m_window->show();
    QVERIFY(QTest::qWaitForWindowExposed(m_window));
}

void TestQuickDialogProvider::cleanup()
{
    delete m_window;
    m_window = nullptr;
    delete m_engine;
    m_engine = nullptr;
}

namespace {

// dialogOpened() fires before execModal()'s nested QEventLoop starts -- driving the dialog
// synchronously from inside it (calling accept()/reject() directly) runs before that loop
// exists to be woken, silently hanging forever. Deferring via QTimer::singleShot(0, ...) lets
// execModal()'s loop start first.
void deferDrive(QObject *dialog, const std::function<void(QObject *)> &drive)
{
    QTimer::singleShot(0, dialog, [dialog, drive] { drive(dialog); });
}

} // namespace

void TestQuickDialogProvider::testChoiceReturnsClickedButton()
{
    QuickDialogProvider provider(m_window);
    connect(&provider, &QuickDialogProvider::dialogOpened, &provider, [](QObject *dialog) {
        deferDrive(dialog, [](QObject *dlg) {
            // Invoke handleButtonClicked() with the dialog's own raw platform button value for
            // "Yes" (buttonOrder[1], matching DialogButton::Yes's declaration-order index) --
            // not a hardcoded QPlatformDialogHelper::StandardButton constant, so this test
            // doesn't depend on those exact numeric values. handleButtonClicked(button) is an
            // untyped QML function (compiles to a QVariant parameter, not int) -- Q_ARG(int, ...)
            // silently fails to match and invokeMethod() no-ops, hanging execModal() forever
            // waiting for a signal that never fires.
            const QVariantList order = dlg->property("buttonOrder").toList();
            QMetaObject::invokeMethod(dlg, "handleButtonClicked", Q_ARG(QVariant, order.at(1)));
        });
    });

    const DialogButton result = provider.choice(QStringLiteral("Title"), QStringLiteral("Text"),
        {DialogButton::Ok, DialogButton::Yes, DialogButton::No}, DialogButton::Cancel);

    QCOMPARE(result, DialogButton::Yes);
}

void TestQuickDialogProvider::testChoiceReturnsDefaultButtonWhenDismissedWithoutClick()
{
    QuickDialogProvider provider(m_window);
    connect(&provider, &QuickDialogProvider::dialogOpened, &provider, [](QObject *dialog) {
        deferDrive(dialog, [](QObject *dlg) { QMetaObject::invokeMethod(dlg, "reject"); });
    });

    const DialogButton result = provider.choice(QStringLiteral("Title"), QStringLiteral("Text"),
        {DialogButton::Ok}, DialogButton::Cancel);

    QCOMPARE(result, DialogButton::Cancel);
}

void TestQuickDialogProvider::testTextPromptReturnsEnteredValueOnAccept()
{
    QuickDialogProvider provider(m_window);
    connect(&provider, &QuickDialogProvider::dialogOpened, &provider, [](QObject *dialog) {
        deferDrive(dialog, [](QObject *dlg) {
            dlg->setProperty("value", QStringLiteral("new value"));
            QMetaObject::invokeMethod(dlg, "accept");
        });
    });

    const std::optional<QString> result = provider.textPrompt(
        QStringLiteral("Title"), QStringLiteral("Label"), QStringLiteral("initial"));

    QVERIFY(result.has_value());
    QCOMPARE(*result, QStringLiteral("new value"));
}

void TestQuickDialogProvider::testTextPromptReturnsNulloptOnCancel()
{
    QuickDialogProvider provider(m_window);
    connect(&provider, &QuickDialogProvider::dialogOpened, &provider, [](QObject *dialog) {
        deferDrive(dialog, [](QObject *dlg) { QMetaObject::invokeMethod(dlg, "reject"); });
    });

    const std::optional<QString> result = provider.textPrompt(
        QStringLiteral("Title"), QStringLiteral("Label"), QStringLiteral("initial"));

    QVERIFY(!result.has_value());
}

void TestQuickDialogProvider::testGetOpenFileNameReturnsSelectedPath()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    // Canonicalize the temp directory itself (always exists, so this never fails) before
    // building the file path from it -- on macOS, QTemporaryDir's path lives under /var, a
    // symlink to /private/var, and the real native file dialog resolves selectedFile through
    // that symlink on accept(); comparing against the un-resolved /var path would spuriously
    // fail there while passing everywhere else.
    const QString canonicalDir = QFileInfo(dir.path()).canonicalFilePath();
    const QString filePath = QDir(canonicalDir).filePath("pick_me.panda");
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.close();

    QuickDialogProvider provider(m_window);
    connect(&provider, &QuickDialogProvider::dialogOpened, &provider, [filePath](QObject *dialog) {
        // Must be set synchronously here, *before* execModal()'s open() call runs (not deferred
        // alongside accept()): QQuickFileDialog::accept() overwrites selectedFiles from the
        // real platform helper's own internal selection, discarding anything set on the outer
        // QML wrapper afterwards -- setting it here instead feeds QQuickFileDialog::onShow()'s
        // own "if (!m_options->initiallySelectedFiles().isEmpty()) fileDialog->selectFile(...)"
        // path, which seeds that real internal selection correctly.
        dialog->setProperty("selectedFile", QUrl::fromLocalFile(filePath));
        deferDrive(dialog, [](QObject *dlg) { QMetaObject::invokeMethod(dlg, "accept"); });
    });

    const QString result = provider.getOpenFileName(
        nullptr, QStringLiteral("Pick a file"), dir.path(), QStringLiteral("*"));

    QCOMPARE(result, filePath);
}

void TestQuickDialogProvider::testGetOpenFileNameReturnsEmptyOnCancel()
{
    QuickDialogProvider provider(m_window);
    connect(&provider, &QuickDialogProvider::dialogOpened, &provider, [](QObject *dialog) {
        deferDrive(dialog, [](QObject *dlg) { QMetaObject::invokeMethod(dlg, "reject"); });
    });

    const QString result = provider.getOpenFileName(
        nullptr, QStringLiteral("Pick a file"), QString(), QStringLiteral("*"));

    QVERIFY(result.isEmpty());
}

void TestQuickDialogProvider::testGetSaveFileNameReturnsPathAndFilter()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    // See testGetOpenFileNameReturnsSelectedPath()'s comment: canonicalize the temp directory
    // before building the compared path, for the same macOS /var-symlink reason.
    const QString canonicalDir = QFileInfo(dir.path()).canonicalFilePath();
    const QString filePath = QDir(canonicalDir).filePath("save_me.panda");

    QuickDialogProvider provider(m_window);
    connect(&provider, &QuickDialogProvider::dialogOpened, &provider, [filePath](QObject *dialog) {
        // See testGetOpenFileNameReturnsSelectedPath()'s comment: must be set synchronously
        // here, before open() runs, not deferred alongside accept().
        dialog->setProperty("selectedFile", QUrl::fromLocalFile(filePath));
        deferDrive(dialog, [](QObject *dlg) { QMetaObject::invokeMethod(dlg, "accept"); });
    });

    const FileDialogResult result = provider.getSaveFileName(
        nullptr, QStringLiteral("Save a file"), dir.path(), QStringLiteral("Panda files (*.panda)"));

    QCOMPARE(result.fileName, filePath);
    // Not the full "Panda files (*.panda)" string (unlike RealFileDialogProvider's Widgets-side
    // equivalent, which returns Qt's own untouched QFileDialog::getSaveFileName() filter
    // out-parameter) -- QQuickFileNameFilter::name is documented as just the display-name
    // portion; Qt's QQuickFileNameFilter::update() strips everything from '(' onward.
    QCOMPARE(result.selectedFilter, QStringLiteral("Panda files"));
}

void TestQuickDialogProvider::testGetSaveFileNameReturnsEmptyOnCancel()
{
    QuickDialogProvider provider(m_window);
    connect(&provider, &QuickDialogProvider::dialogOpened, &provider, [](QObject *dialog) {
        deferDrive(dialog, [](QObject *dlg) { QMetaObject::invokeMethod(dlg, "reject"); });
    });

    const FileDialogResult result = provider.getSaveFileName(
        nullptr, QStringLiteral("Save a file"), QString(), QStringLiteral("*"));

    QVERIFY(result.fileName.isEmpty());
}

void TestQuickDialogProvider::testGetSaveFileNameWithDirPathSetsCurrentFolder()
{
    // getSaveFileName()'s own return value never surfaces currentFolder, so this must be
    // observed via the dialogOpened hook directly (QuickDialogProvider.cpp sets it before
    // emitting dialogOpened, so it's already correct by the time this handler runs).
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QuickDialogProvider provider(m_window);
    QString observedFolder;
    connect(&provider, &QuickDialogProvider::dialogOpened, &provider,
        [&observedFolder](QObject *dialog) {
            observedFolder = dialog->property("currentFolder").toUrl().toLocalFile();
            deferDrive(dialog, [](QObject *dlg) { QMetaObject::invokeMethod(dlg, "reject"); });
        });

    provider.getSaveFileName(nullptr, QStringLiteral("Save a file"), dir.path(), QString());

    QCOMPARE(observedFolder, QFileInfo(dir.path()).absoluteFilePath());
}

void TestQuickDialogProvider::testGetSaveFileNameWithFullFilePathPresetsSelectedFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    // See testGetOpenFileNameReturnsSelectedPath()'s comment: canonicalize the temp directory
    // before building the compared path, for the same macOS /var-symlink reason.
    const QString canonicalDir = QFileInfo(dir.path()).canonicalFilePath();
    const QString suggestedPath = QDir(canonicalDir).filePath("suggested.panda"); // need not exist yet

    QuickDialogProvider provider(m_window);
    connect(&provider, &QuickDialogProvider::dialogOpened, &provider, [](QObject *dialog) {
        // Accept without touching selectedFile at all -- getSaveFileName() must have already
        // pre-populated it from the full suggested path before dialogOpened() fired.
        deferDrive(dialog, [](QObject *dlg) { QMetaObject::invokeMethod(dlg, "accept"); });
    });

    const FileDialogResult result = provider.getSaveFileName(
        nullptr, QStringLiteral("Save a file"), suggestedPath, QString());

    QCOMPARE(result.fileName, QFileInfo(suggestedPath).absoluteFilePath());
}
