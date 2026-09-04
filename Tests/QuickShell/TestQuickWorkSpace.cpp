// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickWorkSpace.h"

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QRectF>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Core/ItemWithId.h"
#include "App/Core/Settings.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/DialogProvider.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "App/Versions.h"
#include "Tests/QuickShell/StubDialogProvider.h"

#if !defined(Q_OS_WIN)
#include <csignal>
#include <sys/resource.h>
#endif

namespace {

/// Write a minimal but parseable .panda file at \a path using the given \a version as the
/// format version in the header (no elements, no connections). Mirrors the Widgets-side
/// TestWorkspaceFileops.cpp's identical helper; the byte format is shared, portable Core code
/// (Serialization/VersionInfo), so it parses identically through QuickWorkSpace::load().
void writeMinimalOldFormatFile(const QString &path, const QVersionNumber &version)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_12);
    stream << Serialization::MAGIC_HEADER_CIRCUIT;
    stream << version;
    stream << QString();  // dolphin filename field
    stream << QRectF();   // scene rect field
    // No elements — empty but fully valid circuit
}

} // namespace

void TestQuickWorkSpace::initTestCase()
{
    QVERIFY2(m_tempDir.isValid(), "Member temporary directory creation failed");
}

void TestQuickWorkSpace::init()
{
    QVERIFY2(m_tempDir.isValid(), "Member temporary directory creation failed");
}

// ============================================================
// Autosave Triggering Tests
// ============================================================

void TestQuickWorkSpace::testAutosaveTriggersOnCircuitChange()
{
    Settings::setAutosaveFiles({});

    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();
    auto *undoStack = canvas->undoStack();

    QVERIFY2(undoStack->isClean(), "Undo stack should be in clean state");

    canvas->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::And)}, canvas));
    QVERIFY2(!undoStack->isClean(), "Undo stack should be dirty after circuit change");

    workspace.flushPendingAutosave();

    QVERIFY2(!Settings::autosaveFiles().isEmpty(), "Autosave should be triggered when circuit changes");
}

void TestQuickWorkSpace::testAutosaveSkippedWhenClean()
{
    Settings::setAutosaveFiles({});

    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();
    auto *undoStack = canvas->undoStack();

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    canvas->receiveCommand(new CanvasAddItemsCommand({andGate}, canvas));
    workspace.flushPendingAutosave();
    QVERIFY2(!Settings::autosaveFiles().isEmpty(), "Precondition: adding the element must have written an autosave entry");

    undoStack->undo();
    QVERIFY2(undoStack->isClean(), "Undo stack should be back in clean state after undo()");
    workspace.flushPendingAutosave();

    QVERIFY2(Settings::autosaveFiles().isEmpty(), "A clean undo stack must have its autosave entry removed, not skipped silently while stale");
}

void TestQuickWorkSpace::testAutosaveAfterElementModify()
{
    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();
    auto *undoStack = canvas->undoStack();

    auto *led = ElementFactory::buildElement(ElementType::Led);
    canvas->receiveCommand(new CanvasAddItemsCommand({led}, canvas));
    workspace.flushPendingAutosave();

    Settings::setAutosaveFiles({});
    canvas->receiveCommand(new CanvasRotateCommand({led}, 90, canvas));
    workspace.flushPendingAutosave();

    QVERIFY2(!undoStack->isClean(), "Undo stack should be dirty after circuit change");
    QVERIFY2(!Settings::autosaveFiles().isEmpty(), "Modifying an element should trigger autosave and update settings");
}

void TestQuickWorkSpace::testAutosaveAfterElementDelete()
{
    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();
    auto *undoStack = canvas->undoStack();

    auto *led = ElementFactory::buildElement(ElementType::Led);
    canvas->receiveCommand(new CanvasAddItemsCommand({led}, canvas));
    workspace.flushPendingAutosave();

    Settings::setAutosaveFiles({});
    canvas->receiveCommand(new CanvasDeleteItemsCommand({led}, canvas));
    workspace.flushPendingAutosave();

    QVERIFY2(!undoStack->isClean(), "Undo stack should be dirty after circuit change");
    QVERIFY2(!Settings::autosaveFiles().isEmpty(), "Deleting an element should trigger autosave and update settings");
}

void TestQuickWorkSpace::testAutosaveSignalEmitted()
{
    Settings::setAutosaveFiles({});

    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();

    QSignalSpy fileSpy(&workspace, &QuickWorkSpace::fileChanged);

    canvas->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::Led)}, canvas));

    const bool signalEmitted = fileSpy.wait(2000);

    QVERIFY2(signalEmitted || !fileSpy.isEmpty(),
             "fileChanged signal should be emitted when circuit is modified and autosave triggered");
}

// ============================================================
// Autosave File Naming Tests
// ============================================================

void TestQuickWorkSpace::testAutosaveFileCreatedInAppDataForNewProject()
{
    Settings::setAutosaveFiles({});

    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();

    canvas->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::And)}, canvas));
    workspace.flushPendingAutosave();

    const QStringList autosaves = Settings::autosaveFiles();
    QVERIFY2(!autosaves.isEmpty(), "Circuit change should produce an autosave entry");

    // Every autosave entry ends with .panda regardless of directory, so that half alone can't
    // distinguish a correct AppData path from a wrong one -- check the directory too, matching
    // QuickWorkSpace::autosave()'s own path construction for a never-saved project.
    const QString appDataAutosaveDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/autosaves";
    for (const QString &autosave : std::as_const(autosaves)) {
        QVERIFY2(!autosave.isEmpty(), "Autosave entry should not be empty");
        QVERIFY2(QFileInfo(autosave).absolutePath() == QFileInfo(appDataAutosaveDir).absoluteFilePath(),
                 qPrintable(QString("Autosave path should be under the AppData autosaves directory: %1").arg(autosave)));
    }
}

void TestQuickWorkSpace::testAutosaveInCurrentDirForSavedProject()
{
    // A saved project's autosave should land next to its .panda file, not in the AppData
    // autosaves directory (that's testAutosaveFileCreatedInAppDataForNewProject's case).
    Settings::setAutosaveFiles({});

    const QString testFile = m_tempDir.filePath("in_current_dir_circuit.panda");

    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();

    canvas->addItem(ElementFactory::buildElement(ElementType::Led));

    QCOMPARE(workspace.save(testFile), QuickWorkSpace::SaveOutcome::Saved);
    QVERIFY2(QFile::exists(testFile), "Save must have written the project file");

    canvas->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::And)}, canvas));
    workspace.flushPendingAutosave();

    const QStringList autosaves = Settings::autosaveFiles();
    QVERIFY2(!autosaves.isEmpty(), "A dirtied saved project should still produce an autosave entry");

    const QString &autosavePath = autosaves.first();
    QVERIFY2(QFile::exists(autosavePath), qPrintable(QStringLiteral("Autosave file should exist: %1").arg(autosavePath)));
    QCOMPARE(QFileInfo(autosavePath).absolutePath(), QFileInfo(testFile).absolutePath());
}

