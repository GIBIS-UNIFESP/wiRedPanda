// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickWorkSpace.h"

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QUndoStack>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/Settings.h"
#include "App/Element/ElementFactory.h"
#include "App/IO/Serialization.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/Versions.h"

namespace {

/// Write a minimal but parseable .panda file at \a path using the given \a version as the
/// format version in the header (no elements, no connections). Same fixture the old Widgets
/// test used, minus the Scene/Workspace.h dependency.
void writeMinimalOldFormatFile(const QString &path, const QVersionNumber &version)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_12);
    stream << Serialization::MAGIC_HEADER_CIRCUIT;
    stream << version;
    stream << QString(); // dolphin filename field
    stream << QRectF();  // scene rect field
    // No elements — empty but fully valid circuit
}

} // namespace

void TestQuickWorkSpace::testAutosaveTriggersOnCircuitChange()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    Settings::setAutosaveFiles({});

    QuickWorkSpace workspace;
    auto *undoStack = workspace.canvas()->undoStack();
    QVERIFY2(undoStack->isClean(), "Undo stack should start clean");

    QList<ItemWithId *> items{ElementFactory::buildElement(ElementType::And)};
    undoStack->push(new CanvasAddItemsCommand(items, workspace.canvas()));
    QVERIFY2(!undoStack->isClean(), "Undo stack should be dirty after a real add command");

    workspace.flushPendingAutosave();

    QVERIFY2(!Settings::autosaveFiles().isEmpty(), "Autosave should be triggered when the circuit changes");
}

void TestQuickWorkSpace::testAutosaveSkippedWhenClean()
{
    QuickWorkSpace workspace;
    auto *undoStack = workspace.canvas()->undoStack();
    QVERIFY2(undoStack->isClean(), "Undo stack should start clean");

    workspace.canvas()->addItem(ElementFactory::buildElement(ElementType::And));
    // Adding directly (not through a command) never touches the undo stack.
    QVERIFY2(undoStack->isClean(), "Undo stack should remain clean when items are added outside a command");
}

void TestQuickWorkSpace::testAutosaveFileNamingConvention()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    Settings::setAutosaveFiles({});

    QuickWorkSpace workspace;
    QList<ItemWithId *> items{ElementFactory::buildElement(ElementType::And)};
    workspace.canvas()->undoStack()->push(new CanvasAddItemsCommand(items, workspace.canvas()));
    workspace.flushPendingAutosave();

    const QStringList autosaves = Settings::autosaveFiles();
    QVERIFY2(!autosaves.isEmpty(), "Circuit change should produce an autosave entry");

    for (const QString &autosave : autosaves) {
        QVERIFY2(!autosave.isEmpty(), "Autosave entry should not be empty");
        QVERIFY2(autosave.endsWith(".panda"), qPrintable("Autosave should end with .panda: " + autosave));
        QVERIFY2(QFileInfo(autosave).fileName().startsWith('.'),
                 qPrintable("Autosave filename should be hidden (dot-prefixed): " + autosave));
        QVERIFY2(QFile::exists(autosave), qPrintable("Autosave file should exist: " + autosave));
        QVERIFY2(QFileInfo(autosave).size() > 0, "Autosave file should contain data");
    }
}

void TestQuickWorkSpace::testAutosaveFilesUniqueAcrossWorkspaces()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    Settings::setAutosaveFiles({});

    QString autosave1;
    {
        QuickWorkSpace ws1;
        QList<ItemWithId *> items{ElementFactory::buildElement(ElementType::And)};
        ws1.canvas()->undoStack()->push(new CanvasAddItemsCommand(items, ws1.canvas()));
        ws1.flushPendingAutosave();

        const auto autosaves = Settings::autosaveFiles();
        QVERIFY2(!autosaves.isEmpty(), "First workspace should produce an autosave entry");
        autosave1 = autosaves.first();
    }

    QString autosave2;
    {
        QuickWorkSpace ws2;
        QList<ItemWithId *> items{ElementFactory::buildElement(ElementType::Or)};
        ws2.canvas()->undoStack()->push(new CanvasAddItemsCommand(items, ws2.canvas()));
        ws2.flushPendingAutosave();

        const auto autosaves = Settings::autosaveFiles();
        QVERIFY2(!autosaves.isEmpty(), "Second workspace should produce an autosave entry");
        autosave2 = autosaves.size() >= 2 ? autosaves.last() : autosaves.first();
    }

    QVERIFY2(!autosave1.isEmpty() && !autosave2.isEmpty(), "Both autosave paths must be captured");
    QVERIFY2(autosave1 != autosave2, "Different workspaces must have different autosave files");
}

