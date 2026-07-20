// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestWorkspace.h"

#include <QDataStream>
#include <QFile>
#include <QFileDevice>
#include <QLayout>
#include <QResizeEvent>
#include <QTemporaryDir>

#include <QSignalSpy>

#include "App/Core/Application.h"
#include "App/Core/Settings.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Exercise/ExerciseEngine.h"
#include "App/Exercise/ExerciseOverlay.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MinimapWidget.h"
#include "App/Versions.h"
#include "Tests/Common/TestUtils.h"

namespace {
// Builds a hand-crafted load stream at QVersionNumber(4, 4) -- older than FormatRev::current
// and predating both hasUnifiedMetadata (V_4_6) and hasMetadata (V_4_5), so the payload is
// just a bare dolphinFileName + sceneRect with zero elements following. Also predates
// hasCompressedPayload (Rev100), so no qCompress() wrapping is needed.
QByteArray buildOldFormatLoadStream()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    stream << Serialization::MAGIC_HEADER_CIRCUIT;
    stream << quint32(2) << 4 << 4; // QVersionNumber(4, 4)
    stream << QString(); // dolphinFileName
    stream << QRectF();  // scene rect
    return data;
}

// Builds a hand-crafted load stream at QVersionNumber(999) -- newer than any real release, so
// past both hasUnifiedMetadata (V_4_6) and hasCompressedPayload (Rev100): the payload is a
// single compressed unit holding just the unified metadata map, zero elements following.
QByteArray buildNewerFormatLoadStream()
{
    QMap<QString, QVariant> metadata;
    metadata["dolphinFileName"] = QString();
    metadata["sceneRect"] = QRectF();

    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);
    payloadStream << metadata;

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    stream << Serialization::MAGIC_HEADER_CIRCUIT;
    stream << quint32(1) << 999; // QVersionNumber(999)
    Serialization::writePayload(stream, payload);
    return data;
}
} // namespace

void TestWorkspaceUnit::testWorkspaceCreation()
{
    WorkSpace workspace;
    QVERIFY(workspace.scene() != nullptr);
}

void TestWorkspaceUnit::testTabManagement()
{
    WorkSpace workspace;
    QVERIFY(workspace.scene() != nullptr);
    QVERIFY(workspace.view() != nullptr);
}

void TestWorkspaceUnit::testModificationTracking()
{
    WorkSpace workspace;
    // An empty workspace should not be from a newer version
    QVERIFY(!workspace.isFromNewerVersion());
}

void TestWorkspaceUnit::testICEditing()
{
    WorkSpace workspace;
    QVERIFY(!workspace.isInlineIC());
    QVERIFY(workspace.parentWorkspace() == nullptr);
}

void TestWorkspaceUnit::testFullScreenHandling()
{
    WorkSpace workspace;
    QVERIFY(workspace.scene() != nullptr);
    QVERIFY(workspace.view() != nullptr);
    QVERIFY(workspace.simulation() != nullptr);
}

void TestWorkspaceUnit::testMinimapDefaultPositionWithoutPersistedGeometry()
{
    Settings::setMinimapGeometry(QRect()); // ensure a clean slate regardless of test order

    WorkSpace workspace;
    workspace.resize(400, 300);
    workspace.layout()->activate();
    // resizeEvent() isn't reliably delivered to a never-shown top-level widget in a headless
    // test run (layout activation still sizes m_view correctly via the layout engine, just
    // not through the event path) -- call the geometry logic directly, same as the app's
    // own resizeEvent() does.
    workspace.applyMinimapGeometry();

    const QRect viewGeom = workspace.m_view.geometry();
    const int margin = 12;
    const int minimapW = workspace.m_minimap->width();
    const int minimapH = workspace.m_minimap->height();

    // No persisted geometry: falls back to the widget's own default size, bottom-right.
    QCOMPARE(workspace.m_minimap->pos(), QPoint(viewGeom.right() - minimapW - margin, viewGeom.bottom() - minimapH - margin));
}