void TestQuickWorkSpace::testAutosaveFileNamingPatternIsLoadable()
{
    Settings::setAutosaveFiles({});

    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();

    canvas->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::And)}, canvas));
    workspace.flushPendingAutosave();

    const QStringList autosaves = Settings::autosaveFiles();
    QVERIFY2(!autosaves.isEmpty(), "Circuit change should produce an autosave entry");

    for (const QString &autosave : std::as_const(autosaves)) {
        QVERIFY2(!autosave.isEmpty(), "Autosave entry should not be empty");
        QVERIFY2(autosave.endsWith(".panda"), qPrintable(QString("Autosave should end with .panda: %1").arg(autosave)));

        const QFileInfo info(autosave);
        QVERIFY2(info.fileName().startsWith("."), qPrintable(QString("Autosave filename should start with dot: %1").arg(info.fileName())));
        QVERIFY2(info.size() > 16, qPrintable(QString("Autosave file must have content (%1 bytes): %2").arg(info.size()).arg(autosave)));

        // Round-trip: the written file must actually be loadable and contain the element.
        QuickWorkSpace reloaded;
        try {
            reloaded.load(autosave);
        } catch (const Pandaception &e) {
            QFAIL(qPrintable(QString("Autosave file should be loadable: %1\nError: %2").arg(autosave, e.what())));
        }
        QVERIFY2(!reloaded.canvas()->elements().isEmpty(), "Loaded autosave should contain elements");
    }
}

void TestQuickWorkSpace::testAutosavePathCreatedIfNotExists()
{
    Settings::setAutosaveFiles({});

    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();

    canvas->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::Led)}, canvas));
    workspace.flushPendingAutosave();

    const QStringList autosaves = Settings::autosaveFiles();
    QVERIFY2(!autosaves.isEmpty(), "Modification should trigger autosave");

    const QString &autosaveFile = autosaves.first();
    QVERIFY2(QFile::exists(autosaveFile), qPrintable(QString("Autosave file must exist: %1").arg(autosaveFile)));

    const QFileInfo info(autosaveFile);
    QVERIFY2(info.size() > 100, qPrintable(QString("Autosave should have content (%1 bytes)").arg(info.size())));
}

void TestQuickWorkSpace::testAutosaveFilePermissions()
{
    Settings::setAutosaveFiles({});

    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();

    canvas->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::Led)}, canvas));
    workspace.flushPendingAutosave();

    const QStringList autosaves = Settings::autosaveFiles();
    QVERIFY2(!autosaves.isEmpty(), "Workspace should create autosave file");

    const QFileInfo fileInfo(autosaves.first());
    QVERIFY2(fileInfo.isReadable(), "Autosave file should be readable");
    QVERIFY2(fileInfo.isWritable(), "Autosave file should be writable");
}

void TestQuickWorkSpace::testMultipleWorkspacesProduceDistinctAutosaves()
{
    // Two independently-dirtied workspaces must never collide on the same autosave filename --
    // the random-suffix generation in QuickWorkSpace::autosave() is what this actually verifies.
    Settings::setAutosaveFiles({});

    QString autosave1;
    {
        QuickWorkSpace ws1;
        CanvasItem *canvas = ws1.canvas();
        canvas->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::Led)}, canvas));
        ws1.flushPendingAutosave();

        const QStringList autosaves = Settings::autosaveFiles();
        QVERIFY2(!autosaves.isEmpty(), "First workspace should produce an autosave entry");
        autosave1 = autosaves.first();
        QVERIFY2(QFile::exists(autosave1), qPrintable(QString("First autosave file should exist: %1").arg(autosave1)));
    }

    QString autosave2;
    {
        QuickWorkSpace ws2;
        CanvasItem *canvas = ws2.canvas();
        canvas->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::And)}, canvas));
        ws2.flushPendingAutosave();

        const QStringList autosaves = Settings::autosaveFiles();
        QVERIFY2(!autosaves.isEmpty(), "Second workspace should produce an autosave entry");
        autosave2 = autosaves.size() >= 2 ? autosaves.last() : autosaves.first();
    }

    QVERIFY2(!autosave1.isEmpty() && !autosave2.isEmpty(), "Both autosave paths must be captured");
    QVERIFY2(autosave1 != autosave2,
             qPrintable(QString("Different workspaces must have different autosave files.\n  File 1: %1\n  File 2: %2").arg(autosave1, autosave2)));
}

// ============================================================
// Autosave Cleanup Tests
// ============================================================

void TestQuickWorkSpace::testAutosaveDeletedOnExplicitSave()
{
    // A real save must remove this workspace's own previously-tracked autosave entry (both the
    // Settings entry and the leftover file on disk).
    Settings::setAutosaveFiles({});

    const QString saveFile = m_tempDir.filePath("explicit_save_test.panda");

    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();

    canvas->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::Led)}, canvas));
    workspace.flushPendingAutosave();

    const QStringList autosavesBefore = Settings::autosaveFiles();
    QVERIFY2(!autosavesBefore.isEmpty(), "Precondition: dirtying the project must have written an autosave entry");
    const QString trackedAutosave = autosavesBefore.first();

    QCOMPARE(workspace.save(saveFile), QuickWorkSpace::SaveOutcome::Saved);
    QVERIFY2(canvas->undoStack()->isClean(), "Undo stack should be clean after save");

    const QStringList autosavesAfter = Settings::autosaveFiles();
    QVERIFY2(!autosavesAfter.contains(trackedAutosave), "The previously-tracked autosave entry must be removed after an explicit save");
    QVERIFY2(!QFile::exists(trackedAutosave), "A real save must delete the leftover autosave file, not just its Settings entry");
}