void TestQuickWorkSpace::testAutosaveDeletedOnExplicitSave()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    Settings::setAutosaveFiles({});

    QuickWorkSpace workspace;
    QList<ItemWithId *> items{ElementFactory::buildElement(ElementType::And)};
    workspace.canvas()->undoStack()->push(new CanvasAddItemsCommand(items, workspace.canvas()));
    workspace.flushPendingAutosave();

    const QString autosavePath = Settings::autosaveFiles().value(0);
    QVERIFY2(!autosavePath.isEmpty(), "Modification should produce an autosave entry");
    QVERIFY(QFile::exists(autosavePath));

    const QString saveFile = tempDir.filePath("explicit_save.panda");
    workspace.save(saveFile);

    QVERIFY2(!QFile::exists(autosavePath), "Autosave file should be removed once the user explicitly saves");
    QVERIFY2(!Settings::autosaveFiles().contains(autosavePath), "Autosave entry should be removed from Settings on explicit save");
}

void TestQuickWorkSpace::testAutosaveRemovedOnCleanDestruction()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    Settings::setAutosaveFiles({});

    QString autosavePath;
    {
        QuickWorkSpace workspace;
        QList<ItemWithId *> items{ElementFactory::buildElement(ElementType::And)};
        workspace.canvas()->undoStack()->push(new CanvasAddItemsCommand(items, workspace.canvas()));
        workspace.flushPendingAutosave();

        autosavePath = Settings::autosaveFiles().value(0);
        QVERIFY2(!autosavePath.isEmpty(), "Modification should produce an autosave entry");
        QVERIFY(QFile::exists(autosavePath));
    } // ~QuickWorkSpace: a clean shutdown discards the autosave (it's not a crash recovery case)

    QVERIFY2(!QFile::exists(autosavePath), "Autosave file should be removed on clean workspace destruction");
    QVERIFY2(!Settings::autosaveFiles().contains(autosavePath), "Autosave entry should be removed from Settings on clean destruction");
}

void TestQuickWorkSpace::testFlushPendingAutosaveRunsImmediately()
{
    // The 500ms autosave debounce delays writes; flushPendingAutosave() runs the deferred
    // handler synchronously.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    Settings::setAutosaveFiles({});

    QuickWorkSpace ws;
    QList<ItemWithId *> items{ElementFactory::buildElement(ElementType::And)};
    ws.canvas()->undoStack()->push(new CanvasAddItemsCommand(items, ws.canvas()));

    // The push only starts the debounce timer — Settings should not have updated yet.
    QCOMPARE(Settings::autosaveFiles().size(), 0);

    ws.flushPendingAutosave();

    QVERIFY(Settings::autosaveFiles().size() >= 1);
}

void TestQuickWorkSpace::testAutosaveTruncatesOnShrink()
{
    // QSaveFile commits via atomic rename and truncates as part of the rename, so a
    // shrunk circuit must produce a strictly smaller autosave file, not leave a stale tail.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    Settings::setAutosaveFiles({});

    QuickWorkSpace ws;

    QList<ItemWithId *> bigItems;
    for (int i = 0; i < 25; ++i) {
        bigItems.append(ElementFactory::buildElement(ElementType::Led));
    }
    ws.canvas()->undoStack()->push(new CanvasAddItemsCommand(bigItems, ws.canvas()));
    ws.flushPendingAutosave();

    const QString autosavePath = Settings::autosaveFiles().value(0);
    QVERIFY(!autosavePath.isEmpty());
    const qint64 sizeBig = QFileInfo(autosavePath).size();
    QVERIFY(sizeBig > 0);

    QList<ItemWithId *> toRemove;
    int kept = 0;
    for (auto *elm : ws.canvas()->elements()) {
        if (kept >= 2) {
            toRemove.append(elm);
        } else {
            ++kept;
        }
    }
    QVERIFY(!toRemove.isEmpty());
    ws.canvas()->undoStack()->push(new CanvasDeleteItemsCommand(toRemove, ws.canvas()));
    ws.flushPendingAutosave();

    const qint64 sizeSmall = QFileInfo(autosavePath).size();
    QVERIFY2(sizeSmall < sizeBig,
             qPrintable(QString("Shrunk autosave (%1 bytes) must be smaller than the prior write (%2 bytes)")
                            .arg(sizeSmall).arg(sizeBig)));
}