void TestWorkspaceUnit::testMinimapRestoresPersistedGeometry()
{
    const QRect persisted(30, 40, 200, 150);
    Settings::setMinimapGeometry(persisted);

    WorkSpace workspace;
    workspace.resize(400, 300);
    workspace.layout()->activate();
    workspace.applyMinimapGeometry();

    QCOMPARE(workspace.m_minimap->geometry(), persisted);
}

void TestWorkspaceUnit::testMinimapReclampsOnSubsequentResize()
{
    Settings::setMinimapGeometry(QRect(30, 40, 200, 150));

    WorkSpace workspace;
    workspace.resize(400, 300);
    workspace.layout()->activate();
    workspace.applyMinimapGeometry();
    QCOMPARE(workspace.m_minimap->geometry(), QRect(30, 40, 200, 150));

    // Change the persisted value mid-session: a later resize must re-clamp the minimap's
    // own current geometry, not re-read (and jump to) this new Settings value -- Settings
    // is only ever written *from* a user-driven move/resize, never read again after the
    // first layout.
    Settings::setMinimapGeometry(QRect(0, 0, 50, 50));

    // Shrunk drastically -- comfortably above the minimap's own 160x120 minimum (so this
    // exercises re-clamping the position, not the Qt-enforced size floor), but well below
    // where (30,40,200,150) still fits.
    workspace.resize(200, 180);
    workspace.layout()->activate();
    workspace.applyMinimapGeometry();

    const QRect viewGeom = workspace.m_view.geometry();
    const int margin = 12;
    const QRect result = workspace.m_minimap->geometry();

    QVERIFY2(result != QRect(0, 0, 50, 50), "must not have jumped to the changed Settings value");
    QVERIFY2(result.x() >= margin && result.x() + result.width() <= viewGeom.width() - margin,
             "x must be re-clamped within the shrunk view");
    QVERIFY2(result.y() >= margin && result.y() + result.height() <= viewGeom.height() - margin,
             "y must be re-clamped within the shrunk view");
}

void TestWorkspaceUnit::testMinimapRestoreClampsOversizedGeometry()
{
    // Persisted from a much larger window/monitor than the one being restored into.
    Settings::setMinimapGeometry(QRect(10, 10, 900, 700));

    WorkSpace workspace;
    workspace.resize(300, 200);
    workspace.layout()->activate();
    workspace.applyMinimapGeometry();

    const QRect viewGeom = workspace.m_view.geometry();
    const int margin = 12;
    QVERIFY2(workspace.m_minimap->width() <= viewGeom.width() - 2 * margin,
             "restored size must be clamped to fit the current view, not just repositioned");
    QVERIFY2(workspace.m_minimap->height() <= viewGeom.height() - 2 * margin,
             "restored size must be clamped to fit the current view, not just repositioned");
    QVERIFY(workspace.m_minimap->x() >= margin);
    QVERIFY(workspace.m_minimap->y() >= margin);
}

void TestWorkspaceUnit::testMinimapIgnoresPreShowResizeThenRestoresOnShow()
{
    const QRect persisted(30, 40, 200, 150);
    Settings::setMinimapGeometry(persisted);

    WorkSpace workspace;
    // Resize while still hidden, exactly like MainWindow does via restoreGeometry() before
    // it's ever shown (which, for a window that was maximized at quit, applies the smaller
    // pre-maximize "normal" size first). Whatever resizeEvent(s) this produces must not
    // consume the one-time restore -- otherwise it locks in geometry clamped against a size
    // the window never actually settles at.
    workspace.resize(160, 120);
    QVERIFY(!workspace.m_minimapPositioned);

    // Resize to the real final size and show it. Since nothing resizes again after becoming
    // visible (unlike a maximize, which would trigger a further visible resizeEvent), the
    // showEvent() backstop timer is what performs the restore here.
    workspace.resize(400, 300);
    workspace.show();
    QTest::qWait(300);

    QVERIFY(workspace.m_minimapPositioned);
    QCOMPARE(workspace.m_minimap->geometry(), persisted);
}