void TestQuickWorkSpace::testFlushPendingAutosaveRunsImmediatelyB3()
{
    // The 500ms autosave debounce delays writes; flushPendingAutosave() runs the deferred
    // handler synchronously -- needed so the destructor and explicit Save flows don't drop a
    // pending autosave on the floor.
    Settings::setAutosaveFiles({});

    QuickWorkSpace ws;
    CanvasItem *canvas = ws.canvas();
    canvas->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::Led)}, canvas));

    // The push fires QUndoStack::indexChanged immediately, which only schedules the debounce
    // timer -- Settings should not have updated yet.
    QCOMPARE(Settings::autosaveFiles().size(), 0);

    ws.flushPendingAutosave();

    QVERIFY(!Settings::autosaveFiles().isEmpty());
}

void TestQuickWorkSpace::testAutosaveTruncatesOnShrinkB2()
{
    // Regression: a QSaveFile commit truncates as part of its atomic rename, so a circuit that
    // shrinks between autosave writes must produce a strictly smaller file, not leave a
    // previous run's trailing bytes intact.
    Settings::setAutosaveFiles({});

    QuickWorkSpace ws;
    CanvasItem *canvas = ws.canvas();

    QList<ItemWithId *> bigItems;
    for (int i = 0; i < 25; ++i) {
        bigItems.append(ElementFactory::buildElement(ElementType::Led));
    }
    canvas->receiveCommand(new CanvasAddItemsCommand(bigItems, canvas));
    ws.flushPendingAutosave();

    const QStringList autosavesBig = Settings::autosaveFiles();
    QVERIFY(!autosavesBig.isEmpty());
    const QString &autosavePath = autosavesBig.first();
    const qint64 sizeBig = QFileInfo(autosavePath).size();
    QVERIFY(sizeBig > 0);

    QList<ItemWithId *> toRemove;
    int kept = 0;
    for (auto *elm : canvas->elements()) {
        if (kept >= 2) {
            toRemove.append(elm);
        } else {
            ++kept;
        }
    }
    QVERIFY(!toRemove.isEmpty());
    canvas->receiveCommand(new CanvasDeleteItemsCommand(toRemove, canvas));
    ws.flushPendingAutosave();

    const qint64 sizeSmall = QFileInfo(autosavePath).size();
    QVERIFY2(sizeSmall < sizeBig,
             qPrintable(QString("Shrunk autosave (%1 bytes) must be smaller than the prior write (%2 bytes).").arg(sizeSmall).arg(sizeBig)));
}

// ============================================================
// Save Operations Tests
// ============================================================

void TestQuickWorkSpace::testSaveToNewFile()
{
    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    andGate->setPos(10, 20);
    canvas->addItem(andGate);
    canvas->undoStack()->setClean();

    const QString filePath = m_tempDir.path() + "/test_circuit.panda";

    try {
        QCOMPARE(workspace.save(filePath), QuickWorkSpace::SaveOutcome::Saved);

        const QFileInfo fileInfo(filePath);
        QVERIFY2(fileInfo.exists(), "Expected file was not created");
        QVERIFY2(fileInfo.size() > 0, "File should contain data");
        QCOMPARE(workspace.fileInfo().absoluteFilePath(), filePath);
    } catch (const std::exception &e) {
        QFAIL(QString("Save failed with exception: %1").arg(e.what()).toUtf8().constData());
    }
}

void TestQuickWorkSpace::testSaveToExistingFileOverwrites()
{
    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();

    canvas->addItem(ElementFactory::buildElement(ElementType::Or));
    canvas->undoStack()->setClean();

    const QString filePath = m_tempDir.path() + "/overwrite_test.panda";

    try {
        workspace.save(filePath);
        QVERIFY2(QFileInfo(filePath).exists(), "File should exist at the specified path");
        const qint64 sizeBeforeExtraElement = QFileInfo(filePath).size();

        canvas->addItem(ElementFactory::buildElement(ElementType::Not));
        canvas->undoStack()->setClean();

        workspace.save(filePath);

        const QFileInfo fileInfo(filePath);
        QVERIFY2(fileInfo.exists(), "Expected file was not created");
        QVERIFY2(fileInfo.size() > 0, "File should contain data");
        QVERIFY2(fileInfo.size() != sizeBeforeExtraElement, "Overwriting with an added element should change the saved file's size");
    } catch (const std::exception &e) {
        QFAIL(QString("Save failed with exception: %1").arg(e.what()).toUtf8().constData());
    }
}

void TestQuickWorkSpace::testSaveWithSpecialCharactersInFilename()
{
    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();

    canvas->addItem(ElementFactory::buildElement(ElementType::And));
    canvas->undoStack()->setClean();

    const QString filePath = m_tempDir.path() + "/test_circuit_2025.panda";

    try {
        workspace.save(filePath);

        const QFileInfo fileInfo(filePath);
        QVERIFY2(fileInfo.exists(), "Expected file was not created");
        QCOMPARE(fileInfo.fileName(), QString("test_circuit_2025.panda"));
    } catch (const std::exception &e) {
        QFAIL(QString("Save with special characters failed: %1").arg(e.what()).toUtf8().constData());
    }
}

void TestQuickWorkSpace::testSavePreservesElementData()
{
    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();

    auto *input = ElementFactory::buildElement(ElementType::InputSwitch);
    input->setPos(0, 0);
    input->setLabel("INPUT");
    canvas->addItem(input);

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    andGate->setPos(50, 0);
    canvas->addItem(andGate);

    auto *output = ElementFactory::buildElement(ElementType::Led);
    output->setPos(100, 0);
    output->setLabel("OUTPUT");
    canvas->addItem(output);

    canvas->undoStack()->setClean();

    const QString filePath = m_tempDir.path() + "/preserve_data_test.panda";

    try {
        workspace.save(filePath);

        QuickWorkSpace workspace2;
        workspace2.load(filePath);
        CanvasItem *canvas2 = workspace2.canvas();

        QCOMPARE(canvas2->elements().size(), 3);

        bool foundInput = false;
        bool foundOutput = false;
        for (auto *elem : canvas2->elements()) {
            if (elem->label() == "INPUT") {
                foundInput = true;
                QCOMPARE(elem->pos(), QPointF(0, 0));
            }
            if (elem->label() == "OUTPUT") {
                foundOutput = true;
                QCOMPARE(elem->pos(), QPointF(100, 0));
            }
        }

        QVERIFY2(foundInput, "Should find expected input element in loaded circuit");
        QVERIFY2(foundOutput, "Should find expected output element in loaded circuit");
    } catch (const std::exception &e) {
        QFAIL(QString("Preserve data test failed: %1").arg(e.what()).toUtf8().constData());
    }
}