void TestQuickWorkSpace::testFirstSaveCopiesExternalAppearanceFile()
{
    // A custom LED appearance picked before the project has ever been saved lives somewhere
    // unrelated to the eventual project folder. The first real save must copy it alongside
    // the .panda file.
    QTemporaryDir sourceDir;
    QVERIFY(sourceDir.isValid());
    const QString imagePath = sourceDir.filePath("custom_led.svg");
    QVERIFY(QFile::copy(":/Components/Output/Led/WhiteLed.svg", imagePath));

    QTemporaryDir projectDir;
    QVERIFY(projectDir.isValid());
    const QString saveFile = projectDir.filePath("circuit.panda");

    QuickWorkSpace workspace;
    auto *ledElm = ElementFactory::buildElement(ElementType::Led);
    workspace.canvas()->addItem(ledElm);
    ledElm->setAppearance(false, imagePath);

    workspace.save(saveFile);

    const QString copiedImagePath = projectDir.filePath("custom_led.svg");
    QVERIFY2(QFile::exists(copiedImagePath),
             "Custom appearance file should be copied next to the project on its first save");
}

void TestQuickWorkSpace::testResaveCopiesNewlyAddedDependency()
{
    // Re-saving to the SAME location after a new external dependency was added since the
    // last save must also copy that new file.
    QTemporaryDir projectDir;
    QVERIFY(projectDir.isValid());
    const QString saveFile = projectDir.filePath("resave_project.panda");

    QuickWorkSpace workspace;
    workspace.canvas()->addItem(ElementFactory::buildElement(ElementType::And));
    workspace.save(saveFile);

    QTemporaryDir sourceDir;
    QVERIFY(sourceDir.isValid());
    const QString imagePath = sourceDir.filePath("custom_led.svg");
    QVERIFY(QFile::copy(":/Components/Output/Led/WhiteLed.svg", imagePath));

    auto *ledElm = ElementFactory::buildElement(ElementType::Led);
    workspace.canvas()->addItem(ledElm);
    ledElm->setAppearance(false, imagePath);

    workspace.save(saveFile);

    const QString copiedImagePath = projectDir.filePath("custom_led.svg");
    QVERIFY2(QFile::exists(copiedImagePath), "Newly added dependency should be copied on re-save to the same directory");
}

void TestQuickWorkSpace::testSaveToNewFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QuickWorkSpace workspace;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    andGate->setPos(10, 20);
    workspace.canvas()->addItem(andGate);

    const QString filePath = tempDir.filePath("test_circuit.panda");
    workspace.save(filePath);

    QFileInfo fileInfo(filePath);
    QVERIFY2(fileInfo.exists(), "Expected file was not created");
    QVERIFY2(fileInfo.size() > 0, "File should contain data");
    QCOMPARE(workspace.fileInfo().absoluteFilePath(), filePath);
}

void TestQuickWorkSpace::testSaveToExistingFileOverwrite()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QuickWorkSpace workspace;
    workspace.canvas()->addItem(ElementFactory::buildElement(ElementType::Or));

    const QString filePath = tempDir.filePath("overwrite_test.panda");
    workspace.save(filePath);
    QVERIFY2(QFileInfo(filePath).exists(), "File should exist after first save");

    workspace.canvas()->addItem(ElementFactory::buildElement(ElementType::Not));
    workspace.save(filePath);

    QFileInfo fileInfo(filePath);
    QVERIFY2(fileInfo.exists(), "File should still exist after overwrite");
    QVERIFY2(fileInfo.size() > 0, "File should contain data after overwrite");
}