void TestWorkspaceUnit::testResizeRepositionsVisibleExerciseOverlay()
{
    WorkSpace workspace;
    workspace.resize(500, 400);
    workspace.show();
    QVERIFY(QTest::qWaitForWindowExposed(&workspace));

    ExerciseEngine engine;
    auto *overlay = new ExerciseOverlay(&engine, &workspace);
    overlay->show();
    QVERIFY(overlay->isVisible());
    workspace.setExerciseOverlay(overlay);

    const QSize oldSize = workspace.size();
    workspace.resize(700, 550);
    QResizeEvent event(workspace.size(), oldSize);
    workspace.resizeEvent(&event);

    const int expectedX = (workspace.width() - overlay->width()) / 2;
    const int expectedY = workspace.height() - overlay->height() - 12;
    QCOMPARE(overlay->pos(), QPoint(expectedX, expectedY));
}

void TestWorkspaceUnit::testOnMinimapGeometryChangeFinishedPersistsSettings()
{
    WorkSpace workspace;
    const QRect geometry(11, 22, 333, 222);
    workspace.onMinimapGeometryChangeFinished(geometry);
    QCOMPARE(Settings::minimapGeometry(), geometry);
}

void TestWorkspaceUnit::testSaveWarnsAndNoOpsForNewerVersionFile()
{
    WorkSpace workspace;
    workspace.m_loadedVersion = QVersionNumber(999);

    const bool prevInteractive = Application::interactiveMode;
    Application::interactiveMode = true;
    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("out.panda");
    const auto outcome = workspace.save(path);

    Application::interactiveMode = prevInteractive;

    QCOMPARE(static_cast<int>(outcome), static_cast<int>(WorkSpace::SaveOutcome::Saved));
    QCOMPARE(dismisser.dismissCount(), 1);
    QVERIFY2(!QFile::exists(path), "A newer-version file must not actually be written to disk");
}

void TestWorkspaceUnit::testSaveCopiesAssociatedDolphinFile()
{
    QTemporaryDir oldDir;
    QTemporaryDir newDir;
    QVERIFY(oldDir.isValid() && newDir.isValid());

    QFile dolphinFile(oldDir.filePath("waveform.dolphin"));
    QVERIFY(dolphinFile.open(QIODevice::WriteOnly));
    dolphinFile.write("dummy");
    dolphinFile.close();

    WorkSpace workspace;
    workspace.scene()->setContextDir(oldDir.path());
    workspace.setDolphinFileName("waveform.dolphin");

    const auto outcome = workspace.save(newDir.filePath("out.panda"));
    QCOMPARE(static_cast<int>(outcome), static_cast<int>(WorkSpace::SaveOutcome::Saved));
    QVERIFY2(QFile::exists(newDir.filePath("waveform.dolphin")),
             "The associated dolphin waveform file must be copied alongside the .panda file");
}

void TestWorkspaceUnit::testSaveRemovesStaleAutosaveFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString autosavePath = dir.filePath("stale_autosave.panda");
    QFile autosaveFile(autosavePath);
    QVERIFY(autosaveFile.open(QIODevice::WriteOnly));
    autosaveFile.write("dummy");
    autosaveFile.close();

    WorkSpace workspace;
    workspace.m_autosaveFileName = autosavePath;
    Settings::setAutosaveFiles(QStringList{autosavePath});

    const auto outcome = workspace.save(dir.filePath("out.panda"));
    QCOMPARE(static_cast<int>(outcome), static_cast<int>(WorkSpace::SaveOutcome::Saved));
    QVERIFY2(!QFile::exists(autosavePath), "A successful save must remove the now-redundant autosave file");
    QVERIFY(!Settings::autosaveFiles().contains(autosavePath));
    QVERIFY(workspace.m_autosaveFileName.isEmpty());
}