void TestQuickWorkSpace::testSaveEmptyCircuit()
{
    const QString saveFile = m_tempDir.filePath("empty.panda");

    QuickWorkSpace workspace;

    try {
        workspace.save(saveFile);

        const QFileInfo fileInfo(saveFile);
        QVERIFY2(fileInfo.exists(), "Expected file should exist");
        QVERIFY2(fileInfo.size() > 0, "Saved file should contain data");
    } catch (const Pandaception &e) {
        QFAIL(qPrintable(QString("Failed to save empty circuit: %1").arg(e.what())));
    }
}

// ============================================================
// External-file dependency copying on save
// ============================================================

void TestQuickWorkSpace::testSaveCopiesExternalAppearanceFileOnFirstSave()
{
    // A brand-new (never-saved) workspace's first save must copy a custom appearance file from
    // an unrelated directory into the project directory.
    QTemporaryDir sourceDir;
    QVERIFY(sourceDir.isValid());
    const QString sourceImagePath = sourceDir.filePath("custom_led.svg");
    QVERIFY(QFile::copy(":/Components/Input/switchOff.svg", sourceImagePath));

    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();
    auto *led = ElementFactory::buildElement(ElementType::Led);
    canvas->addItem(led);
    led->setAppearance(false, sourceImagePath);
    canvas->undoStack()->setClean();

    const QString projectFilePath = m_tempDir.path() + "/new_project_with_appearance.panda";
    try {
        workspace.save(projectFilePath);
    } catch (const std::exception &e) {
        QFAIL(QString("Save failed with exception: %1").arg(e.what()).toUtf8().constData());
    }

    const QString expectedCopiedPath = QFileInfo(projectFilePath).absolutePath() + "/custom_led.svg";
    QVERIFY2(QFileInfo(expectedCopiedPath).exists(), "Custom appearance file should be copied into the project directory on first save");
}

void TestQuickWorkSpace::testSaveCopiesNewlyAddedDependencyOnResave()
{
    // An already-saved workspace, saved again to the SAME location after a new external
    // dependency was added since the last save, must also copy that new file.
    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();
    canvas->addItem(ElementFactory::buildElement(ElementType::And));
    canvas->undoStack()->setClean();

    const QString projectFilePath = m_tempDir.path() + "/resave_project.panda";
    try {
        workspace.save(projectFilePath);
    } catch (const std::exception &e) {
        QFAIL(QString("Initial save failed with exception: %1").arg(e.what()).toUtf8().constData());
    }

    QTemporaryDir sourceDir;
    QVERIFY(sourceDir.isValid());
    const QString sourceImagePath = sourceDir.filePath("custom_led.svg");
    QVERIFY(QFile::copy(":/Components/Input/switchOff.svg", sourceImagePath));

    auto *led = ElementFactory::buildElement(ElementType::Led);
    canvas->addItem(led);
    led->setAppearance(false, sourceImagePath);
    canvas->undoStack()->setClean();

    try {
        workspace.save(projectFilePath);
    } catch (const std::exception &e) {
        QFAIL(QString("Re-save failed with exception: %1").arg(e.what()).toUtf8().constData());
    }

    const QString expectedCopiedPath = QFileInfo(projectFilePath).absolutePath() + "/custom_led.svg";
    QVERIFY2(QFileInfo(expectedCopiedPath).exists(), "Newly added dependency should be copied on re-save to the same directory");
}

// ============================================================
// Load Operations Tests
// ============================================================

void TestQuickWorkSpace::testLoadNonExistentFileThrowsAndLeavesProjectUnchanged()
{
    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();
    auto *existingGate = ElementFactory::buildElement(ElementType::And);
    canvas->addItem(existingGate);

    const QString nonExistentPath = m_tempDir.path() + "/nonexistent_file_xyz.panda";
    QVERIFY2(!QFileInfo(nonExistentPath).exists(), "Non-existent file should not exist");

    bool exceptionThrown = false;
    try {
        workspace.load(nonExistentPath);
    } catch (const Pandaception &) {
        exceptionThrown = true;
    } catch (const std::exception &) {
        exceptionThrown = true;
    }

    QVERIFY2(exceptionThrown, "Exception should be thrown when loading non-existent file");

    // A failed load of a non-existent file must not touch the already-open project.
    QCOMPARE(canvas->elements().size(), 1);
    QCOMPARE(canvas->elements().constFirst(), existingGate);
}

void TestQuickWorkSpace::testLoadCorruptedFileHandling()
{
    const QString filePath = m_tempDir.path() + "/corrupted.panda";
    {
        QFile file(filePath);
        QVERIFY2(file.open(QIODevice::WriteOnly), qPrintable(QString("Failed to open %1: %2").arg(file.fileName(), file.errorString())));
        file.write("This is not a valid circuit file format");
        file.close();
    }

    QuickWorkSpace workspace;

    try {
        workspace.load(filePath);
        QVERIFY2(workspace.canvas()->elements().isEmpty(), "A load that survives corrupted data must not fabricate elements from it");
    } catch (const std::exception &e) {
        QVERIFY2(!QString(e.what()).isEmpty(), "Exception should explain why the file is corrupted");
        QVERIFY2(workspace.canvas()->elements().isEmpty(), "A failed load must not leave a partially-populated scene");
    }
}

void TestQuickWorkSpace::testLoadEmptyCircuit()
{
    const QString filePath = m_tempDir.path() + "/empty_circuit.panda";
    {
        QuickWorkSpace workspace;
        workspace.canvas()->undoStack()->setClean();
        workspace.save(filePath);
    }

    QuickWorkSpace workspace;
    try {
        workspace.load(filePath);
        QCOMPARE(workspace.fileInfo().absoluteFilePath(), filePath);
        QVERIFY2(workspace.canvas()->elements().isEmpty(), "Scene elements should be empty after reload");
    } catch (const std::exception &e) {
        QFAIL(QString("Load empty circuit failed: %1").arg(e.what()).toUtf8().constData());
    }
}

void TestQuickWorkSpace::testSaveToInvalidPathThrows()
{
    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();
    canvas->addItem(ElementFactory::buildElement(ElementType::Led));

    const QString invalidPath = "/root/definitely/invalid/path/that/cannot/be/created/file.panda";

    bool exceptionThrown = false;
    try {
        workspace.save(invalidPath);
        QFAIL("save() should either throw an exception or fail gracefully for invalid path");
    } catch (const Pandaception &) {
        exceptionThrown = true;
    }

    QVERIFY2(exceptionThrown, "Saving to invalid path should throw Pandaception");
}

// ============================================================
// File State Management Tests
// ============================================================