void TestQuickWorkSpace::testSavePreservesElementData()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QuickWorkSpace workspace;

    auto *input = ElementFactory::buildElement(ElementType::InputSwitch);
    input->setPos(0, 0);
    input->setLabel("INPUT");
    workspace.canvas()->addItem(input);

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    andGate->setPos(50, 0);
    workspace.canvas()->addItem(andGate);

    auto *output = ElementFactory::buildElement(ElementType::Led);
    output->setPos(100, 0);
    output->setLabel("OUTPUT");
    workspace.canvas()->addItem(output);

    const QString filePath = tempDir.filePath("preserve_data_test.panda");
    workspace.save(filePath);

    QuickWorkSpace workspace2;
    workspace2.load(filePath);

    QCOMPARE(workspace2.canvas()->elements().size(), 3);

    bool foundInput = false;
    bool foundOutput = false;
    for (auto *elem : workspace2.canvas()->elements()) {
        if (elem->label() == "INPUT") {
            foundInput = true;
            QCOMPARE(elem->pos(), QPointF(0, 0));
        }
        if (elem->label() == "OUTPUT") {
            foundOutput = true;
            QCOMPARE(elem->pos(), QPointF(100, 0));
        }
    }
    QVERIFY2(foundInput, "Should find the labeled input element after reload");
    QVERIFY2(foundOutput, "Should find the labeled output element after reload");
}

void TestQuickWorkSpace::testSaveEmptyCircuit()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QuickWorkSpace workspace;
    const QString filePath = tempDir.filePath("empty.panda");
    workspace.save(filePath);

    QFileInfo fileInfo(filePath);
    QVERIFY2(fileInfo.exists(), "Empty-circuit save should still create a file");
    QVERIFY2(fileInfo.size() > 0, "Empty-circuit save should still contain a valid header");

    QuickWorkSpace reloaded;
    reloaded.load(filePath);
    QVERIFY2(reloaded.canvas()->elements().isEmpty(), "Reloaded empty circuit should have no elements");
}

void TestQuickWorkSpace::testLoadFromValidFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString filePath = tempDir.filePath("valid_circuit.panda");

    {
        QuickWorkSpace workspace;
        workspace.canvas()->addItem(ElementFactory::buildElement(ElementType::And));
        workspace.save(filePath);
    }

    QuickWorkSpace workspace;
    workspace.load(filePath);

    QCOMPARE(workspace.fileInfo().absoluteFilePath(), filePath);
    QVERIFY2(!workspace.canvas()->elements().isEmpty(), "Canvas should contain the loaded circuit's elements");
}

void TestQuickWorkSpace::testLoadNonExistentFileThrows()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QuickWorkSpace workspace;
    const QString nonExistentPath = tempDir.filePath("nonexistent_file_xyz.panda");
    QVERIFY2(!QFileInfo(nonExistentPath).exists(), "Fixture path must not already exist");

    bool exceptionThrown = false;
    try {
        workspace.load(nonExistentPath);
    } catch (const Pandaception &) {
        exceptionThrown = true;
    }
    QVERIFY2(exceptionThrown, "Loading a non-existent file should throw Pandaception");
    QVERIFY2(workspace.canvas()->elements().isEmpty(), "Canvas should remain empty after a failed load");
}

void TestQuickWorkSpace::testLoadCorruptedFileHandledGracefully()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString filePath = tempDir.filePath("corrupted.panda");
    {
        QFile file(filePath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("This is not a valid circuit file format");
    }

    QuickWorkSpace workspace;
    // Either a thrown Pandaception or graceful recovery is acceptable — the only real
    // requirement is that malformed input never crashes the process.
    try {
        workspace.load(filePath);
    } catch (const Pandaception &) {
    } catch (const std::exception &) {
    }
}

void TestQuickWorkSpace::testSaveToInvalidPathFails()
{
    QuickWorkSpace workspace;
    workspace.canvas()->addItem(ElementFactory::buildElement(ElementType::And));

    const QString beforePath = workspace.fileInfo().absoluteFilePath();

    const bool prevInteractive = Application::interactiveMode;
    Application::interactiveMode = true;

    const QString badPath = "/root/definitely/invalid/path/that/cannot/be/created/file.panda";
    bool threw = false;
    QuickWorkSpace::SaveOutcome outcome = QuickWorkSpace::SaveOutcome::Saved;
    try {
        outcome = workspace.save(badPath);
    } catch (const std::exception &) {
        threw = true;
    }

    Application::interactiveMode = prevInteractive;

    QVERIFY2(threw || outcome == QuickWorkSpace::SaveOutcome::ReadOnlyTarget,
             "Saving to an unwritable path must fail (throw or ReadOnlyTarget), never silently succeed");
    QCOMPARE(workspace.fileInfo().absoluteFilePath(), beforePath);
}