void TestWorkspaceUnit::testLoadThrowsWhenFileCannotBeOpened()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot simulate an unreadable file on Windows (uses ACLs, not Unix permission bits)");
#else
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("unreadable.panda");
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("dummy");
    file.close();
    QVERIFY(QFile::setPermissions(path, QFileDevice::Permissions()));

    WorkSpace workspace;
    bool threw = false;
    try {
        workspace.load(path);
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(threw, "load() must throw when an existing file cannot be opened for reading");

    QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
#endif
}

void TestWorkspaceUnit::testLoadMigratesNonPandaSuffixedFileName()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("old_circuit"); // deliberately no .panda suffix

    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(buildOldFormatLoadStream());
    file.close();

    const bool prevMigration = Application::migrationEnabled;
    Application::migrationEnabled = true;

    WorkSpace workspace;
    workspace.load(path);

    Application::migrationEnabled = prevMigration;

    QVERIFY2(QFile::exists(path + ".panda"),
             "Migrating a non-.panda-suffixed file must append the suffix before re-saving");
}

void TestWorkspaceUnit::testLoadWarnsWhenMigrationTargetIsReadOnly()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot make a directory unwritable on Windows (uses ACLs, not Unix permission bits)");
#else
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath("old_circuit.panda");
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(buildOldFormatLoadStream());
    file.close();

    // Pre-create the versioned backup file load() would otherwise try to write during
    // migration, so createVersionedBackup()'s own QFile::exists() check short-circuits it as
    // a no-op -- otherwise it would throw when the directory below is locked, before ever
    // reaching the read-only re-save this test targets.
    QVERIFY(QFile::copy(path, dir.filePath("old_circuit.v4.4.panda")));

    // Lock the directory only after the fixtures are written -- mirrors the established
    // locked-dir technique used elsewhere in this suite (e.g. TestDolphinFile).
    QVERIFY(QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));

    const bool prevMigration = Application::migrationEnabled;
    const bool prevInteractive = Application::interactiveMode;
    Application::migrationEnabled = true;
    Application::interactiveMode = true;
    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();

    WorkSpace workspace;
    workspace.load(path);

    Application::migrationEnabled = prevMigration;
    Application::interactiveMode = prevInteractive;

    // One dialog for the older-format upgrade notice; the migration re-save then silently
    // finds the target read-only (logged via qCWarning, not a second dialog).
    QCOMPARE(dismisser.dismissCount(), 1);

    QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
#endif
}