void TestQuickWorkSpace::testModifiedFlagAfterAddElement()
{
    // CanvasItem::addItem() (used elsewhere in this file to set up fixtures before manually
    // calling setClean()) never touches the undo stack -- only the real command path
    // (CanvasAddItemsCommand) marks the workspace modified.
    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();

    QVERIFY2(canvas->undoStack()->isClean(), "Undo stack should be in clean state initially");

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    canvas->receiveCommand(new CanvasAddItemsCommand({andGate}, canvas));

    QCOMPARE(canvas->elements().size(), 1);
    QVERIFY2(!canvas->undoStack()->isClean(), "Undo stack should be dirty after adding an element via command");
}

void TestQuickWorkSpace::testModifiedFlagClearedAfterSave()
{
    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();

    canvas->addItem(ElementFactory::buildElement(ElementType::And));
    canvas->undoStack()->setClean();

    const QString filePath = m_tempDir.path() + "/save_state_test.panda";

    try {
        workspace.save(filePath);
        QVERIFY2(canvas->undoStack()->isClean(), "Undo stack should be in clean state after save");
    } catch (const std::exception &e) {
        QFAIL(QString("Save state test failed: %1").arg(e.what()).toUtf8().constData());
    }
}

void TestQuickWorkSpace::testFilePathUpdatedAfterSave()
{
    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();

    canvas->addItem(ElementFactory::buildElement(ElementType::And));
    canvas->undoStack()->setClean();

    const QString filePath = m_tempDir.path() + "/file_path_update_test.panda";

    try {
        workspace.save(filePath);

        QCOMPARE(workspace.fileInfo().absoluteFilePath(), filePath);
        QVERIFY2(QFileInfo(filePath).exists(), "File should exist at the specified path");
    } catch (const std::exception &e) {
        QFAIL(QString("File path update test failed: %1").arg(e.what()).toUtf8().constData());
    }
}

void TestQuickWorkSpace::testFilePathPreservedAfterLoad()
{
    const QString filePath = m_tempDir.path() + "/preserve_path_test.panda";
    {
        QuickWorkSpace workspace;
        workspace.canvas()->addItem(ElementFactory::buildElement(ElementType::And));
        workspace.canvas()->undoStack()->setClean();
        workspace.save(filePath);
    }

    QuickWorkSpace workspace;
    try {
        workspace.load(filePath);

        QCOMPARE(workspace.fileInfo().absoluteFilePath(), filePath);
        QCOMPARE(workspace.fileInfo().fileName(), QString("preserve_path_test.panda"));
    } catch (const std::exception &e) {
        QFAIL(QString("File path preservation test failed: %1").arg(e.what()).toUtf8().constData());
    }
}

void TestQuickWorkSpace::testSaveFailureLeavesFileIdentityUnchanged()
{
    // A save that never actually writes anything (bad target path) must leave the workspace's
    // reported identity exactly as it was before.
    QuickWorkSpace workspace;
    CanvasItem *canvas = workspace.canvas();
    canvas->addItem(ElementFactory::buildElement(ElementType::And));

    const QString beforePath = workspace.fileInfo().absoluteFilePath();
    const QString beforeContextDir = canvas->contextDir();

    const bool prevInteractive = Application::interactiveMode;
    Application::interactiveMode = true;

    const QString badPath = m_tempDir.path() + "/no_such_directory/project.panda";
    bool threw = false;
    QuickWorkSpace::SaveOutcome outcome = QuickWorkSpace::SaveOutcome::Saved;
    try {
        outcome = workspace.save(badPath);
    } catch (const std::exception &) {
        threw = true;
    }

    Application::interactiveMode = prevInteractive;

    QVERIFY2(threw || outcome == QuickWorkSpace::SaveOutcome::ReadOnlyTarget,
             "Expected save() to a nonexistent directory to fail (throw or ReadOnlyTarget)");
    QCOMPARE(workspace.fileInfo().absoluteFilePath(), beforePath);
    QCOMPARE(canvas->contextDir(), beforeContextDir);
}

// ============================================================
// Metadata / lastId
// ============================================================

void TestQuickWorkSpace::testDolphinFileNameStorage()
{
    QuickWorkSpace workspace;

    const QString dolphinName = "test_simulation.dolphin";
    workspace.setDolphinFileName(dolphinName);

    QCOMPARE(workspace.dolphinFileName(), dolphinName);
}

void TestQuickWorkSpace::testLastIdInitializationValue()
{
    QuickWorkSpace workspace;
    QVERIFY2(workspace.lastId() >= 0, "Item ID should be non-negative");
}

void TestQuickWorkSpace::testLastIdGetterSetter()
{
    QuickWorkSpace workspace;

    const int initialLastId = workspace.lastId();
    const int newLastId = initialLastId + 100;

    workspace.setLastId(newLastId);
    QCOMPARE(workspace.lastId(), newLastId);

    workspace.setLastId(newLastId + 50);
    QCOMPARE(workspace.lastId(), newLastId + 50);
}

void TestQuickWorkSpace::testLastIdPersistenceOnLoad()
{
    // After a real save/load round-trip, lastId() must be at least as high as the highest
    // element ID actually present in the loaded file.
    const QString filePath = m_tempDir.path() + "/last_id_persistence_test.panda";
    int maxSavedId = -1;
    {
        QuickWorkSpace workspace;
        CanvasItem *canvas = workspace.canvas();
        for (int i = 0; i < 3; ++i) {
            auto *elm = ElementFactory::buildElement(ElementType::And);
            canvas->addItem(elm);
            maxSavedId = (std::max)(maxSavedId, elm->id());
        }
        canvas->undoStack()->setClean();
        workspace.save(filePath);
    }

    QuickWorkSpace reloaded;
    reloaded.load(filePath);

    QVERIFY2(reloaded.lastId() >= maxSavedId, "lastId() after load must be at least the highest saved element ID");
}

void TestQuickWorkSpace::testFileInfoAfterCreation()
{
    QuickWorkSpace workspace;

    const QFileInfo fileInfo = workspace.fileInfo();

    QVERIFY2(!fileInfo.exists(), "A freshly-created workspace's fileInfo must not point at an existing file");

    const QString absolutePath = fileInfo.absoluteFilePath();
    QVERIFY2(!absolutePath.isNull(), "Absolute path should not be null");
}

// ============================================================
// Auto-migration: QuickWorkSpace::load
// ============================================================