void TestQuickWorkSpace::testModifiedFlagAfterAddElement()
{
    // Adding directly via CanvasItem::addItem() never touches the undo stack — only going
    // through a real command (as the UI's "Add Element" actions do) marks the workspace
    // modified. Exercise that real path here.
    QuickWorkSpace workspace;
    auto *undoStack = workspace.canvas()->undoStack();
    QVERIFY2(undoStack->isClean(), "Undo stack should start clean");

    QList<ItemWithId *> items{ElementFactory::buildElement(ElementType::And)};
    workspace.canvas()->receiveCommand(new CanvasAddItemsCommand(items, workspace.canvas()));

    QCOMPARE(workspace.canvas()->elements().size(), 1);
    QVERIFY2(!undoStack->isClean(), "Undo stack should be dirty after adding an element via a real command");
}

void TestQuickWorkSpace::testModifiedFlagClearedAfterSave()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QuickWorkSpace workspace;
    QList<ItemWithId *> items{ElementFactory::buildElement(ElementType::And)};
    workspace.canvas()->receiveCommand(new CanvasAddItemsCommand(items, workspace.canvas()));
    QVERIFY(!workspace.canvas()->undoStack()->isClean());

    const QString filePath = tempDir.filePath("save_state_test.panda");
    workspace.save(filePath);

    QVERIFY2(workspace.canvas()->undoStack()->isClean(), "Undo stack should be clean after an explicit save");
}

void TestQuickWorkSpace::testFilePathUpdatedAfterSave()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QuickWorkSpace workspace;
    workspace.canvas()->addItem(ElementFactory::buildElement(ElementType::And));

    const QString filePath = tempDir.filePath("file_path_update_test.panda");
    workspace.save(filePath);

    QCOMPARE(workspace.fileInfo().absoluteFilePath(), filePath);
    QVERIFY2(QFileInfo(filePath).exists(), "File should exist at the specified path");
}

void TestQuickWorkSpace::testFilePathPreservedAfterLoad()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString filePath = tempDir.filePath("preserve_path_test.panda");

    {
        QuickWorkSpace workspace;
        workspace.canvas()->addItem(ElementFactory::buildElement(ElementType::And));
        workspace.save(filePath);
    }

    QuickWorkSpace workspace;
    workspace.load(filePath);

    QCOMPARE(workspace.fileInfo().absoluteFilePath(), filePath);
    QCOMPARE(workspace.fileInfo().fileName(), QString("preserve_path_test.panda"));
}

void TestQuickWorkSpace::testDolphinFileNameStorage()
{
    QuickWorkSpace workspace;
    const QString dolphinName = "test_simulation.dolphin";
    workspace.setDolphinFileName(dolphinName);
    QCOMPARE(workspace.dolphinFileName(), dolphinName);
}

void TestQuickWorkSpace::testLastIdGetterSetter()
{
    QuickWorkSpace workspace;
    const int initial = workspace.lastId();

    workspace.setLastId(initial + 100);
    QCOMPARE(workspace.lastId(), initial + 100);

    workspace.setLastId(initial + 150);
    QCOMPARE(workspace.lastId(), initial + 150);
}

void TestQuickWorkSpace::testLastIdPersistenceOnLoad()
{
    // After a real save/load round-trip, lastId() must be at least as high as the highest
    // element ID actually present in the loaded file, so a freshly-loaded circuit can never
    // hand out a new element ID that collides with an existing one.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString filePath = tempDir.filePath("last_id_persistence_test.panda");

    int maxSavedId = -1;
    {
        QuickWorkSpace workspace;
        for (int i = 0; i < 3; ++i) {
            auto *elm = ElementFactory::buildElement(ElementType::And);
            workspace.canvas()->addItem(elm);
            maxSavedId = (std::max)(maxSavedId, elm->id());
        }
        workspace.save(filePath);
    }

    QuickWorkSpace reloaded;
    reloaded.load(filePath);

    QVERIFY2(reloaded.lastId() >= maxSavedId, "lastId() after load must be at least the highest saved element ID");
}