void TestWorkspaceUnit::testLoadWarnsForNewerVersionFile()
{
    QByteArray data = buildNewerFormatLoadStream();
    QDataStream stream(&data, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    WorkSpace workspace;
    workspace.m_fileInfo = QFileInfo("dummy.panda");

    const bool prevInteractive = Application::interactiveMode;
    Application::interactiveMode = true;
    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();

    workspace.load(stream, version, QString());

    Application::interactiveMode = prevInteractive;
    QCOMPARE(dismisser.dismissCount(), 1);
}

void TestWorkspaceUnit::testLoadInfoForOlderVersionFile()
{
    QByteArray data = buildOldFormatLoadStream();
    QDataStream stream(&data, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    WorkSpace workspace;
    workspace.m_fileInfo = QFileInfo("dummy.panda");

    const bool prevInteractive = Application::interactiveMode;
    Application::interactiveMode = true;
    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();

    workspace.load(stream, version, QString());

    Application::interactiveMode = prevInteractive;
    QCOMPARE(dismisser.dismissCount(), 1);
}

void TestWorkspaceUnit::testAutosaveSkipsInlineICTabs()
{
    WorkSpace workspace;
    workspace.m_isInlineIC = true;
    workspace.m_autosaveFileName.clear();
    workspace.autosave();
    QVERIFY(workspace.m_autosaveFileName.isEmpty());
}

void TestWorkspaceUnit::testAutosaveSkipsNewerVersionFile()
{
    WorkSpace workspace;
    workspace.m_loadedVersion = QVersionNumber(999);
    workspace.m_autosaveFileName.clear();
    workspace.autosave();
    QVERIFY(workspace.m_autosaveFileName.isEmpty());
}

void TestWorkspaceUnit::testAutosaveRemovesFileWhenUndoStackIsClean()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString autosavePath = dir.filePath("stale.panda");
    QFile file(autosavePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("dummy");
    file.close();

    WorkSpace workspace;
    workspace.m_autosaveFileName = autosavePath;
    Settings::setAutosaveFiles(QStringList{autosavePath});
    QVERIFY(workspace.scene()->undoStack()->isClean());

    QSignalSpy spy(&workspace, &WorkSpace::fileChanged);
    workspace.autosave();

    QVERIFY2(!QFile::exists(autosavePath), "autosave() on a clean undo stack must remove the stale autosave file");
    QVERIFY(!Settings::autosaveFiles().contains(autosavePath));
    QVERIFY(workspace.m_autosaveFileName.isEmpty());
    QCOMPARE(spy.count(), 1);
}

void TestWorkspaceUnit::testAutosaveFallsBackToAppDataWhenProjectDirIsReadOnly()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot make a directory unwritable on Windows (uses ACLs, not Unix permission bits)");
#else
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));

    WorkSpace workspace;
    workspace.m_fileInfo = QFileInfo(dir.filePath("project.panda"));
    auto *gate = new And();
    workspace.scene()->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{gate}, workspace.scene()));
    QVERIFY(!workspace.scene()->undoStack()->isClean());

    workspace.autosave();

    QVERIFY2(!workspace.m_autosaveFileName.isEmpty(), "autosave() must still produce a file even when the project dir is read-only");
    QVERIFY2(!workspace.m_autosaveFileName.startsWith(dir.path()),
             "must fall back to the AppData autosaves dir, not the read-only project dir");
    QVERIFY(QFile::exists(workspace.m_autosaveFileName));

    QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    QFile::remove(workspace.m_autosaveFileName);
#endif
}

void TestWorkspaceUnit::testAutosaveRemovesPreviousFileWhenProjectDirChanges()
{
    QTemporaryDir oldDir;
    QTemporaryDir newDir;
    QVERIFY(oldDir.isValid() && newDir.isValid());

    const QString oldAutosavePath = oldDir.filePath(".old_autosave.panda");
    QFile oldFile(oldAutosavePath);
    QVERIFY(oldFile.open(QIODevice::WriteOnly));
    oldFile.write("dummy");
    oldFile.close();

    WorkSpace workspace;
    workspace.m_autosaveFileName = oldAutosavePath;
    workspace.m_fileInfo = QFileInfo(newDir.filePath("project.panda"));

    auto *gate = new And();
    workspace.scene()->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{gate}, workspace.scene()));

    workspace.autosave();

    QVERIFY2(!QFile::exists(oldAutosavePath),
             "Changing project directories must remove the stale autosave file in the old location");
    QVERIFY2(!workspace.m_autosaveFileName.isEmpty() && workspace.m_autosaveFileName != oldAutosavePath,
             "A new autosave file must be created in the new project directory");

    QFile::remove(workspace.m_autosaveFileName);
}

void TestWorkspaceUnit::testAutosaveThrowsWhenFileCannotBeOpened()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString autosavePath = dir.filePath("preset_autosave.panda");
    // A directory, not a file -- QSaveFile::open(WriteOnly) can't open it for writing.
    QVERIFY(QDir(dir.path()).mkdir("preset_autosave.panda"));

    WorkSpace workspace;
    workspace.m_fileInfo = QFileInfo(dir.filePath("project.panda"));
    workspace.m_autosaveFileName = autosavePath;

    auto *gate = new And();
    workspace.scene()->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{gate}, workspace.scene()));

    bool threw = false;
    try {
        workspace.autosave();
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(threw, "autosave() must throw when the autosave file cannot be opened");
}