void TestQuickWorkSpace::testMigrationDisabledSkipsBackupAndResave()
{
    QVERIFY2(m_tempDir.isValid(), "Temp dir must be valid");
    const QString path = m_tempDir.path() + "/old_disabled.panda";

    writeMinimalOldFormatFile(path, Versions::V_4_2);

    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray originalBytes = f.readAll();
    f.close();

    QVERIFY2(!Application::migrationEnabled, "Migration must be disabled in tests");

    QuickWorkSpace ws;
    ws.load(path);

    const QString backupPath = m_tempDir.path() + "/old_disabled.v4.2.panda";
    QVERIFY2(!QFile::exists(backupPath), "No backup must be created when migration is disabled");

    QFile f2(path);
    QVERIFY(f2.open(QIODevice::ReadOnly));
    QCOMPARE(f2.readAll(), originalBytes);
}

void TestQuickWorkSpace::testMigrationEnabledCreatesBackup()
{
    QVERIFY2(m_tempDir.isValid(), "Temp dir must be valid");
    const QString path = m_tempDir.path() + "/old_enabled.panda";

    writeMinimalOldFormatFile(path, Versions::V_4_2);

    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray originalBytes = f.readAll();
    f.close();

    Application::migrationEnabled = true;
    QuickWorkSpace ws;
    try {
        ws.load(path);
    } catch (const std::exception &e) {
        Application::migrationEnabled = false;
        QFAIL(qPrintable(QString("load() must not throw for a valid (empty) old-format file: %1").arg(e.what())));
    }
    Application::migrationEnabled = false;

    const QString backupPath = m_tempDir.path() + "/old_enabled.v4.2.panda";
    QVERIFY2(QFile::exists(backupPath), qPrintable("Backup file not found: " + backupPath));

    QFile bf(backupPath);
    QVERIFY(bf.open(QIODevice::ReadOnly));
    QCOMPARE(bf.readAll(), originalBytes);
}

void TestQuickWorkSpace::testMigrationUpdatesFileVersion()
{
    QVERIFY2(m_tempDir.isValid(), "Temp dir must be valid");
    const QString path = m_tempDir.path() + "/old_resaved.panda";

    writeMinimalOldFormatFile(path, Versions::V_4_2);

    Application::migrationEnabled = true;
    QuickWorkSpace ws;
    try {
        ws.load(path);
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

void TestQuickWorkSpace::testMigrationCurrentVersionSkips()
{
    QVERIFY2(m_tempDir.isValid(), "Temp dir must be valid");
    const QString path = m_tempDir.path() + "/current_format.panda";

    {
        QuickWorkSpace ws;
        ws.canvas()->addItem(ElementFactory::buildElement(ElementType::And));
        ws.save(path);
    }

    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QDataStream stream(&file);
        QCOMPARE(Serialization::readPandaHeader(stream), FormatRev::current);
    }

    Application::migrationEnabled = true;
    QuickWorkSpace ws2;
    try {
        ws2.load(path);
    } catch (const std::exception &e) {
        Application::migrationEnabled = false;
        QFAIL(qPrintable(QString("load() must not throw for a current-format file: %1").arg(e.what())));
    }
    Application::migrationEnabled = false;

    const QString backupPattern = m_tempDir.path() + "/current_format.v" + FormatRev::current.toString() + ".panda";
    QVERIFY2(!QFile::exists(backupPattern), "No backup must be created for a current-version file");

    const QStringList backups = QDir(m_tempDir.path()).entryList(QStringList{"current_format.v*.panda"}, QDir::Files);
    QVERIFY2(backups.isEmpty(), "No versioned backup should be created for a current-format file");
}

// ============================================================
// Version-warning / read-only / dialog-driven paths (ported from TestWorkspaceUnit)
// ============================================================

namespace {

#if !defined(Q_OS_WIN)
// RAII guard that caps the process's own max file size (RLIMIT_FSIZE) to a few bytes for its
// lifetime, forcing a deferred QSaveFile::commit() write failure without needing a second OS
// user, root, or a full disk. Mirrors Tests/Common/TestUtils.h's identical ScopedTinyFsizeLimit
// -- duplicated locally rather than included from there, since that header also pulls in
// App/Scene/Scene.h (Widgets-only, and stale against the current SimulationHost interface,
// so including it breaks the wiredpanda test build).
struct ScopedTinyFsizeLimit {
    struct rlimit previousLimit {};
    void (*previousHandler)(int) = nullptr;

    ScopedTinyFsizeLimit()
    {
        previousHandler = std::signal(SIGXFSZ, SIG_IGN);
        getrlimit(RLIMIT_FSIZE, &previousLimit);
        struct rlimit tiny = previousLimit;
        tiny.rlim_cur = 16;
        setrlimit(RLIMIT_FSIZE, &tiny);
    }

    ~ScopedTinyFsizeLimit()
    {
        setrlimit(RLIMIT_FSIZE, &previousLimit);
        std::signal(SIGXFSZ, previousHandler);
    }
};
#endif

// Builds a hand-crafted load stream at QVersionNumber(4, 4) -- older than FormatRev::current
// and predating both hasUnifiedMetadata (V_4_6) and hasMetadata (V_4_5), so the payload is
// just a bare dolphinFileName + sceneRect with zero elements following. Also predates
// hasCompressedPayload (Rev100), so no qCompress() wrapping is needed. Mirrors
// TestWorkspace.cpp's identical helper.
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

void TestQuickWorkSpace::testSaveWarnsAndNoOpsForNewerVersionFile()
{
    QuickWorkSpace workspace;
    workspace.m_loadedVersion = QVersionNumber(999);

    const bool prevInteractive = Application::interactiveMode;
    Application::interactiveMode = true;
    ScopedDialogStub dialogGuard;

    const QString path = m_tempDir.path() + "/newer_version_out.panda";
    QFile::remove(path);
    const auto outcome = workspace.save(path);

    Application::interactiveMode = prevInteractive;

    QCOMPARE(static_cast<int>(outcome), static_cast<int>(QuickWorkSpace::SaveOutcome::Saved));
    QCOMPARE(dialogGuard.stub.choiceCallCount, 1);
    QVERIFY2(!QFile::exists(path), "A newer-version file must not actually be written to disk");
}

void TestQuickWorkSpace::testSaveCopiesAssociatedDolphinFile()
{
    QTemporaryDir oldDir;
    QTemporaryDir newDir;
    QVERIFY(oldDir.isValid() && newDir.isValid());

    QFile dolphinFile(oldDir.filePath("waveform.dolphin"));
    QVERIFY(dolphinFile.open(QIODevice::WriteOnly));
    dolphinFile.write("dummy");
    dolphinFile.close();

    QuickWorkSpace workspace;
    workspace.canvas()->setContextDir(oldDir.path());
    workspace.setDolphinFileName("waveform.dolphin");

    const auto outcome = workspace.save(newDir.filePath("out.panda"));
    QCOMPARE(static_cast<int>(outcome), static_cast<int>(QuickWorkSpace::SaveOutcome::Saved));
    QVERIFY2(QFile::exists(newDir.filePath("waveform.dolphin")),
             "The associated dolphin waveform file must be copied alongside the .panda file");
}

void TestQuickWorkSpace::testSaveRemovesStaleAutosaveFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString autosavePath = dir.filePath("stale_autosave.panda");
    QFile autosaveFile(autosavePath);
    QVERIFY(autosaveFile.open(QIODevice::WriteOnly));
    autosaveFile.write("dummy");
    autosaveFile.close();

    QuickWorkSpace workspace;
    workspace.m_autosaveFileName = autosavePath;
    Settings::setAutosaveFiles(QStringList{autosavePath});

    const auto outcome = workspace.save(dir.filePath("out.panda"));
    QCOMPARE(static_cast<int>(outcome), static_cast<int>(QuickWorkSpace::SaveOutcome::Saved));
    QVERIFY2(!QFile::exists(autosavePath), "A successful save must remove the now-redundant autosave file");
    QVERIFY(!Settings::autosaveFiles().contains(autosavePath));
    QVERIFY(workspace.m_autosaveFileName.isEmpty());
}

void TestQuickWorkSpace::testSaveReturnsReadOnlyTargetWhenCommitFailsInteractive()
{
#ifdef Q_OS_WIN
    QSKIP("RLIMIT_FSIZE (used to force a deferred QSaveFile write failure) has no Windows equivalent");
#else
    // isReadOnlyFailure() treats QFileDevice::WriteError (what a deferred QSaveFile write
    // failure surfaces as at commit()) the same as an open()-time permission failure, so in
    // interactive mode this must return ReadOnlyTarget, not throw.
    QuickWorkSpace workspace;
    workspace.canvas()->addItem(ElementFactory::buildElement(ElementType::And));

    const QString path = m_tempDir.path() + "/readonly_target_out.panda";
    QFile::remove(path);

    const bool prevInteractive = Application::interactiveMode;
    Application::interactiveMode = true;

    QuickWorkSpace::SaveOutcome outcome;
    {
        ScopedTinyFsizeLimit tinyLimit;
        outcome = workspace.save(path);
    }
    Application::interactiveMode = prevInteractive;

    QCOMPARE(static_cast<int>(outcome), static_cast<int>(QuickWorkSpace::SaveOutcome::ReadOnlyTarget));
    QVERIFY2(!QFile::exists(path), "A failed commit() must not leave a partial file behind");
#endif
}

void TestQuickWorkSpace::testSaveThrowsWhenCommitFailsNonInteractive()
{
#ifdef Q_OS_WIN
    QSKIP("RLIMIT_FSIZE (used to force a deferred QSaveFile write failure) has no Windows equivalent");
#else
    // Application::interactiveMode stays false (the test-suite default), so save()'s
    // commit()-failure guard takes the throw branch instead of ReadOnlyTarget.
    QuickWorkSpace workspace;
    workspace.canvas()->addItem(ElementFactory::buildElement(ElementType::And));

    const QString path = m_tempDir.path() + "/nonint_commit_fail_out.panda";
    QFile::remove(path);

    bool threw = false;
    {
        ScopedTinyFsizeLimit tinyLimit;
        try {
            workspace.save(path);
        } catch (const std::exception &) {
            threw = true;
        }
    }
    QVERIFY2(threw, "save() must throw when commit() fails outside interactive mode");
    QVERIFY2(!QFile::exists(path), "A failed commit() must not leave a partial file behind");
#endif
}

void TestQuickWorkSpace::testLoadThrowsWhenFileCannotBeOpened()
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

    QuickWorkSpace workspace;
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

void TestQuickWorkSpace::testLoadMigratesNonPandaSuffixedFileName()
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

    QuickWorkSpace workspace;
    workspace.load(path);

    Application::migrationEnabled = prevMigration;

    QVERIFY2(QFile::exists(path + ".panda"),
             "Migrating a non-.panda-suffixed file must append the suffix before re-saving");
}