void TestQuickWorkSpace::testFileInfoEmptyBeforeAnySaveOrLoad()
{
    QuickWorkSpace workspace;
    const QFileInfo fileInfo = workspace.fileInfo();

    QVERIFY2(!fileInfo.exists(), "A freshly created workspace should not report an existing file");
    QVERIFY2(!fileInfo.absoluteFilePath().isNull(), "absoluteFilePath() should never be a null string");
}

void TestQuickWorkSpace::testMigrationDisabledSkipsBackupAndResave()
{
    // With Application::migrationEnabled=false, loading an old-format file must:
    //   (a) not create any backup sidecar, and
    //   (b) not re-save (overwrite) the original file.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = tempDir.filePath("old_disabled.panda");

    writeMinimalOldFormatFile(path, Versions::V_4_2);

    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray originalBytes = f.readAll();
    f.close();

    QVERIFY2(!Application::migrationEnabled, "Migration must be disabled in tests");

    QuickWorkSpace workspace;
    workspace.load(path);

    const QString backupPath = tempDir.filePath("old_disabled.v4.2.panda");
    QVERIFY2(!QFile::exists(backupPath), "No backup must be created when migration is disabled");

    QFile f2(path);
    QVERIFY(f2.open(QIODevice::ReadOnly));
    QCOMPARE(f2.readAll(), originalBytes);
}

void TestQuickWorkSpace::testMigrationEnabledCreatesBackup()
{
    // With Application::migrationEnabled=true, loading an old-format file must create a
    // versioned backup named <basename>.v<old-version>.panda.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = tempDir.filePath("old_enabled.panda");

    writeMinimalOldFormatFile(path, Versions::V_4_2);

    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray originalBytes = f.readAll();
    f.close();

    Application::migrationEnabled = true;
    QuickWorkSpace workspace;
    try {
        workspace.load(path);
    } catch (const std::exception &e) {
        Application::migrationEnabled = false;
        QFAIL(qPrintable(QString("load() must not throw for a valid (empty) old-format file: %1").arg(e.what())));
    }
    Application::migrationEnabled = false;

    const QString backupPath = tempDir.filePath("old_enabled.v4.2.panda");
    QVERIFY2(QFile::exists(backupPath), qPrintable("Backup file not found: " + backupPath));

    QFile bf(backupPath);
    QVERIFY(bf.open(QIODevice::ReadOnly));
    QCOMPARE(bf.readAll(), originalBytes);
}

void TestQuickWorkSpace::testMigrationUpdatesFileVersion()
{
    // After migration, the original file must be re-saved in the current format.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = tempDir.filePath("old_resaved.panda");

    writeMinimalOldFormatFile(path, Versions::V_4_2);

    Application::migrationEnabled = true;
    QuickWorkSpace workspace;
    try {
        workspace.load(path);
    } catch (const std::exception &e) {
        Application::migrationEnabled = false;
        QFAIL(qPrintable(QString("load() must not throw for a valid (empty) old-format file: %1").arg(e.what())));
    }
    Application::migrationEnabled = false;

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QDataStream stream(&file);
    const QVersionNumber versionAfter = Serialization::readPandaHeader(stream);

    QCOMPARE(versionAfter, FormatRev::current);
}

void TestQuickWorkSpace::testMigrationCurrentVersionSkipsBackup()
{
    // Loading a current-format file with migration enabled must NOT create a backup.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = tempDir.filePath("current_format.panda");

    {
        QuickWorkSpace workspace;
        workspace.canvas()->addItem(ElementFactory::buildElement(ElementType::And));
        workspace.save(path);
    }

    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QDataStream stream(&file);
        QCOMPARE(Serialization::readPandaHeader(stream), FormatRev::current);
    }

    Application::migrationEnabled = true;
    QuickWorkSpace workspace2;
    try {
        workspace2.load(path);
    } catch (const std::exception &e) {
        Application::migrationEnabled = false;
        QFAIL(qPrintable(QString("load() must not throw for a current-format file: %1").arg(e.what())));
    }
    Application::migrationEnabled = false;

    const QStringList backups = QDir(tempDir.path()).entryList(QStringList{"current_format.v*.panda"}, QDir::Files);
    QVERIFY2(backups.isEmpty(), "No versioned backup should be created for a current-format file");
}