void TestQuickWorkSpace::testLoadWarnsWhenMigrationTargetIsReadOnly()
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

    // Lock the directory only after the fixtures are written.
    QVERIFY(QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));

    const bool prevMigration = Application::migrationEnabled;
    const bool prevInteractive = Application::interactiveMode;
    Application::migrationEnabled = true;
    Application::interactiveMode = true;
    ScopedDialogStub dialogGuard;

    QuickWorkSpace workspace;
    workspace.load(path);

    Application::migrationEnabled = prevMigration;
    Application::interactiveMode = prevInteractive;

    // One dialog for the older-format upgrade notice; the migration re-save then silently
    // finds the target read-only (logged via qCWarning, not a second dialog).
    QCOMPARE(dialogGuard.stub.choiceCallCount, 1);

    QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
#endif
}

void TestQuickWorkSpace::testLoadWarnsForNewerVersionFile()
{
    QByteArray data = buildNewerFormatLoadStream();
    QDataStream stream(&data, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    QuickWorkSpace workspace;
    workspace.m_fileInfo = QFileInfo("dummy.panda");

    const bool prevInteractive = Application::interactiveMode;
    Application::interactiveMode = true;
    ScopedDialogStub dialogGuard;

    workspace.load(stream, version, QString());

    Application::interactiveMode = prevInteractive;
    QCOMPARE(dialogGuard.stub.choiceCallCount, 1);
}

void TestQuickWorkSpace::testLoadInfoForOlderVersionFile()
{
    QByteArray data = buildOldFormatLoadStream();
    QDataStream stream(&data, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    QuickWorkSpace workspace;
    workspace.m_fileInfo = QFileInfo("dummy.panda");

    const bool prevInteractive = Application::interactiveMode;
    Application::interactiveMode = true;
    ScopedDialogStub dialogGuard;

    workspace.load(stream, version, QString());

    Application::interactiveMode = prevInteractive;
    QCOMPARE(dialogGuard.stub.choiceCallCount, 1);
}

void TestQuickWorkSpace::testAutosaveSkipsInlineICTabs()
{
    QuickWorkSpace workspace;
    workspace.m_isInlineIC = true;
    workspace.m_autosaveFileName.clear();
    workspace.autosave();
    QVERIFY(workspace.m_autosaveFileName.isEmpty());
}

void TestQuickWorkSpace::testAutosaveSkipsNewerVersionFile()
{
    QuickWorkSpace workspace;
    workspace.m_loadedVersion = QVersionNumber(999);
    workspace.m_autosaveFileName.clear();
    workspace.autosave();
    QVERIFY(workspace.m_autosaveFileName.isEmpty());
}

void TestQuickWorkSpace::testAutosaveRemovesFileWhenUndoStackIsClean()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString autosavePath = dir.filePath("stale.panda");
    QFile file(autosavePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("dummy");
    file.close();

    QuickWorkSpace workspace;
    workspace.m_autosaveFileName = autosavePath;
    Settings::setAutosaveFiles(QStringList{autosavePath});
    QVERIFY(workspace.canvas()->undoStack()->isClean());

    QSignalSpy spy(&workspace, &QuickWorkSpace::fileChanged);
    workspace.autosave();

    QVERIFY2(!QFile::exists(autosavePath), "autosave() on a clean undo stack must remove the stale autosave file");
    QVERIFY(!Settings::autosaveFiles().contains(autosavePath));
    QVERIFY(workspace.m_autosaveFileName.isEmpty());
    QCOMPARE(spy.count(), 1);
}

void TestQuickWorkSpace::testAutosaveFallsBackToAppDataWhenProjectDirIsReadOnly()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot make a directory unwritable on Windows (uses ACLs, not Unix permission bits)");
#else
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));

    QuickWorkSpace workspace;
    workspace.m_fileInfo = QFileInfo(dir.filePath("project.panda"));
    workspace.canvas()->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::And)}, workspace.canvas()));
    QVERIFY(!workspace.canvas()->undoStack()->isClean());

    workspace.autosave();

    QVERIFY2(!workspace.m_autosaveFileName.isEmpty(), "autosave() must still produce a file even when the project dir is read-only");
    QVERIFY2(!workspace.m_autosaveFileName.startsWith(dir.path()),
             "must fall back to the AppData autosaves dir, not the read-only project dir");
    QVERIFY(QFile::exists(workspace.m_autosaveFileName));

    QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    QFile::remove(workspace.m_autosaveFileName);
#endif
}

void TestQuickWorkSpace::testAutosaveFallsBackToAppDataForBundledExamplesDir()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("Examples")));
    const QString examplesDir = QDir(dir.path()).absoluteFilePath(QStringLiteral("Examples"));

    // Deliberately left WRITABLE: this is the dev-checkout shape, where the read-only check
    // passes, so writability is not what keeps the autosave out.
    QVERIFY(QFileInfo(examplesDir).isWritable());

    QuickWorkSpace workspace;
    workspace.m_fileInfo = QFileInfo(examplesDir + QStringLiteral("/counter.panda"));
    workspace.canvas()->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::And)}, workspace.canvas()));
    QVERIFY(!workspace.canvas()->undoStack()->isClean());

    // Make that directory the bare-category CWD candidate, which is how a dev checkout's
    // Examples/ is actually reached.
    const QString previousCwd = QDir::currentPath();
    QVERIFY(QDir::setCurrent(dir.path()));
    workspace.autosave();
    QVERIFY(QDir::setCurrent(previousCwd));

    QVERIFY2(!workspace.m_autosaveFileName.isEmpty(),
             "autosave() must still protect the work -- the file is relocated, not skipped");
    QVERIFY2(!workspace.m_autosaveFileName.startsWith(examplesDir),
             "must not write an autosave into the bundled Examples directory");
    QVERIFY(QFile::exists(workspace.m_autosaveFileName));
    QVERIFY2(QDir(examplesDir).entryList(QDir::Files | QDir::Hidden).isEmpty(),
             "the bundled Examples directory must be left completely untouched");

    QFile::remove(workspace.m_autosaveFileName);
}

void TestQuickWorkSpace::testAutosaveRemovesPreviousFileWhenProjectDirChanges()
{
    QTemporaryDir oldDir;
    QTemporaryDir newDir;
    QVERIFY(oldDir.isValid() && newDir.isValid());

    const QString oldAutosavePath = oldDir.filePath(".old_autosave.panda");
    QFile oldFile(oldAutosavePath);
    QVERIFY(oldFile.open(QIODevice::WriteOnly));
    oldFile.write("dummy");
    oldFile.close();

    QuickWorkSpace workspace;
    workspace.m_autosaveFileName = oldAutosavePath;
    workspace.m_fileInfo = QFileInfo(newDir.filePath("project.panda"));

    workspace.canvas()->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::And)}, workspace.canvas()));

    workspace.autosave();

    QVERIFY2(!QFile::exists(oldAutosavePath),
             "Changing project directories must remove the stale autosave file in the old location");
    QVERIFY2(!workspace.m_autosaveFileName.isEmpty() && workspace.m_autosaveFileName != oldAutosavePath,
             "A new autosave file must be created in the new project directory");

    QFile::remove(workspace.m_autosaveFileName);
}

void TestQuickWorkSpace::testAutosaveThrowsWhenFileCannotBeOpened()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString autosavePath = dir.filePath("preset_autosave.panda");
    // A directory, not a file -- QSaveFile::open(WriteOnly) can't open it for writing.
    QVERIFY(QDir(dir.path()).mkdir("preset_autosave.panda"));

    QuickWorkSpace workspace;
    workspace.m_fileInfo = QFileInfo(dir.filePath("project.panda"));
    workspace.m_autosaveFileName = autosavePath;

    workspace.canvas()->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::And)}, workspace.canvas()));

    bool threw = false;
    try {
        workspace.autosave();
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(threw, "autosave() must throw when the autosave file cannot be opened");
}

void TestQuickWorkSpace::testAutosaveThrowsWhenCommitFails()
{
#ifdef Q_OS_WIN
    QSKIP("RLIMIT_FSIZE (used to force a deferred QSaveFile write failure) has no Windows equivalent");
#else
    QuickWorkSpace workspace;
    workspace.canvas()->receiveCommand(new CanvasAddItemsCommand({ElementFactory::buildElement(ElementType::And)}, workspace.canvas()));

    bool threw = false;
    {
        ScopedTinyFsizeLimit tinyLimit;
        try {
            workspace.autosave();
        } catch (const std::exception &) {
            threw = true;
        }
    }
    QVERIFY2(threw, "autosave() must throw when commit() fails");
#endif
}
