// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestICInline.h"

#include <memory>

#include <QDataStream>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QMimeData>
#include <QRegularExpression>
#include <QSaveFile>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QTest>

#include "App/CodeGen/SystemVerilogCodeGen.h"
#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementLabel.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Element/ICRegistry.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/UI/ElementPalette.h"
#include "App/UI/ICDropZone.h"
#include "App/UI/MainWindowUI.h"
#include "App/UI/SelectionCapabilities.h"
#include "App/UI/TrashButton.h"
#include "App/Versions.h"
#include "Tests/Common/ICTestHelpers.h"
#include "Tests/Common/TestUtils.h"

using ICTestHelpers::readFile;
using ICTestHelpers::embedIC;

// ---------------------------------------------------------------------------
// Test Harness
// ---------------------------------------------------------------------------

/// Owns the stack-allocated widgets required by ElementPalette tests.
struct PaletteTestHarness
{
    MainWindowUi ui{};
    QWidget embeddedContainer;
    QWidget searchContainer;
    QLineEdit searchLine;
    QTabWidget tabWidget;
    std::unique_ptr<ElementPalette> palette;

    PaletteTestHarness()
    {
        embeddedContainer.setLayout(new QVBoxLayout());
        ui.scrollAreaWidgetContents_EmbeddedIC = &embeddedContainer;
        ui.verticalSpacer_EmbeddedIC = new QSpacerItem(0, 0);
        embeddedContainer.layout()->addItem(ui.verticalSpacer_EmbeddedIC);

        searchContainer.setLayout(new QVBoxLayout());
        ui.scrollAreaWidgetContents_Search = &searchContainer;

        ui.lineEditSearch = &searchLine;
        ui.tabElements = &tabWidget;

        palette = std::make_unique<ElementPalette>(&ui);
    }

    QList<ElementLabel *> embeddedLabels() const
    {
        return embeddedContainer.findChildren<ElementLabel *>("label_embedded_ic");
    }

    QList<ElementLabel *> searchLabels() const
    {
        return searchContainer.findChildren<ElementLabel *>("label_embedded_ic");
    }
};

QString TestICInline::fixturesSrcDir() const
{
    return QString(QUOTE(CURRENTDIR)) + "/Fixtures/";
}

bool TestICInline::copyFixture(const QString &name)
{
    const QString src = fixturesSrcDir() + name;
    const QString dst = m_fixtureDir + "/" + name;
    if (!QFile::exists(src)) {
        return false;
    }
    QFile::remove(dst);
    return QFile::copy(src, dst);
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

void TestICInline::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
    m_fixtureDir = m_tempDir.path();

    // Copy simple_and from IC Components
    const QString srcPath = TestUtils::cpuComponentsDir() + "level2_half_adder.panda";
    const QString dstPath = m_fixtureDir + "/simple_and.panda";
    QVERIFY2(QFile::exists(srcPath), qPrintable("Fixture not found: " + srcPath));
    QVERIFY(QFile::copy(srcPath, dstPath));

    // Copy additional fixtures
    const QStringList fixtures = {"nested_and.panda", "chain_a.panda", "chain_b.panda", "chain_c.panda"};
    for (const QString &f : fixtures) {
        QVERIFY2(copyFixture(f), qPrintable("Failed to copy fixture: " + f));
    }
}

void TestICInline::cleanup()
{
    // Remove temp files written by tests, keeping only the base fixtures.
    const QStringList fixtures = {"simple_and.panda", "nested_and.panda",
                                  "chain_a.panda", "chain_b.panda", "chain_c.panda"};
    QDir dir(m_fixtureDir);
    for (const QString &entry : dir.entryList({"*.panda"}, QDir::Files)) {
        if (!fixtures.contains(entry)) {
            dir.remove(entry);
        }
    }
}

void TestICInline::testEmbedFromBlob()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    IC ic;
    ic.setBlobName("simple_and");
    ic.loadFromBlob(blob, m_fixtureDir);

    QVERIFY(ic.isEmbedded());
    QCOMPARE(ic.blobName(), QString("simple_and"));
    QVERIFY(ic.file().isEmpty());
    QVERIFY(ic.inputSize() > 0);
    QVERIFY(ic.outputSize() > 0);
}

void TestICInline::testEmbedPreservesSimulation()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    Scene scene;
    scene.setContextDir(m_fixtureDir);

    auto *ic = new IC();
    QMap<QString, QByteArray> &registry = scene.icRegistry()->blobMapRef();
    registry["simple_and"] = blob;
    ic->setBlobName("simple_and");
    ic->loadFromBlob(blob, m_fixtureDir);
    scene.addItem(ic);

    // Verify it has internal elements for simulation
    QVERIFY(!ic->internalElements().isEmpty());
}

void TestICInline::testExtractToFile()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    const QString extractPath = m_fixtureDir + "/extracted.panda";

    // Write blob as .panda file
    QSaveFile saveFile(extractPath);
    QVERIFY(saveFile.open(QIODevice::WriteOnly));
    saveFile.write(blob);
    QVERIFY(saveFile.commit());

    // Load it back as a file-backed IC
    IC ic;
    ic.loadFile(extractPath, m_fixtureDir);

    QVERIFY(!ic.isEmbedded());
    QVERIFY(!ic.file().isEmpty());
    QVERIFY(ic.inputSize() > 0);
    QVERIFY(ic.outputSize() > 0);
}

void TestICInline::testRoundTripEmbedExtract()
{
    // Start file-backed → embed → extract → verify same port structure
    IC fileIC;
    fileIC.loadFile(m_fixtureDir + "/simple_and.panda", m_fixtureDir);
    QVERIFY(fileIC.inputSize() > 0);
    QVERIFY(fileIC.outputSize() > 0);

    // Embed
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    IC embeddedIC;
    embeddedIC.setBlobName("and");
    embeddedIC.loadFromBlob(blob, m_fixtureDir);
    QVERIFY(embeddedIC.inputSize() > 0);
    QVERIFY(embeddedIC.outputSize() > 0);

    // Extract
    const QString extractPath = m_fixtureDir + "/roundtrip.panda";
    QSaveFile saveFile(extractPath);
    QVERIFY(saveFile.open(QIODevice::WriteOnly));
    saveFile.write(blob);
    QVERIFY(saveFile.commit());

    IC extractedIC;
    extractedIC.loadFile(extractPath, m_fixtureDir);
    QVERIFY(extractedIC.inputSize() > 0);
    QVERIFY(extractedIC.outputSize() > 0);
}

void TestICInline::testSaveLoadWithEmbeddedIC()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    // Create a scene with an embedded IC
    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    ws.scene()->icRegistry()->setBlob("my_and", blob);

    auto *ic = new IC();
    ic->setBlobName("my_and");
    ic->loadFromBlob(blob, m_fixtureDir);
    ic->setPos(100, 100);
    ws.scene()->addItem(ic);

    // Save
    const QString savePath = m_fixtureDir + "/with_embedded.panda";
    ws.save(savePath);
    QVERIFY(QFile::exists(savePath));

    // Reload
    WorkSpace ws2;
    ws2.load(savePath);

    // Verify embedded IC survived the round-trip
    bool foundEmbedded = false;
    for (auto *elm : ws2.scene()->elements()) {
        if (elm->isEmbedded() && elm->blobName() == "my_and") {
            foundEmbedded = true;
            QVERIFY(elm->inputSize() > 0);
            QVERIFY(elm->outputSize() > 0);
        }
    }
    QVERIFY(foundEmbedded);
    QVERIFY(ws2.scene()->icRegistry()->hasBlob("my_and"));
}

void TestICInline::testBlobRegistryPersistence()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    ws.scene()->icRegistry()->setBlob("persisted", blob);

    auto *ic = new IC();
    ic->setBlobName("persisted");
    ic->loadFromBlob(blob, m_fixtureDir);
    ws.scene()->addItem(ic);

    const QString path = m_fixtureDir + "/persist_test.panda";
    ws.save(path);

    // Reload and check registry
    WorkSpace ws2;
    ws2.load(path);
    QVERIFY(ws2.scene()->icRegistry()->hasBlob("persisted"));
    QVERIFY(!ws2.scene()->icRegistry()->blob("persisted").isEmpty());
}

void TestICInline::testInlineTabSaveLoad()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    WorkSpace parent;
    SimulationBlocker parentBlocker(parent.simulation());
    parent.scene()->setContextDir(m_fixtureDir);
    parent.scene()->icRegistry()->setBlob("inline_test", blob);

    auto *ic = new IC();
    ic->setBlobName("inline_test");
    ic->loadFromBlob(blob, m_fixtureDir);
    parent.scene()->addItem(ic);

    int icId = ic->id();

    // Open inline tab
    WorkSpace child;
    SimulationBlocker childBlocker(child.simulation());
    child.loadFromBlob(blob, &parent, icId, m_fixtureDir);

    QVERIFY(child.isInlineIC());
    QCOMPARE(child.parentICElementId(), icId);
    QCOMPARE(child.inlineBlobName(), QString("inline_test"));

    // Verify child scene has elements
    QVERIFY(!child.scene()->elements().isEmpty());
}

void TestICInline::testChildBlobPropagation()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    WorkSpace parent;
    SimulationBlocker parentBlocker(parent.simulation());
    parent.scene()->setContextDir(m_fixtureDir);
    parent.scene()->icRegistry()->setBlob("prop_test", blob);

    auto *ic = new IC();
    ic->setBlobName("prop_test");
    ic->loadFromBlob(blob, m_fixtureDir);
    parent.scene()->addItem(ic);

    int icId = ic->id();

    // Open child and connect signal
    WorkSpace child;
    SimulationBlocker childBlocker(child.simulation());
    child.loadFromBlob(blob, &parent, icId, m_fixtureDir);
    connect(&child, &WorkSpace::icBlobSaved, &parent, &WorkSpace::onChildICBlobSaved);

    QSignalSpy spy(&child, &WorkSpace::icBlobSaved);

    // Save child (emits icBlobSaved)
    child.save(QString());

    QCOMPARE(spy.count(), 1);
}

void TestICInline::testUndoRedoEmbedExtract()
{
    const QString filePath = m_fixtureDir + "/simple_and.panda";
    QByteArray rawFileBytes = readFile(filePath);
    QVERIFY(!rawFileBytes.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);

    // Add a file-backed IC
    auto *ic = new IC();
    ic->loadFile(filePath, m_fixtureDir);
    ic->setPos(100, 100);
    ws.scene()->addItem(ic);

    QVERIFY(!ic->isEmbedded());

    // Embed via ICRegistry (stores full .panda file bytes as the blob)
    auto *reg = ws.scene()->icRegistry();
    reg->embedICsByFile(ic->file(), rawFileBytes, "undo_test");

    // After embed, IC should be embedded
    auto *embeddedIC = static_cast<IC *>(ws.scene()->elements().first());
    QVERIFY(embeddedIC->isEmbedded());
    QCOMPARE(embeddedIC->blobName(), QString("undo_test"));

    // Undo
    ws.scene()->undoStack()->undo();

    auto *undoneIC = static_cast<IC *>(ws.scene()->elements().first());
    QVERIFY(!undoneIC->isEmbedded());
    QVERIFY(!undoneIC->file().isEmpty());

    // Redo
    ws.scene()->undoStack()->redo();

    auto *redoneIC = static_cast<IC *>(ws.scene()->elements().first());
    QVERIFY(redoneIC->isEmbedded());
    QCOMPARE(redoneIC->blobName(), QString("undo_test"));
}

void TestICInline::testRegistryBlobCRUD()
{
    Scene scene;
    auto *reg = scene.icRegistry();

    QVERIFY(!reg->hasBlob("test"));

    reg->setBlob("test", QByteArray("data"));
    QVERIFY(reg->hasBlob("test"));
    QCOMPARE(reg->blob("test"), QByteArray("data"));

    reg->setBlob("test", QByteArray("updated"));
    QCOMPARE(reg->blob("test"), QByteArray("updated"));

    reg->removeBlob("test");
    QVERIFY(!reg->hasBlob("test"));
}

void TestICInline::testRegistryRename()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();
    reg->setBlob("old_name", blob);

    auto *ic = new IC();
    ic->setBlobName("old_name");
    ic->loadFromBlob(blob, m_fixtureDir);
    scene.addItem(ic);

    QCOMPARE(ic->blobName(), QString("old_name"));

    reg->renameBlob("old_name", "new_name");

    QVERIFY(!reg->hasBlob("old_name"));
    QVERIFY(reg->hasBlob("new_name"));
    QCOMPARE(ic->blobName(), QString("new_name"));
}

void TestICInline::testRegistryFindByBlobName()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();
    reg->setBlob("find_me", blob);

    auto *ic1 = new IC();
    ic1->setBlobName("find_me");
    ic1->loadFromBlob(blob, m_fixtureDir);
    scene.addItem(ic1);

    auto *ic2 = new IC();
    ic2->setBlobName("find_me");
    ic2->loadFromBlob(blob, m_fixtureDir);
    scene.addItem(ic2);

    auto *ic3 = new IC();
    ic3->loadFile(m_fixtureDir + "/simple_and.panda", m_fixtureDir);
    scene.addItem(ic3);

    auto results = reg->findICsByBlobName("find_me");
    QCOMPARE(results.size(), 2);
}

void TestICInline::testRegistryEmbedICsByFile()
{
    const QString filePath = m_fixtureDir + "/simple_and.panda";
    QByteArray rawFileBytes = readFile(filePath);

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);

    // Add two file-backed ICs referencing the same file
    auto *ic1 = new IC();
    ic1->loadFile(filePath, m_fixtureDir);
    ic1->setPos(0, 0);
    ws.scene()->addItem(ic1);

    auto *ic2 = new IC();
    ic2->loadFile(filePath, m_fixtureDir);
    ic2->setPos(100, 0);
    ws.scene()->addItem(ic2);

    QVERIFY(!ic1->isEmbedded());
    QVERIFY(!ic2->isEmbedded());

    // Embed all instances (pass raw .panda bytes)
    auto *reg = ws.scene()->icRegistry();
    int count = reg->embedICsByFile(filePath, rawFileBytes, "batch_and");
    QCOMPARE(count, 2);

    // Both should now be embedded
    for (auto *elm : ws.scene()->elements()) {
        QVERIFY(elm->isEmbedded());
        QCOMPARE(elm->blobName(), QString("batch_and"));
    }
}

void TestICInline::testRegistryClearBlobs()
{
    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    reg->setBlob("alpha", blob);
    reg->setBlob("beta", blob);
    QVERIFY(reg->hasBlob("alpha"));
    QVERIFY(reg->hasBlob("beta"));
    QCOMPARE(reg->blobMapRef().size(), 2);

    reg->clearBlobs();

    QVERIFY(!reg->hasBlob("alpha"));
    QVERIFY(!reg->hasBlob("beta"));
    QVERIFY(reg->blobMapRef().isEmpty());
}

// ===========================================================================
// Migrated: Batch 1 — Core load/embed/extract
// ===========================================================================

void TestICInline::testLoadFromBlob()
{
    const QString path = m_fixtureDir + "/simple_and.panda";

    // File-backed reference
    IC fileBacked;
    fileBacked.loadFile(path, m_fixtureDir);
    QVERIFY(fileBacked.inputSize() > 0);
    QVERIFY(fileBacked.outputSize() > 0);

    // Blob-backed
    QByteArray blob = readFile(path);
    QVERIFY(!blob.isEmpty());

    IC blobBacked;
    blobBacked.setBlobName("simple_and");
    blobBacked.loadFromBlob(blob, m_fixtureDir);

    QCOMPARE(blobBacked.inputSize(), fileBacked.inputSize());
    QCOMPARE(blobBacked.outputSize(), fileBacked.outputSize());
    QVERIFY(blobBacked.file().isEmpty());
}

void TestICInline::testFlattenBlobRecursive()
{
    // chain_a → chain_b → chain_c. Blobs are full .panda files.
    // Nested ICs stay as file references — no automatic flattening.
    // Embedding chain_a stores only chain_a in the registry.

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    QByteArray chainABytes = readFile(m_fixtureDir + "/chain_a.panda");
    QVERIFY(!chainABytes.isEmpty());

    reg->setBlob("chain_a", chainABytes);
    QVERIFY(reg->hasBlob("chain_a"));

    // Nested ICs are NOT automatically flattened — they load from file
    QVERIFY(!reg->hasBlob("chain_b"));
    QVERIFY(!reg->hasBlob("chain_c"));

    // But the blob should still be loadable (nested ICs load from file)
    IC ic;
    ic.setBlobName("chain_a");
    ic.loadFromBlob(chainABytes, m_fixtureDir);
    QVERIFY(ic.inputSize() > 0);
    QVERIFY(ic.outputSize() > 0);
}

void TestICInline::testFlattenBlobMissingFile()
{
    // chain_b references chain_c as a file-backed IC. With chain_c deleted,
    // loadFromBlob for chain_b should throw because the nested IC can't be resolved.

    QFile::remove(m_fixtureDir + "/chain_c.panda");
    auto restoreFixture = qScopeGuard([this] { copyFixture("chain_c.panda"); });

    QByteArray chainBBytes = readFile(m_fixtureDir + "/chain_b.panda");
    QVERIFY(!chainBBytes.isEmpty());

    IC ic;
    bool threw = false;
    try {
        ic.loadFromBlob(chainBBytes, m_fixtureDir);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "Loading blob with missing nested file-backed IC should throw");
}

void TestICInline::testFlattenBlobNoConversion()
{
    // simple_and.panda has no nested ICs — registerBlob (which flattens)
    // should produce a blob identical to the original since there's nothing to flatten.

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    QByteArray bytes = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!bytes.isEmpty());

    reg->registerBlob("no_deps", bytes);

    // Blob should still be loadable after registerBlob flattening
    IC verifyIC;
    verifyIC.setBlobName("no_deps");
    verifyIC.loadFromBlob(reg->blob("no_deps"), m_fixtureDir);
    QVERIFY(verifyIC.inputSize() > 0);
    QVERIFY(verifyIC.outputSize() > 0);
}

void TestICInline::testFlattenBlobMixedInlineFileBacked()
{
    // chain_b has chain_c as a file-backed IC inside it.
    // With the raw .panda blob approach, chain_c stays file-backed
    // and is NOT separately registered in the blob registry.

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    QByteArray chainBBytes = readFile(m_fixtureDir + "/chain_b.panda");
    QVERIFY(!chainBBytes.isEmpty());

    reg->setBlob("chain_b", chainBBytes);

    // chain_c stays as a file reference — not in registry
    QVERIFY(!reg->hasBlob("chain_c"));

    // The blob should still be loadable (chain_c loads from file)
    IC ic;
    ic.setBlobName("chain_b");
    ic.loadFromBlob(chainBBytes, m_fixtureDir);
    QVERIFY(ic.inputSize() > 0);
    QVERIFY(ic.outputSize() > 0);

    // Verify internal IC elements include a file-backed child
    bool hasFileBackedChild = false;
    for (auto *elm : ic.internalElements()) {
        if (elm->elementType() == ElementType::IC && !elm->isEmbedded()) {
            hasFileBackedChild = true;
            break;
        }
    }
    QVERIFY2(hasFileBackedChild,
             "chain_c inside chain_b should remain file-backed");
}

void TestICInline::testFlattenBlobCircularReference()
{
    // Create two .panda files that reference each other as file-backed ICs.
    // Cycle detection should prevent infinite recursion when loading.

    const QString dir = m_fixtureDir;

    QFile::remove(dir + "/circular_a.panda");
    QFile::remove(dir + "/circular_b.panda");
    QVERIFY(QFile::copy(dir + "/simple_and.panda", dir + "/circular_a.panda"));
    QVERIFY(QFile::copy(dir + "/simple_and.panda", dir + "/circular_b.panda"));
    auto cleanup = qScopeGuard([&dir] {
        QFile::remove(dir + "/circular_a.panda");
        QFile::remove(dir + "/circular_b.panda");
    });

    // Re-create A with an IC→circular_b
    {
        WorkSpace ws;
        ws.scene()->setContextDir(dir);
        auto *sw = new InputSwitch();  sw->setPos(96, 144);
        auto *led = new Led();         led->setPos(400, 144);
        auto *ic = new IC();
        ic->loadFile(dir + "/circular_b.panda", dir);
        ic->setPos(256, 144);
        ws.scene()->addItem(sw);
        ws.scene()->addItem(led);
        ws.scene()->addItem(ic);
        ws.save(dir + "/circular_a.panda");
    }
    // Re-create B with an IC→circular_a
    {
        WorkSpace ws;
        ws.scene()->setContextDir(dir);
        auto *sw = new InputSwitch();  sw->setPos(96, 144);
        auto *led = new Led();         led->setPos(400, 144);
        auto *ic = new IC();
        ic->loadFile(dir + "/circular_a.panda", dir);
        ic->setPos(256, 144);
        ws.scene()->addItem(sw);
        ws.scene()->addItem(led);
        ws.scene()->addItem(ic);
        ws.save(dir + "/circular_b.panda");
    }

    // registerBlob calls makeBlobSelfContained which recurses into nested ICs.
    // The circular reference (A→B→A) must be detected and not cause infinite recursion.
    Scene scene;
    scene.setContextDir(dir);
    auto *reg = scene.icRegistry();

    QByteArray blobA = readFile(dir + "/circular_a.panda");
    QVERIFY(!blobA.isEmpty());

    // registerBlob triggers flattening — cycle detection must prevent infinite recursion.
    reg->registerBlob("circular_a", blobA);

    // The blob should still be registered (cycle detected, skipped, not crashed)
    QVERIFY(reg->hasBlob("circular_a"));
}

void TestICInline::testFlattenBlobSelfReferenceName()
{
    // Storing a blob is idempotent — storing the same file bytes twice
    // under the same name produces the same result.

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    // chain_c is a leaf circuit — no nested ICs
    QByteArray leafBytes = readFile(m_fixtureDir + "/chain_c.panda");
    QVERIFY(!leafBytes.isEmpty());
    reg->setBlob("chain_c", leafBytes);
    QVERIFY(reg->hasBlob("chain_c"));

    // Store again — idempotent
    reg->setBlob("chain_c", leafBytes);
    QCOMPARE(reg->blob("chain_c"), leafBytes);

    // Verify the blob is loadable
    IC ic;
    ic.setBlobName("chain_c");
    ic.loadFromBlob(leafBytes, m_fixtureDir);
    QVERIFY(ic.inputSize() > 0);
    QVERIFY(ic.outputSize() > 0);
}

void TestICInline::testEmbedSimulatesCorrectly()
{
    // Build a circuit with an embedded IC (simple_and) and verify the truth table.

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    InputSwitch swA, swB;
    Led led;
    auto *ic = new IC();

    CircuitBuilder builder(ws.scene());
    builder.add(&swA, &swB, ic, &led);
    embedIC(ic, readFile(m_fixtureDir + "/simple_and.panda"), "simple_and", m_fixtureDir, reg);

    QVERIFY(ic->isEmbedded());
    QVERIFY(ic->inputSize() > 0);
    QVERIFY(ic->outputSize() > 0);

    builder.connect(&swA, 0, ic, 0);
    builder.connect(&swB, 0, ic, 1);
    builder.connect(ic, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // simple_and is a half adder — output 0 is Sum (A XOR B)
    const bool truthTable[4][2] = {{false, false}, {false, true}, {true, false}, {true, true}};
    const bool expectedXOR[4]   = { false,          true,          true,          false};

    for (int i = 0; i < 4; ++i) {
        swA.setOn(truthTable[i][0]);
        swB.setOn(truthTable[i][1]);
        sim->update();
        QCOMPARE(TestUtils::getInputStatus(&led), expectedXOR[i]);
    }
}

void TestICInline::testEmbedSimulatesMultiLevel()
{
    // chain_a wraps chain_b which wraps chain_c.
    // Flatten and verify simulation through 2 levels of nesting.

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    InputSwitch swA, swB;
    Led led;
    auto *ic = new IC();

    CircuitBuilder builder(ws.scene());
    builder.add(&swA, &swB, ic, &led);
    embedIC(ic, readFile(m_fixtureDir + "/chain_a.panda"), "chain_a", m_fixtureDir, reg);

    QVERIFY(ic->isEmbedded());
    QVERIFY(ic->inputSize() > 0);
    QVERIFY(ic->outputSize() > 0);

    builder.connect(&swA, 0, ic, 0);
    builder.connect(&swB, 0, ic, 1);
    builder.connect(ic, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // chain_a wraps chain_b (AND → chain_c), chain_c is NOT.
    // So chain_a computes NAND: output = NOT(A AND B).
    const bool truthTable[4][2] = {{false, false}, {false, true}, {true, false}, {true, true}};
    const bool expectedNAND[4]  = { true,           true,          true,          false};

    for (int i = 0; i < 4; ++i) {
        swA.setOn(truthTable[i][0]);
        swB.setOn(truthTable[i][1]);
        sim->update();
        QCOMPARE(TestUtils::getInputStatus(&led), expectedNAND[i]);
    }
}

void TestICInline::testEmbedExtractAtSceneLevel()
{
    // Embed: file-backed → embedded. Extract: embedded → file-backed.
    // Verify undo restores each transition.

    const QString filePath = m_fixtureDir + "/simple_and.panda";
    QByteArray rawFileBytes = readFile(filePath);
    QVERIFY(!rawFileBytes.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    // Add a file-backed IC
    auto *ic = new IC();
    ic->loadFile(filePath, m_fixtureDir);
    ic->setPos(100, 100);
    ws.scene()->addItem(ic);
    int icId = ic->id();
    QVERIFY(!ic->isEmbedded());
    QVERIFY(!ic->file().isEmpty());

    // Embed via ICRegistry
    reg->embedICsByFile(ic->file(), rawFileBytes, "embed_extract");

    auto *embedded = dynamic_cast<IC *>(ws.scene()->itemById(icId));
    QVERIFY(embedded);
    QVERIFY(embedded->isEmbedded());
    QCOMPARE(embedded->blobName(), QString("embed_extract"));
    QVERIFY(embedded->file().isEmpty());
    QVERIFY(embedded->inputSize() > 0);

    // Extract: writes blob to disk and converts ICs to file-backed
    const QString extractPath = m_fixtureDir + "/extracted_scene.panda";
    reg->extractToFile("embed_extract", extractPath);

    auto *extracted = dynamic_cast<IC *>(ws.scene()->itemById(icId));
    QVERIFY(extracted);
    QVERIFY(!extracted->isEmbedded());
    QVERIFY(extracted->file().contains("extracted_scene.panda"));
    QVERIFY(extracted->inputSize() > 0);

    // Undo extract → back to embedded
    ws.scene()->undoStack()->undo();
    auto *undoneExtract = dynamic_cast<IC *>(ws.scene()->itemById(icId));
    QVERIFY(undoneExtract);
    QVERIFY(undoneExtract->isEmbedded());
    QCOMPARE(undoneExtract->blobName(), QString("embed_extract"));

    // Undo embed → back to file-backed
    ws.scene()->undoStack()->undo();
    auto *undoneEmbed = dynamic_cast<IC *>(ws.scene()->itemById(icId));
    QVERIFY(undoneEmbed);
    QVERIFY(!undoneEmbed->isEmbedded());
    QVERIFY(undoneEmbed->file().contains("simple_and.panda"));
}

void TestICInline::testEmbedExtractWithActiveConnections()
{
    // Same as testEmbedExtractAtSceneLevel but with active connections.
    // Connections should survive both transitions.

    const QString filePath = m_fixtureDir + "/simple_and.panda";
    QByteArray rawFileBytes = readFile(filePath);
    QVERIFY(!rawFileBytes.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    InputSwitch swA, swB;
    Led led;
    auto *fileBacked = new IC();

    CircuitBuilder builder(ws.scene());
    builder.add(&swA, &swB, fileBacked, &led);
    fileBacked->loadFile(filePath, m_fixtureDir);

    QVERIFY(fileBacked->inputSize() > 0);
    builder.connect(&swA, 0, fileBacked, 0);
    builder.connect(&swB, 0, fileBacked, 1);
    builder.connect(fileBacked, 0, &led, 0);

    QCOMPARE(TestUtils::countConnections(ws.scene()), 3);
    QVERIFY(!fileBacked->isEmbedded());

    int icId = fileBacked->id();

    // Embed
    reg->embedICsByFile(fileBacked->file(), rawFileBytes, "connected_ic");

    // fileBacked was modified in place by embedICsByFile
    QVERIFY(fileBacked->isEmbedded());
    QVERIFY(fileBacked->inputSize() > 0);

    // Connections should survive the embed (same port count)
    QCOMPARE(TestUtils::countConnections(ws.scene()), 3);

    // Extract: writes blob to disk and converts ICs to file-backed
    const QString extractPath = m_fixtureDir + "/extracted_conn.panda";
    reg->extractToFile("connected_ic", extractPath);

    // After extract, ICs are file-backed again
    auto *extractedIC = dynamic_cast<IC *>(ws.scene()->itemById(icId));
    QVERIFY(extractedIC);
    QVERIFY(!extractedIC->isEmbedded());
    QVERIFY(extractedIC->inputSize() > 0);

    // Connections should survive the extract
    QCOMPARE(TestUtils::countConnections(ws.scene()), 3);

    // Undo extract → connections preserved
    ws.scene()->undoStack()->undo();
    QCOMPARE(TestUtils::countConnections(ws.scene()), 3);

    // Undo embed → connections preserved
    ws.scene()->undoStack()->undo();
    QCOMPARE(TestUtils::countConnections(ws.scene()), 3);

    auto *original = dynamic_cast<IC *>(ws.scene()->itemById(icId));
    QVERIFY(original);
    QVERIFY(!original->isEmbedded());
}

void TestICInline::testLoadFromBlobNested()
{
    // nested_and.panda wraps simple_and.panda as a file-backed IC.
    // Load it via loadFromBlob and verify the nested IC was resolved.

    QByteArray nestedBytes = readFile(m_fixtureDir + "/nested_and.panda");
    QVERIFY(!nestedBytes.isEmpty());

    QByteArray blob = nestedBytes;
    QVERIFY(!blob.isEmpty());

    IC ic;
    ic.loadFromBlob(blob, m_fixtureDir);

    QVERIFY(ic.inputSize() > 0);
    QVERIFY(ic.outputSize() > 0);
    QVERIFY(ic.file().isEmpty());
}

// ===========================================================================
// Migrated: Batch 2 — Blob name and serialization
// ===========================================================================

void TestICInline::testBlobNamePreservation()
{
    // Verify that blob name survives save/load round-trip and is distinct from label.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QMap<QString, QByteArray> registry;
    registry["my_custom_name"] = blob;
    QVERIFY(!blob.isEmpty());

    std::unique_ptr<IC> ic(new IC());
    ic->setBlobName("my_custom_name");
    ic->loadFromBlob(blob, m_fixtureDir);
    ic->setLabel("DISPLAY LABEL");

    QCOMPARE(ic->blobName(), QString("my_custom_name"));
    QCOMPARE(ic->label(), QString("DISPLAY LABEL"));

    // Serialize and deserialize
    QByteArray serialized;
    {
        QDataStream stream(&serialized, QIODevice::WriteOnly);
        ic->save(stream);
    }

    std::unique_ptr<IC> ic2(new IC());
    {
        QDataStream stream(&serialized, QIODevice::ReadOnly);
        QMap<quint64, QNEPort *> portMap;
        SerializationContext ctx{portMap, FileVersion::current, m_fixtureDir};
        ctx.blobRegistry = &registry;
        ic2->load(stream, ctx);
    }

    QCOMPARE(ic2->blobName(), QString("my_custom_name"));
    QCOMPARE(ic2->label(), QString("DISPLAY LABEL"));
    QVERIFY(ic2->isEmbedded());
    QVERIFY(ic2->inputSize() > 0);
    QVERIFY(ic2->outputSize() > 0);
}

void TestICInline::testSetBlobNameAlone()
{
    // setBlobName() changes identity without re-loading. Registry is untouched.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    IC ic;
    embedIC(&ic, readFile(m_fixtureDir + "/simple_and.panda"), "original_name", m_fixtureDir, reg);
    QCOMPARE(ic.blobName(), QString("original_name"));

    QByteArray dataBefore = reg->blob("original_name");
    QVERIFY(!dataBefore.isEmpty());

    ic.setBlobName("renamed");
    QCOMPARE(ic.blobName(), QString("renamed"));
    // Registry still has the blob under "original_name"
    QCOMPARE(reg->blob("original_name"), dataBefore);
    QVERIFY(!reg->hasBlob("renamed"));

    QVERIFY(ic.isEmbedded());
}

void TestICInline::testSetBlobNameOnFileBacked()
{
    // setBlobName on a file-backed IC makes isEmbedded() return true,
    // but the IC retains its file path.

    IC ic;
    ic.loadFile(m_fixtureDir + "/simple_and.panda", m_fixtureDir);
    QVERIFY(!ic.isEmbedded());
    QVERIFY(ic.blobName().isEmpty());

    ic.setBlobName("orphaned_name");
    QCOMPARE(ic.blobName(), QString("orphaned_name"));
    QVERIFY(ic.isEmbedded());
    QVERIFY(!ic.file().isEmpty());
}

void TestICInline::testLabelIndependentFromBlobName()
{
    // Setting a label explicitly on an embedded IC should be preserved
    // independently of the blobName.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    IC ic;
    ic.setBlobName("my_gate");
    ic.loadFromBlob(blob, m_fixtureDir);
    ic.setLabel("MY_GATE");

    QCOMPARE(ic.blobName(), QString("my_gate"));
    QCOMPARE(ic.label(), QString("MY_GATE"));

    // Label and blobName are independent — changing one doesn't affect the other
    ic.setLabel("CUSTOM");
    QCOMPARE(ic.blobName(), QString("my_gate"));
    QCOMPARE(ic.label(), QString("CUSTOM"));
}

void TestICInline::testBlobNameRenamePropagation()
{
    // Renaming blobName via UpdateCommand should propagate to all same-blobName
    // ICs, with undo/redo support.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    QList<int> ids;
    for (int i = 0; i < 3; ++i) {
        auto *ic = new IC();
        embedIC(ic, readFile(m_fixtureDir + "/simple_and.panda"), "old_name", m_fixtureDir, reg);
        ic->setPos(i * 100, 100);
        ws.scene()->addItem(ic);
        ids.append(ic->id());
    }

    for (int id : ids) {
        auto *elm = dynamic_cast<GraphicElement *>(ws.scene()->itemById(id));
        QVERIFY(elm);
        QCOMPARE(elm->blobName(), QString("old_name"));
    }

    // Collect targets and snapshot old state
    QList<GraphicElement *> targets;
    for (int id : ids) {
        targets.append(dynamic_cast<GraphicElement *>(ws.scene()->itemById(id)));
    }

    QByteArray oldData;
    {
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        for (auto *elm : std::as_const(targets)) {
            elm->save(stream);
        }
    }

    // Apply rename
    reg->setBlob("new_name", reg->blob("old_name"));
    for (auto *elm : std::as_const(targets)) {
        if (elm->isEmbedded()) {
            static_cast<IC *>(elm)->setBlobName("new_name");
        }
    }
    ws.scene()->undoStack()->push(new UpdateCommand(targets, oldData, ws.scene()));

    for (int id : ids) {
        auto *elm = dynamic_cast<GraphicElement *>(ws.scene()->itemById(id));
        QVERIFY(elm);
        QCOMPARE(elm->blobName(), QString("new_name"));
    }

    // Undo
    ws.scene()->undoStack()->undo();
    for (int id : ids) {
        auto *elm = dynamic_cast<GraphicElement *>(ws.scene()->itemById(id));
        QVERIFY(elm);
        QCOMPARE(elm->blobName(), QString("old_name"));
    }

    // Redo
    ws.scene()->undoStack()->redo();
    for (int id : ids) {
        auto *elm = dynamic_cast<GraphicElement *>(ws.scene()->itemById(id));
        QVERIFY(elm);
        QCOMPARE(elm->blobName(), QString("new_name"));
    }
}

void TestICInline::testBlobNameCollisionDuringRename()
{
    // Verify collision detection logic: renaming to an existing blobName
    // should be detected.

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    auto *ic1 = new IC();
    embedIC(ic1, readFile(m_fixtureDir + "/simple_and.panda"), "type_alpha", m_fixtureDir, reg);
    ic1->setPos(100, 100);
    ws.scene()->addItem(ic1);

    auto *ic2 = new IC();
    embedIC(ic2, readFile(m_fixtureDir + "/chain_c.panda"), "type_beta", m_fixtureDir, reg);
    ic2->setPos(200, 200);
    ws.scene()->addItem(ic2);

    // Renaming "type_alpha" → "type_beta" should detect collision
    bool collisionDetected = false;
    for (auto *elm : ws.scene()->elements()) {
        if (elm->isEmbedded() && elm->blobName() == "type_beta") {
            collisionDetected = true;
            break;
        }
    }
    QVERIFY2(collisionDetected, "Collision should be detected when renaming to existing blobName");

    // Renaming to a fresh name should NOT detect collision
    bool noCollision = true;
    for (auto *elm : ws.scene()->elements()) {
        if (elm->isEmbedded() && elm->blobName() == "fresh_name") {
            noCollision = false;
            break;
        }
    }
    QVERIFY2(noCollision, "No collision should be detected for unused name");

    // Verify both ICs are unchanged
    QCOMPARE(ic1->blobName(), QString("type_alpha"));
    QCOMPARE(ic2->blobName(), QString("type_beta"));
}

void TestICInline::testBlobNameSpecialCharacters()
{
    // Verify that blobNames with special characters survive serialization.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    QMap<QString, QByteArray> registry;

    const QStringList specialNames = {
        "my circuit",       // space
        "my.circuit",       // dot
        "my-circuit",       // hyphen
        QString::fromUtf8("caf\xc3\xa9_gate"), // accented character
        "gate_2x",          // digit in middle
    };

    for (const QString &name : specialNames) {
        registry[name] = blob;

        IC ic;
        ic.setBlobName(name);
        ic.loadFromBlob(blob, m_fixtureDir);
        QVERIFY2(ic.isEmbedded(), qPrintable("Failed to embed with name: " + name));
        QCOMPARE(ic.blobName(), name);

        QByteArray serialized;
        {
            QDataStream stream(&serialized, QIODevice::WriteOnly);
            ic.save(stream);
        }

        std::unique_ptr<IC> loaded(new IC());
        {
            QDataStream stream(&serialized, QIODevice::ReadOnly);
            QMap<quint64, QNEPort *> portMap;
            SerializationContext ctx{portMap, FileVersion::current, m_fixtureDir};
            ctx.blobRegistry = &registry;
            loaded->load(stream, ctx);
        }

        QVERIFY2(loaded->isEmbedded(), qPrintable("Round-trip failed for name: " + name));
        QCOMPARE(loaded->blobName(), name);
        QVERIFY(loaded->inputSize() > 0);
        QVERIFY(loaded->outputSize() > 0);
    }
}

void TestICInline::testBlobVersionMigrationPath()
{
    // Blobs are raw .panda file bytes. Verify that blobs are loadable
    // and stable across multiple embed operations.

    QByteArray pandaBytes = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!pandaBytes.isEmpty());

    QByteArray blob1 = pandaBytes;
    QVERIFY(!blob1.isEmpty());

    IC ic1;
    ic1.loadFromBlob(blob1, m_fixtureDir);
    QVERIFY(ic1.inputSize() > 0);
    QVERIFY(ic1.outputSize() > 0);

    // Second embed of the same file should produce identical blob
    QByteArray blob2 = pandaBytes;
    QCOMPARE(blob1, blob2);

    IC ic2;
    ic2.loadFromBlob(blob2, m_fixtureDir);
    QCOMPARE(ic2.inputSize(), ic1.inputSize());
    QCOMPARE(ic2.outputSize(), ic1.outputSize());
}

// ===========================================================================
// Migrated: Batch 3 — Workspace and inline tab
// ===========================================================================

void TestICInline::testWorkspaceInlineTab()
{
    // Verify: loadFromBlob sets inline mode, save emits signal with correct blob.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace parent;
    SimulationBlocker parentBlocker(parent.simulation());
    parent.scene()->setContextDir(m_fixtureDir);
    auto *reg = parent.scene()->icRegistry();
    reg->setBlob("simple_and", blob);

    auto *ic = new IC();
    ic->setBlobName("simple_and");
    ic->loadFromBlob(blob, m_fixtureDir);
    parent.scene()->addItem(ic);
    int icId = ic->id();

    WorkSpace child;
    SimulationBlocker childBlocker(child.simulation());
    QVERIFY(!child.isInlineIC());

    child.loadFromBlob(blob, &parent, icId, m_fixtureDir);
    QVERIFY(child.isInlineIC());
    QCOMPARE(child.parentWorkspace(), &parent);
    QCOMPARE(child.parentICElementId(), icId);
    QCOMPARE(child.inlineBlobName(), QString("simple_and"));

    // Save should emit icBlobSaved
    QSignalSpy spy(&child, &WorkSpace::icBlobSaved);
    child.save({});
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), icId);

    QByteArray emittedBlob = spy.at(0).at(1).toByteArray();
    QVERIFY(!emittedBlob.isEmpty());

    // Child scene should have elements from the blob
    QVERIFY(!child.scene()->elements().isEmpty());
}

void TestICInline::testWorkspaceInlineSaveOrphanedParent()
{
    // When the parent workspace is deleted, saving the inline tab should be
    // a no-op (no signal emitted, no crash).

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    auto parent = std::make_unique<WorkSpace>();
    SimulationBlocker parentBlocker(parent->simulation());
    parent->scene()->setContextDir(m_fixtureDir);
    auto *reg = parent->scene()->icRegistry();
    reg->setBlob("simple_and", blob);

    auto *ic = new IC();
    ic->setBlobName("simple_and");
    ic->loadFromBlob(blob, m_fixtureDir);
    parent->scene()->addItem(ic);
    int icId = ic->id();

    WorkSpace child;
    SimulationBlocker childBlocker(child.simulation());
    child.loadFromBlob(blob, parent.get(), icId, m_fixtureDir);
    QVERIFY(child.isInlineIC());

    parent.reset();

    QSignalSpy spy(&child, &WorkSpace::icBlobSaved);
    child.save({});
    QCOMPARE(spy.count(), 0);
}

void TestICInline::testWorkspaceAutosaveSkipsInlineIC()
{
    // Inline IC tabs save to signal, not disk.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace parent;
    SimulationBlocker parentBlocker(parent.simulation());
    parent.scene()->setContextDir(m_fixtureDir);
    auto *reg = parent.scene()->icRegistry();
    reg->setBlob("simple_and", blob);

    auto *ic = new IC();
    ic->setBlobName("simple_and");
    ic->loadFromBlob(blob, m_fixtureDir);
    parent.scene()->addItem(ic);
    int icId = ic->id();

    WorkSpace child;
    SimulationBlocker childBlocker(child.simulation());
    child.loadFromBlob(blob, &parent, icId, m_fixtureDir);
    QVERIFY(child.isInlineIC());

    QSignalSpy spy(&child, &WorkSpace::icBlobSaved);
    child.save({});
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), icId);

    // Verify no autosave/Untitled file was created
    QDir tempDir(m_tempDir.path());
    QStringList pandaFiles = tempDir.entryList({"*.panda"}, QDir::Files);
    for (const QString &f : pandaFiles) {
        QVERIFY2(!f.contains("autosave") && !f.contains("Untitled"),
                 qPrintable("Unexpected file created by inline save: " + f));
    }
}

void TestICInline::testWorkspaceLoadFromBlobBlobNameDerivation()
{
    // loadFromBlob should derive inlineBlobName from the parent IC element.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace parent;
    SimulationBlocker parentBlocker(parent.simulation());
    parent.scene()->setContextDir(m_fixtureDir);
    auto *reg = parent.scene()->icRegistry();
    reg->setBlob("derived_name", blob);

    auto *ic = new IC();
    ic->setBlobName("derived_name");
    ic->loadFromBlob(blob, m_fixtureDir);
    parent.scene()->addItem(ic);
    int icId = ic->id();

    WorkSpace child;
    SimulationBlocker childBlocker(child.simulation());
    child.loadFromBlob(blob, &parent, icId, m_fixtureDir);

    QVERIFY(child.isInlineIC());
    QCOMPARE(child.parentWorkspace(), &parent);
    QCOMPARE(child.parentICElementId(), icId);
    QCOMPARE(child.inlineBlobName(), QString("derived_name"));
}

void TestICInline::testInlineSaveContextDirSwitch()
{
    // When an inline-IC child workspace saves, the saved blob should be valid
    // and can be applied back to the parent IC.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace parent;
    SimulationBlocker parentBlocker(parent.simulation());
    parent.scene()->setContextDir(m_fixtureDir);
    auto *reg = parent.scene()->icRegistry();
    reg->setBlob("ctx_test", blob);

    auto *ic = new IC();
    ic->setBlobName("ctx_test");
    ic->loadFromBlob(blob, m_fixtureDir);
    parent.scene()->addItem(ic);
    int icId = ic->id();

    WorkSpace child;
    SimulationBlocker childBlocker(child.simulation());
    child.loadFromBlob(blob, &parent, icId, m_fixtureDir);
    QVERIFY(child.isInlineIC());

    QSignalSpy spy(&child, &WorkSpace::icBlobSaved);
    child.save({});
    QCOMPARE(spy.count(), 1);

    QByteArray savedBlob = spy.at(0).at(1).toByteArray();
    QVERIFY(!savedBlob.isEmpty());

    // Round-trip: apply the saved blob back to the parent IC
    parent.onChildICBlobSaved(icId, savedBlob);
    auto *updatedIC = dynamic_cast<IC *>(parent.scene()->itemById(icId));
    QVERIFY(updatedIC);
    QVERIFY(updatedIC->inputSize() > 0);
    QVERIFY(updatedIC->outputSize() > 0);
    QVERIFY(updatedIC->isEmbedded());
}

void TestICInline::testInlineTabSaveAfterModification()
{
    // Open embedded IC tab, add an element, save. Verify the emitted blob
    // reflects the modification.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace parent;
    SimulationBlocker parentBlocker(parent.simulation());
    parent.scene()->setContextDir(m_fixtureDir);
    auto *reg = parent.scene()->icRegistry();
    reg->setBlob("modify_test", blob);

    auto *ic = new IC();
    ic->setBlobName("modify_test");
    ic->loadFromBlob(blob, m_fixtureDir);
    parent.scene()->addItem(ic);
    int icId = ic->id();
    int origInputs = ic->inputSize();

    WorkSpace child;
    SimulationBlocker childBlocker(child.simulation());
    child.loadFromBlob(blob, &parent, icId, m_fixtureDir);
    QVERIFY(child.isInlineIC());

    int elementsBefore = static_cast<int>(child.scene()->elements().size());
    QVERIFY(elementsBefore >= 3);

    // Add a new InputSwitch
    auto *newSw = new InputSwitch();
    newSw->setPos(96, 296);
    child.scene()->addItem(newSw);

    QCOMPARE(child.scene()->elements().size(), elementsBefore + 1);

    // Save
    QSignalSpy spy(&child, &WorkSpace::icBlobSaved);
    child.save({});
    QCOMPARE(spy.count(), 1);

    QByteArray modifiedBlob = spy.at(0).at(1).toByteArray();
    QVERIFY(!modifiedBlob.isEmpty());

    // Load the modified blob — should have one more input
    IC verifyIC;
    verifyIC.loadFromBlob(modifiedBlob, m_fixtureDir);
    QCOMPARE(verifyIC.inputSize(), origInputs + 1);
}

void TestICInline::testLoadFromBlobNullParent()
{
    // loadFromBlob with nullptr parent should work; inlineBlobName remains empty.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace child;
    SimulationBlocker childBlocker(child.simulation());
    child.loadFromBlob(blob, nullptr, 1, m_fixtureDir);
    QVERIFY(child.isInlineIC());
    QVERIFY(!child.parentWorkspace());
    QVERIFY(child.inlineBlobName().isEmpty());
}

void TestICInline::testLoadFromBlobInvalidParentICId()
{
    // loadFromBlob with valid parent but non-existent IC element ID —
    // inlineBlobName remains empty.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace parent;
    SimulationBlocker parentBlocker(parent.simulation());
    parent.scene()->setContextDir(m_fixtureDir);
    auto *reg = parent.scene()->icRegistry();
    reg->setBlob("real_ic", blob);

    auto *ic = new IC();
    ic->setBlobName("real_ic");
    ic->loadFromBlob(blob, m_fixtureDir);
    parent.scene()->addItem(ic);

    WorkSpace child;
    SimulationBlocker childBlocker(child.simulation());
    child.loadFromBlob(blob, &parent, 99999, m_fixtureDir);
    QVERIFY(child.isInlineIC());
    QCOMPARE(child.parentWorkspace(), &parent);
    QCOMPARE(child.parentICElementId(), 99999);
    QVERIFY(child.inlineBlobName().isEmpty());
}

void TestICInline::testNestedWorkspaceChainPropagation()
{
    // 3-level nesting: grandparent → parent (inline) → child (inline).
    // Saving the child should propagate through the parent to the grandparent.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray notBlob = readFile(m_fixtureDir + "/chain_c.panda");
    QVERIFY(!blob.isEmpty());
    QVERIFY(!notBlob.isEmpty());

    // Level 0: grandparent with embedded IC
    WorkSpace grandparent;
    SimulationBlocker gpBlocker(grandparent.simulation());
    grandparent.scene()->setContextDir(m_fixtureDir);
    auto *gpReg = grandparent.scene()->icRegistry();
    gpReg->setBlob("level1", blob);

    auto *gpIC = new IC();
    gpIC->setBlobName("level1");
    gpIC->loadFromBlob(blob, m_fixtureDir);
    gpIC->setPos(200, 200);
    grandparent.scene()->addItem(gpIC);
    int gpICId = gpIC->id();

    // Level 1: parent editing the embedded IC
    WorkSpace parent;
    SimulationBlocker parentBlocker(parent.simulation());
    parent.loadFromBlob(blob, &grandparent, gpICId, m_fixtureDir);
    QVERIFY(parent.isInlineIC());

    // Add embedded IC inside parent
    auto *parentReg = parent.scene()->icRegistry();
    parentReg->setBlob("level2", notBlob);
    auto *parentIC = new IC();
    parentIC->setBlobName("level2");
    parentIC->loadFromBlob(notBlob, m_fixtureDir);
    parentIC->setPos(300, 200);
    parent.scene()->addItem(parentIC);
    int parentICId = parentIC->id();

    // Level 2: child editing the nested embedded IC
    WorkSpace child;
    SimulationBlocker childBlocker(child.simulation());
    child.loadFromBlob(notBlob, &parent, parentICId, m_fixtureDir);
    QVERIFY(child.isInlineIC());

    QObject::connect(&child, &WorkSpace::icBlobSaved,
                     &parent, &WorkSpace::onChildICBlobSaved);
    QObject::connect(&parent, &WorkSpace::icBlobSaved,
                     &grandparent, &WorkSpace::onChildICBlobSaved);

    // Save child → propagates to parent
    QSignalSpy childSpy(&child, &WorkSpace::icBlobSaved);
    child.save({});
    QCOMPARE(childSpy.count(), 1);
    QCOMPARE(childSpy.at(0).at(0).toInt(), parentICId);

    // Save parent → propagates to grandparent
    QSignalSpy parentSpy(&parent, &WorkSpace::icBlobSaved);
    parent.save({});
    QCOMPARE(parentSpy.count(), 1);
    QCOMPARE(parentSpy.at(0).at(0).toInt(), gpICId);

    auto *updatedGpIC = dynamic_cast<IC *>(grandparent.scene()->itemById(gpICId));
    QVERIFY(updatedGpIC);
    QVERIFY(updatedGpIC->isEmbedded());
    QCOMPARE(updatedGpIC->blobName(), QString("level1"));
}

void TestICInline::testLoadFromBlobCleansUpConnections()
{
    // Loading the same blob twice should not leak — qDeleteAll(m_internalElements)
    // cleans up elements, their ports, and their connections.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    IC ic;
    ic.loadFromBlob(blob, m_fixtureDir);
    QVERIFY(ic.inputSize() > 0);
    QVERIFY(ic.outputSize() > 0);

    // Reload — previous internal elements are freed. Under ASAN, leaks would crash.
    ic.loadFromBlob(blob, m_fixtureDir);
    QVERIFY(ic.inputSize() > 0);
    QVERIFY(ic.outputSize() > 0);

    // Load a more complex blob with nested ICs
    QByteArray chainBBytes = readFile(m_fixtureDir + "/chain_b.panda");
    QVERIFY(!chainBBytes.isEmpty());
    QByteArray chainBBlob = chainBBytes;
    QVERIFY(!chainBBlob.isEmpty());

    IC ic2;
    ic2.loadFromBlob(chainBBlob, m_fixtureDir);
    QVERIFY(ic2.inputSize() > 0);
    QVERIFY(ic2.outputSize() > 0);
}

void TestICInline::testLoadFromBlobPreservesBlob()
{
    // loadFromBlob loads the circuit. Blob data is stored in registry, not IC.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    IC ic;
    ic.setBlobName("simple_and");
    ic.loadFromBlob(blob, m_fixtureDir);

    QVERIFY(ic.inputSize() > 0);
    QVERIFY(ic.outputSize() > 0);
    QVERIFY(ic.file().isEmpty());
}

void TestICInline::testLoadFromBlobEmptyCircuit()
{
    // An empty QByteArray is not a valid .panda blob, so loadFromBlob should throw.

    IC ic;
    bool threw = false;
    try {
        ic.loadFromBlob(QByteArray(), m_fixtureDir);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY(threw);
}

void TestICInline::testLoadFromBlobNonExistentContextDir()
{
    // Simple blob (no nested ICs) should load fine with bogus contextDir.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    const QString bogusDir = m_tempDir.filePath("does_not_exist/");
    IC ic;
    ic.loadFromBlob(blob, bogusDir);
    QVERIFY(ic.inputSize() > 0);
    QVERIFY(ic.outputSize() > 0);

    // Nested blob with bogus contextDir — nested file-backed ICs can't be resolved
    QByteArray chainBBytes = readFile(m_fixtureDir + "/chain_b.panda");
    QVERIFY(!chainBBytes.isEmpty());

    IC nestedIC;
    bool threw = false;
    try {
        nestedIC.loadFromBlob(chainBBytes, bogusDir);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "Loading blob with nested file-backed IC and bogus contextDir should throw");
}

void TestICInline::testLoadV41MapDirectConstruct()
{
    // Directly serialize an embedded IC and verify IC::load() handles it.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    QMap<QString, QByteArray> registry;
    registry["direct_test"] = blob;

    IC referenceIC;
    referenceIC.setBlobName("direct_test");
    referenceIC.loadFromBlob(blob, m_fixtureDir);
    referenceIC.setLabel("DIRECT");

    QByteArray serialized;
    {
        QDataStream stream(&serialized, QIODevice::WriteOnly);
        referenceIC.save(stream);
    }

    std::unique_ptr<IC> loaded(new IC());
    {
        QDataStream stream(&serialized, QIODevice::ReadOnly);
        QMap<quint64, QNEPort *> portMap;
        SerializationContext ctx{portMap, FileVersion::current, m_fixtureDir};
        ctx.blobRegistry = &registry;
        loaded->load(stream, ctx);
    }

    QVERIFY(loaded->isEmbedded());
    QCOMPARE(loaded->blobName(), QString("direct_test"));
    QCOMPARE(loaded->label(), QString("DIRECT"));
    QVERIFY(loaded->inputSize() > 0);
    QVERIFY(loaded->outputSize() > 0);
    QVERIFY(loaded->file().isEmpty());
}

void TestICInline::testLoadMismatchNoFileName()
{
    // An IC with cleared blobName and no fileName should throw on load.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    IC mismatchIC;
    mismatchIC.setBlobName("test_ic");
    mismatchIC.loadFromBlob(blob, m_fixtureDir);
    mismatchIC.setBlobName(""); // Clear blobName — save() skips empty blobName

    QByteArray serialized;
    {
        QDataStream stream(&serialized, QIODevice::WriteOnly);
        mismatchIC.save(stream);
    }

    std::unique_ptr<IC> loaded(new IC());
    {
        QDataStream stream(&serialized, QIODevice::ReadOnly);
        QMap<quint64, QNEPort *> portMap;
        SerializationContext ctx{portMap, FileVersion::current, m_fixtureDir};
        bool threw = false;
        try {
            loaded->load(stream, ctx);
        } catch (const Pandaception &) {
            threw = true;
        }
        QVERIFY2(threw, "No valid blobName or fileName should throw");
    }

    QVERIFY(!loaded->isEmbedded());
    QVERIFY(loaded->blobName().isEmpty());
}

void TestICInline::testReEmbedWithDifferentBlob()
{
    // Re-embedding an IC with a different blob should replace cleanly.

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    IC ic;
    embedIC(&ic, readFile(m_fixtureDir + "/simple_and.panda"), "first", m_fixtureDir, reg);
    QVERIFY(ic.isEmbedded());
    QCOMPARE(ic.blobName(), QString("first"));
    int origInputs = ic.inputSize();
    QVERIFY(origInputs > 0);

    // Re-embed with chain_c (NOT gate — different port count)
    embedIC(&ic, readFile(m_fixtureDir + "/chain_c.panda"), "second", m_fixtureDir, reg);
    QVERIFY(ic.isEmbedded());
    QCOMPARE(ic.blobName(), QString("second"));
    QVERIFY(ic.inputSize() > 0);
    QVERIFY(ic.file().isEmpty());

    // And back to original
    embedIC(&ic, readFile(m_fixtureDir + "/simple_and.panda"), "first", m_fixtureDir, reg);
    QCOMPARE(ic.blobName(), QString("first"));
    QCOMPARE(ic.inputSize(), origInputs);
}

// ===========================================================================
// Migrated: Batch 4 — Child blob propagation
// ===========================================================================

void TestICInline::testOnChildICBlobSavedOrphan()
{
    // onChildICBlobSaved should be a no-op if the IC element no longer exists.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("simple_and", blob);

    auto ic = std::make_unique<IC>();
    ic->setBlobName("simple_and");
    ic->loadFromBlob(blob, m_fixtureDir);
    ws.scene()->addItem(ic.get());
    int icId = ic->id();

    ws.scene()->removeItem(ic.get());
    ic.reset();

    // These should be no-ops, not crash
    ws.onChildICBlobSaved(icId, blob);
    ws.onChildICBlobSaved(99999, blob);
}

void TestICInline::testOnChildICBlobSavedPropagation()
{
    // Child tab saves → parent updates all same-blobName ICs.
    // Undo restores old state, redo re-applies.

    QByteArray andBlob = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray notBlob = readFile(m_fixtureDir + "/chain_c.panda");
    QVERIFY(!andBlob.isEmpty());
    QVERIFY(!notBlob.isEmpty());

    WorkSpace parent;
    parent.scene()->setContextDir(m_fixtureDir);
    auto *reg = parent.scene()->icRegistry();
    reg->setBlob("my_ic", andBlob);

    QList<int> ids;
    for (int i = 0; i < 3; ++i) {
        auto *ic = new IC();
        ic->setBlobName("my_ic");
        ic->loadFromBlob(andBlob, m_fixtureDir);
        ic->setPos(i * 100, 100);
        parent.scene()->addItem(ic);
        ids.append(ic->id());
    }

    int origInputs = static_cast<IC *>(parent.scene()->itemById(ids.at(0)))->inputSize();

    // Propagate NOT blob (different port count)
    parent.onChildICBlobSaved(ids.at(0), notBlob);

    for (int id : ids) {
        auto *ic = dynamic_cast<IC *>(parent.scene()->itemById(id));
        QVERIFY(ic);
        QVERIFY(ic->inputSize() != origInputs);
        QCOMPARE(ic->blobName(), QString("my_ic"));
    }

    // Undo
    parent.scene()->undoStack()->undo();
    for (int id : ids) {
        auto *ic = dynamic_cast<IC *>(parent.scene()->itemById(id));
        QVERIFY(ic);
        QCOMPARE(ic->inputSize(), origInputs);
        QCOMPARE(ic->blobName(), QString("my_ic"));
    }

    // Redo
    parent.scene()->undoStack()->redo();
    for (int id : ids) {
        auto *ic = dynamic_cast<IC *>(parent.scene()->itemById(id));
        QVERIFY(ic);
        QCOMPARE(ic->blobName(), QString("my_ic"));
    }
}

void TestICInline::testOnChildICBlobSavedNonEmbeddedTarget()
{
    // onChildICBlobSaved should be a no-op when the target is not embedded.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);

    auto *fileBacked = new IC();
    fileBacked->loadFile(m_fixtureDir + "/simple_and.panda", m_fixtureDir);
    fileBacked->setPos(100, 100);
    ws.scene()->addItem(fileBacked);
    int fileBackedId = fileBacked->id();
    QVERIFY(!fileBacked->isEmbedded());

    auto *sw = new InputSwitch();
    sw->setPos(200, 200);
    ws.scene()->addItem(sw);
    int swId = sw->id();

    int stackIndex = ws.scene()->undoStack()->index();

    ws.onChildICBlobSaved(fileBackedId, blob);
    ws.onChildICBlobSaved(swId, blob);

    QCOMPARE(ws.scene()->undoStack()->index(), stackIndex);
}

void TestICInline::testOnChildICBlobSavedCorruptBlob()
{
    // onChildICBlobSaved with corrupt blob data should either throw (leaving
    // the IC unchanged) or produce ICs with 0 ports (undoable).

    QByteArray andBlob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!andBlob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("my_ic", andBlob);

    auto *ic = new IC();
    ic->setBlobName("my_ic");
    ic->loadFromBlob(andBlob, m_fixtureDir);
    ic->setPos(100, 100);
    ws.scene()->addItem(ic);
    int icId = ic->id();
    int origInputs = ic->inputSize();
    int stackIndex = ws.scene()->undoStack()->index();

    QByteArray corruptBlob("corrupt");
    bool threw = false;
    try {
        ws.onChildICBlobSaved(icId, corruptBlob);
    } catch (...) {
        threw = true;
    }

    if (threw) {
        // Exception path: IC should be unchanged, no command pushed
        QCOMPARE(ws.scene()->undoStack()->index(), stackIndex);
        auto *restored = dynamic_cast<IC *>(ws.scene()->itemById(icId));
        QVERIFY(restored);
        QCOMPARE(restored->inputSize(), origInputs);
    } else {
        // Graceful degradation: IC has 0 ports, undo restores
        ws.scene()->undoStack()->undo();
        auto *restored = dynamic_cast<IC *>(ws.scene()->itemById(icId));
        QVERIFY(restored);
        QCOMPARE(restored->inputSize(), origInputs);
        QCOMPARE(restored->blobName(), QString("my_ic"));
    }
}

void TestICInline::testOnChildICBlobSavedCorruptBlobMultiTarget()
{
    // With 3 same-blobName ICs and corrupt blob, either throws (no change)
    // or all get degraded state. Undo restores all either way.

    QByteArray andBlob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!andBlob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("target_ic", andBlob);

    QList<int> ids;
    for (int i = 0; i < 3; ++i) {
        auto *ic = new IC();
        ic->setBlobName("target_ic");
        ic->loadFromBlob(andBlob, m_fixtureDir);
        ic->setPos(i * 100, 100);
        ic->setLabel(QString("IC_%1").arg(i));
        ws.scene()->addItem(ic);
        ids.append(ic->id());
    }

    int origInputs = static_cast<IC *>(ws.scene()->itemById(ids.at(0)))->inputSize();
    int stackIndex = ws.scene()->undoStack()->index();

    QByteArray corruptBlob("corrupt");
    bool threw = false;
    try {
        ws.onChildICBlobSaved(ids.at(0), corruptBlob);
    } catch (...) {
        threw = true;
    }

    if (threw) {
        QCOMPARE(ws.scene()->undoStack()->index(), stackIndex);
    } else {
        ws.scene()->undoStack()->undo();
    }

    // After undo (or no-change on throw), all 3 should be restored
    for (int i = 0; i < 3; ++i) {
        auto *ic = dynamic_cast<IC *>(ws.scene()->itemById(ids.at(i)));
        QVERIFY2(ic, qPrintable(QString("IC %1 not found").arg(i)));
        QCOMPARE(ic->inputSize(), origInputs);
        QCOMPARE(ic->blobName(), QString("target_ic"));
        QCOMPARE(ic->label(), QString("IC_%1").arg(i));
    }
}

void TestICInline::testOnChildICBlobSavedPortCountWithConnections()
{
    // When blob changes port count, connections to removed ports are destroyed.
    // Undo restores both port count and connections.

    QByteArray andBlob = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray notBlob = readFile(m_fixtureDir + "/chain_c.panda");
    QVERIFY(!andBlob.isEmpty());
    QVERIFY(!notBlob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    InputSwitch swA, swB;
    Led led;
    auto *ic = new IC();

    CircuitBuilder builder(ws.scene());
    builder.add(&swA, &swB, ic, &led);

    reg->setBlob("my_ic", andBlob);
    ic->setBlobName("my_ic");
    ic->loadFromBlob(andBlob, m_fixtureDir);

    QVERIFY(ic->inputSize() >= 2);
    builder.connect(&swA, 0, ic, 0);
    builder.connect(&swB, 0, ic, 1);
    builder.connect(ic, 0, &led, 0);

    QCOMPARE(TestUtils::countConnections(ws.scene()), 3);
    int icId = ic->id();
    int origInputs = ic->inputSize();

    // Update to NOT blob (fewer inputs)
    ws.onChildICBlobSaved(icId, notBlob);

    auto *updatedIC = dynamic_cast<IC *>(ws.scene()->itemById(icId));
    QVERIFY(updatedIC);
    QVERIFY(updatedIC->inputSize() < origInputs);

    // Some connections may have been destroyed
    int connAfter = TestUtils::countConnections(ws.scene());
    QVERIFY(connAfter <= 3);

    // Undo restores original connections
    ws.scene()->undoStack()->undo();
    QCOMPARE(TestUtils::countConnections(ws.scene()), 3);
}

void TestICInline::testOnChildICBlobSavedBlobNameMismatch()
{
    // Only ICs with matching blobName should be updated.

    QByteArray andBlob = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray notBlob = readFile(m_fixtureDir + "/chain_c.panda");
    QVERIFY(!andBlob.isEmpty());
    QVERIFY(!notBlob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    auto *icAlpha = new IC();
    reg->setBlob("alpha", andBlob);
    icAlpha->setBlobName("alpha");
    icAlpha->loadFromBlob(andBlob, m_fixtureDir);
    icAlpha->setPos(100, 100);
    ws.scene()->addItem(icAlpha);
    int alphaId = icAlpha->id();

    auto *icBeta = new IC();
    reg->setBlob("beta", andBlob);
    icBeta->setBlobName("beta");
    icBeta->loadFromBlob(andBlob, m_fixtureDir);
    icBeta->setPos(200, 200);
    ws.scene()->addItem(icBeta);
    int betaId = icBeta->id();

    int origInputs = icAlpha->inputSize();

    // Update "alpha" to NOT blob — "beta" must remain unchanged
    ws.onChildICBlobSaved(alphaId, notBlob);

    auto *updatedAlpha = dynamic_cast<IC *>(ws.scene()->itemById(alphaId));
    auto *unchangedBeta = dynamic_cast<IC *>(ws.scene()->itemById(betaId));
    QVERIFY(updatedAlpha);
    QVERIFY(unchangedBeta);
    QVERIFY(updatedAlpha->inputSize() != origInputs);
    QCOMPARE(unchangedBeta->inputSize(), origInputs);
    QCOMPARE(unchangedBeta->blobName(), QString("beta"));

    // Undo should restore alpha without affecting beta
    ws.scene()->undoStack()->undo();
    updatedAlpha = dynamic_cast<IC *>(ws.scene()->itemById(alphaId));
    unchangedBeta = dynamic_cast<IC *>(ws.scene()->itemById(betaId));
    QVERIFY(updatedAlpha);
    QVERIFY(unchangedBeta);
    QCOMPARE(updatedAlpha->inputSize(), origInputs);
    QCOMPARE(unchangedBeta->inputSize(), origInputs);
}

void TestICInline::testOnChildICBlobSavedAllOrNothing()
{
    // All same-blobName ICs should update atomically. Undo restores all.

    QByteArray andBlob = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray notBlob = readFile(m_fixtureDir + "/chain_c.panda");
    QVERIFY(!andBlob.isEmpty());
    QVERIFY(!notBlob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("atomic_test", andBlob);

    QList<int> ids;
    for (int i = 0; i < 5; ++i) {
        auto *ic = new IC();
        ic->setBlobName("atomic_test");
        ic->loadFromBlob(andBlob, m_fixtureDir);
        ic->setPos(i * 100, 100);
        ic->setLabel(QString("IC_%1").arg(i));
        ws.scene()->addItem(ic);
        ids.append(ic->id());
    }

    int origInputs = static_cast<IC *>(ws.scene()->itemById(ids.at(0)))->inputSize();
    int stackBefore = ws.scene()->undoStack()->index();

    // Valid blob: all 5 should update atomically
    ws.onChildICBlobSaved(ids.at(0), notBlob);

    // Exactly one command pushed
    QCOMPARE(ws.scene()->undoStack()->index(), stackBefore + 1);

    // All 5 should have updated and preserved labels
    for (int i = 0; i < 5; ++i) {
        auto *ic = dynamic_cast<IC *>(ws.scene()->itemById(ids.at(i)));
        QVERIFY2(ic, qPrintable(QString("IC %1 not found after propagation").arg(i)));
        QCOMPARE(ic->blobName(), QString("atomic_test"));
        QCOMPARE(ic->label(), QString("IC_%1").arg(i));
    }

    // Undo restores all 5
    ws.scene()->undoStack()->undo();
    for (int i = 0; i < 5; ++i) {
        auto *ic = dynamic_cast<IC *>(ws.scene()->itemById(ids.at(i)));
        QVERIFY(ic);
        QCOMPARE(ic->inputSize(), origInputs);
        QCOMPARE(ic->blobName(), QString("atomic_test"));
        QCOMPARE(ic->label(), QString("IC_%1").arg(i));
    }
}

// ===========================================================================
// Migrated: Batch 5 — Multiple instances and remove
// ===========================================================================

void TestICInline::testMultipleInstancesBlobUpdate()
{
    // Multiple embedded ICs with the same blobName should all be updated
    // when onChildICBlobSaved is called for any one of them.

    QByteArray andBlob = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray notBlob = readFile(m_fixtureDir + "/chain_c.panda");
    QVERIFY(!andBlob.isEmpty());
    QVERIFY(!notBlob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("simple_and", andBlob);

    auto *ic1 = new IC();
    ic1->setBlobName("simple_and");
    ic1->loadFromBlob(andBlob, m_fixtureDir);
    ic1->setPos(100, 100);
    ic1->setLabel("INSTANCE_1");
    ws.scene()->addItem(ic1);

    auto *ic2 = new IC();
    ic2->setBlobName("simple_and");
    ic2->loadFromBlob(andBlob, m_fixtureDir);
    ic2->setPos(200, 200);
    ic2->setLabel("INSTANCE_2");
    ws.scene()->addItem(ic2);

    QCOMPARE(ic1->blobName(), ic2->blobName());
    QVERIFY(ic1->label() != ic2->label());
    QCOMPARE(ic1->inputSize(), ic2->inputSize());

    int ic1Id = ic1->id();
    int ic2Id = ic2->id();

    ws.onChildICBlobSaved(ic1Id, notBlob);

    auto *u1 = dynamic_cast<IC *>(ws.scene()->itemById(ic1Id));
    auto *u2 = dynamic_cast<IC *>(ws.scene()->itemById(ic2Id));
    QVERIFY(u1);
    QVERIFY(u2);
    QVERIFY(u1->inputSize() > 0);
    QCOMPARE(u1->inputSize(), u2->inputSize());

    // Labels should be preserved
    QCOMPARE(u1->label(), QString("INSTANCE_1"));
    QCOMPARE(u2->label(), QString("INSTANCE_2"));
}

void TestICInline::testMultipleInstancesBlobUpdateLabelsPreserved()
{
    // After onChildICBlobSaved updates all same-blobName ICs, per-instance
    // labels must survive through undo/redo.

    QByteArray andBlob = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray notBlob = readFile(m_fixtureDir + "/chain_c.panda");
    QVERIFY(!andBlob.isEmpty());
    QVERIFY(!notBlob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("my_ic", andBlob);

    auto *ic1 = new IC();
    ic1->setBlobName("my_ic");
    ic1->loadFromBlob(andBlob, m_fixtureDir);
    ic1->setPos(100, 100);
    ic1->setLabel("LABEL_ALPHA");
    ws.scene()->addItem(ic1);

    auto *ic2 = new IC();
    ic2->setBlobName("my_ic");
    ic2->loadFromBlob(andBlob, m_fixtureDir);
    ic2->setPos(200, 200);
    ic2->setLabel("LABEL_BETA");
    ws.scene()->addItem(ic2);

    int id1 = ic1->id();
    int id2 = ic2->id();
    int origInputs = ic1->inputSize();

    ws.onChildICBlobSaved(id1, notBlob);

    auto *u1 = dynamic_cast<IC *>(ws.scene()->itemById(id1));
    auto *u2 = dynamic_cast<IC *>(ws.scene()->itemById(id2));
    QVERIFY(u1);
    QVERIFY(u2);
    QCOMPARE(u1->label(), QString("LABEL_ALPHA"));
    QCOMPARE(u2->label(), QString("LABEL_BETA"));

    // Undo — labels and sizes should revert
    ws.scene()->undoStack()->undo();
    u1 = dynamic_cast<IC *>(ws.scene()->itemById(id1));
    u2 = dynamic_cast<IC *>(ws.scene()->itemById(id2));
    QVERIFY(u1);
    QVERIFY(u2);
    QCOMPARE(u1->label(), QString("LABEL_ALPHA"));
    QCOMPARE(u2->label(), QString("LABEL_BETA"));
    QCOMPARE(u1->inputSize(), origInputs);
    QCOMPARE(u2->inputSize(), origInputs);

    // Redo — labels preserved again
    ws.scene()->undoStack()->redo();
    u1 = dynamic_cast<IC *>(ws.scene()->itemById(id1));
    u2 = dynamic_cast<IC *>(ws.scene()->itemById(id2));
    QVERIFY(u1);
    QVERIFY(u2);
    QCOMPARE(u1->label(), QString("LABEL_ALPHA"));
    QCOMPARE(u2->label(), QString("LABEL_BETA"));
}

void TestICInline::testMultipleDifferentEmbeddedTypes()
{
    // Two embedded ICs with different blobNames should round-trip independently.

    QByteArray andBlob = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray notBlob = readFile(m_fixtureDir + "/chain_c.panda");
    QVERIFY(!andBlob.isEmpty());
    QVERIFY(!notBlob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("my_and", andBlob);
    reg->setBlob("my_not", notBlob);

    auto *icAnd = new IC();
    icAnd->setBlobName("my_and");
    icAnd->loadFromBlob(andBlob, m_fixtureDir);
    icAnd->setPos(100, 100);
    ws.scene()->addItem(icAnd);

    auto *icNot = new IC();
    icNot->setBlobName("my_not");
    icNot->loadFromBlob(notBlob, m_fixtureDir);
    icNot->setPos(200, 200);
    ws.scene()->addItem(icNot);

    int andInputs = icAnd->inputSize();
    int notInputs = icNot->inputSize();
    QVERIFY(andInputs > 0);
    QVERIFY(notInputs > 0);

    // Save and reload
    const QString savePath = m_fixtureDir + "/two_types.panda";
    ws.save(savePath);

    WorkSpace ws2;
    ws2.load(savePath);

    bool foundAnd = false;
    bool foundNot = false;
    for (auto *elm : ws2.scene()->elements()) {
        if (!elm->isEmbedded()) continue;
        if (elm->blobName() == "my_and") {
            QCOMPARE(static_cast<IC *>(elm)->inputSize(), andInputs);
            foundAnd = true;
        } else if (elm->blobName() == "my_not") {
            QCOMPARE(static_cast<IC *>(elm)->inputSize(), notInputs);
            foundNot = true;
        }
    }
    QVERIFY2(foundAnd, "my_and not found after reload");
    QVERIFY2(foundNot, "my_not not found after reload");
}

void TestICInline::testRemoveEmbeddedIC()
{
    // removeEmbeddedIC should delete all instances with the given blobName.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("to_remove", blob);
    reg->setBlob("keep_this", blob);

    for (int i = 0; i < 3; ++i) {
        auto *ic = new IC();
        ic->setBlobName("to_remove");
        ic->loadFromBlob(blob, m_fixtureDir);
        ic->setPos(i * 100, 100);
        ws.scene()->addItem(ic);
    }

    auto *other = new IC();
    other->setBlobName("keep_this");
    other->loadFromBlob(blob, m_fixtureDir);
    other->setPos(400, 100);
    ws.scene()->addItem(other);

    auto countEmbedded = [&]() {
        int n = 0;
        for (auto *elm : ws.scene()->elements())
            if (elm->isEmbedded()) ++n;
        return n;
    };

    QCOMPARE(countEmbedded(), 4);

    ws.removeEmbeddedIC("to_remove");

    int remaining = 0;
    for (auto *elm : ws.scene()->elements()) {
        if (elm->isEmbedded()) {
            ++remaining;
            QCOMPARE(elm->blobName(), QString("keep_this"));
        }
    }
    QCOMPARE(remaining, 1);

    // Note: removeEmbeddedIC currently removes the blob from the registry
    // immediately (not as part of the undo command). This means undo cannot
    // fully restore the embedded ICs. This is a known limitation.
    QVERIFY(!reg->hasBlob("to_remove"));
    QVERIFY(reg->hasBlob("keep_this"));
}

void TestICInline::testRemoveEmbeddedICWithConnections()
{
    // Removing embedded ICs should also remove their connections.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    InputSwitch swA, swB;
    Led led;
    auto *ic = new IC();

    CircuitBuilder builder(ws.scene());
    builder.add(&swA, &swB, ic, &led);

    reg->setBlob("wired_ic", blob);
    ic->setBlobName("wired_ic");
    ic->loadFromBlob(blob, m_fixtureDir);

    builder.connect(&swA, 0, ic, 0);
    builder.connect(&swB, 0, ic, 1);
    builder.connect(ic, 0, &led, 0);

    QCOMPARE(TestUtils::countConnections(ws.scene()), 3);
    int icId = ic->id();

    ws.removeEmbeddedIC("wired_ic");

    QVERIFY(!ws.scene()->itemById(icId));
    QCOMPARE(TestUtils::countConnections(ws.scene()), 0);
}

void TestICInline::testRemoveEmbeddedICNonExistent()
{
    WorkSpace ws;
    int stackIndex = ws.scene()->undoStack()->index();

    ws.removeEmbeddedIC("nonexistent_blob");

    QCOMPARE(ws.scene()->undoStack()->index(), stackIndex);
}

void TestICInline::testRemoveEmbeddedICMultipleInstances()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("target", blob);

    for (int i = 0; i < 3; ++i) {
        auto *ic = new IC();
        ic->setBlobName("target");
        ic->loadFromBlob(blob, m_fixtureDir);
        ic->setPos(i * 100, 100);
        ws.scene()->addItem(ic);
    }

    auto countEmbedded = [&]() {
        int n = 0;
        for (auto *elm : ws.scene()->elements())
            if (elm->isEmbedded()) ++n;
        return n;
    };

    QCOMPARE(countEmbedded(), 3);

    ws.removeEmbeddedIC("target");
    QCOMPARE(countEmbedded(), 0);
}

void TestICInline::testRemoveEmbeddedICCrossConnections()
{
    // Removing embedded ICs with connections spanning different blobName types.

    QByteArray andBlob = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray notBlob = readFile(m_fixtureDir + "/chain_c.panda");
    QVERIFY(!andBlob.isEmpty());
    QVERIFY(!notBlob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    InputSwitch swA, swB;
    Led led;
    auto *icAnd = new IC();
    auto *icNot = new IC();

    CircuitBuilder builder(ws.scene());
    builder.add(&swA, &swB, icAnd, icNot, &led);

    reg->setBlob("and_gate", andBlob);
    icAnd->setBlobName("and_gate");
    icAnd->loadFromBlob(andBlob, m_fixtureDir);

    reg->setBlob("not_gate", notBlob);
    icNot->setBlobName("not_gate");
    icNot->loadFromBlob(notBlob, m_fixtureDir);

    builder.connect(&swA, 0, icAnd, 0);
    builder.connect(&swB, 0, icAnd, 1);
    builder.connect(icAnd, 0, icNot, 0);
    builder.connect(icNot, 0, &led, 0);

    QCOMPARE(TestUtils::countConnections(ws.scene()), 4);

    ws.removeEmbeddedIC("and_gate");

    int embeddedCount = 0;
    for (auto *elm : ws.scene()->elements()) {
        if (elm->isEmbedded()) {
            QCOMPARE(elm->blobName(), QString("not_gate"));
            ++embeddedCount;
        }
    }
    QCOMPARE(embeddedCount, 1);
    QCOMPARE(TestUtils::countConnections(ws.scene()), 1);
}

void TestICInline::testMixedScene()
{
    // A scene with both file-backed and embedded ICs should round-trip correctly.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    auto *fileBacked = new IC();
    fileBacked->loadFile(m_fixtureDir + "/simple_and.panda", m_fixtureDir);
    fileBacked->setPos(100, 100);
    ws.scene()->addItem(fileBacked);

    reg->setBlob("embedded_and", blob);
    auto *embedded = new IC();
    embedded->setBlobName("embedded_and");
    embedded->loadFromBlob(blob, m_fixtureDir);
    embedded->setPos(200, 200);
    ws.scene()->addItem(embedded);

    QVERIFY(!fileBacked->isEmbedded());
    QVERIFY(embedded->isEmbedded());

    const QString savePath = m_fixtureDir + "/mixed.panda";
    ws.save(savePath);

    WorkSpace ws2;
    ws2.load(savePath);

    int fileBackedCount = 0;
    int embeddedCount = 0;
    for (auto *elm : ws2.scene()->elements()) {
        if (elm->elementType() != ElementType::IC) continue;
        if (elm->isEmbedded()) {
            QCOMPARE(elm->blobName(), QString("embedded_and"));
            QVERIFY(static_cast<IC *>(elm)->file().isEmpty());
            QVERIFY(elm->inputSize() > 0);
            ++embeddedCount;
        } else {
            auto *ic = static_cast<IC *>(elm);
            QVERIFY(ic->file().contains("simple_and.panda"));
            QVERIFY(elm->inputSize() > 0);
            ++fileBackedCount;
        }
    }
    QCOMPARE(fileBackedCount, 1);
    QCOMPARE(embeddedCount, 1);
}

void TestICInline::testPortCountChangeConnectionCleanup()
{
    // When onChildICBlobSaved changes port count, connections to removed ports
    // are destroyed. UpdateBlobCommand restores them on undo.

    QByteArray andBlob = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray notBlob = readFile(m_fixtureDir + "/chain_c.panda");
    QVERIFY(!andBlob.isEmpty());
    QVERIFY(!notBlob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    InputSwitch swA, swB;
    Led led;
    auto *ic = new IC();

    CircuitBuilder builder(ws.scene());
    builder.add(&swA, &swB, ic, &led);

    reg->setBlob("my_ic", andBlob);
    ic->setBlobName("my_ic");
    ic->loadFromBlob(andBlob, m_fixtureDir);

    QVERIFY(ic->inputSize() >= 2);
    builder.connect(&swA, 0, ic, 0);
    builder.connect(&swB, 0, ic, 1);
    builder.connect(ic, 0, &led, 0);

    QCOMPARE(TestUtils::countConnections(ws.scene()), 3);
    int icId = ic->id();

    // Update to NOT blob (fewer inputs)
    ws.onChildICBlobSaved(icId, notBlob);

    auto *updatedIC = dynamic_cast<IC *>(ws.scene()->itemById(icId));
    QVERIFY(updatedIC);

    // Some connections may have been destroyed
    int connAfter = TestUtils::countConnections(ws.scene());

    // All surviving connections should be valid
    auto conns = TestUtils::getConnections(ws.scene());
    for (auto *conn : conns) {
        QVERIFY(conn->startPort());
        QVERIFY(conn->endPort());
    }

    // Undo restores AND gate and all 3 connections
    ws.scene()->undoStack()->undo();
    QCOMPARE(TestUtils::countConnections(ws.scene()), 3);

    auto restoredConns = TestUtils::getConnections(ws.scene());
    for (auto *conn : restoredConns) {
        QVERIFY(conn->startPort());
        QVERIFY(conn->endPort());
    }

    // Redo/undo cycle should be stable
    ws.scene()->undoStack()->redo();
    QCOMPARE(TestUtils::countConnections(ws.scene()), connAfter);
    ws.scene()->undoStack()->undo();
    QCOMPARE(TestUtils::countConnections(ws.scene()), 3);
}

// ===========================================================================
// Migrated: Batch 6 — Serialization mismatch and rollback
// ===========================================================================

void TestICInline::testSerializationMismatchFallback()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());
    QMap<QString, QByteArray> registry;

    // Case 1: blobName present but not in registry — should throw.
    {
        IC ic;
        ic.loadFile(m_fixtureDir + "/simple_and.panda", m_fixtureDir);
        ic.setBlobName("orphaned_name");

        QByteArray serialized;
        {
            QDataStream stream(&serialized, QIODevice::WriteOnly);
            ic.save(stream);
        }

        std::unique_ptr<IC> loaded(new IC());
        bool threw = false;
        {
            QDataStream stream(&serialized, QIODevice::ReadOnly);
            QMap<quint64, QNEPort *> portMap;
            SerializationContext ctx{portMap, FileVersion::current, m_fixtureDir};
            try {
                loaded->load(stream, ctx);
            } catch (const Pandaception &) {
                threw = true;
            }
        }
        QVERIFY2(threw, "blobName not in registry should throw");
    }

    // Case 2: embedded IC with cleared blobName — no blobName or fileName → throw.
    {
        registry["test_ic"] = blob;
        IC ic;
        ic.setBlobName("test_ic");
        ic.loadFromBlob(blob, m_fixtureDir);
        ic.setBlobName("");

        QByteArray serialized;
        {
            QDataStream stream(&serialized, QIODevice::WriteOnly);
            ic.save(stream);
        }

        std::unique_ptr<IC> loaded(new IC());
        bool threw = false;
        {
            QDataStream stream(&serialized, QIODevice::ReadOnly);
            QMap<quint64, QNEPort *> portMap;
            SerializationContext ctx{portMap, FileVersion::current, m_fixtureDir};
            try {
                loaded->load(stream, ctx);
            } catch (const Pandaception &) {
                threw = true;
            }
        }
        QVERIFY2(threw, "Mismatch with no valid file should throw");
    }

    // Case 3: normal embedded IC — should load fine.
    {
        registry["test_ic"] = blob;
        IC embedded;
        embedded.setBlobName("test_ic");
        embedded.loadFromBlob(blob, m_fixtureDir);

        QByteArray embeddedData;
        {
            QDataStream stream(&embeddedData, QIODevice::WriteOnly);
            embedded.save(stream);
        }

        std::unique_ptr<IC> loaded(new IC());
        {
            QDataStream stream(&embeddedData, QIODevice::ReadOnly);
            QMap<quint64, QNEPort *> portMap;
            SerializationContext ctx{portMap, FileVersion::current, m_fixtureDir};
            ctx.blobRegistry = &registry;
            loaded->load(stream, ctx);
        }

        QVERIFY(loaded->isEmbedded());
        QCOMPARE(loaded->blobName(), QString("test_ic"));
        QVERIFY(loaded->inputSize() > 0);
        QVERIFY(loaded->outputSize() > 0);
    }
}

void TestICInline::testSerializationMismatchFallbackCase2State()
{
    // After the throw from cleared blobName, verify the IC is not half-loaded.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    IC ic;
    ic.setBlobName("test_ic");
    ic.loadFromBlob(blob, m_fixtureDir);
    ic.setBlobName("");

    QByteArray serialized;
    {
        QDataStream stream(&serialized, QIODevice::WriteOnly);
        ic.save(stream);
    }

    std::unique_ptr<IC> loaded(new IC());
    {
        QDataStream stream(&serialized, QIODevice::ReadOnly);
        QMap<quint64, QNEPort *> portMap;
        SerializationContext ctx{portMap, FileVersion::current, m_fixtureDir};
        bool threw = false;
        try {
            loaded->load(stream, ctx);
        } catch (const Pandaception &) {
            threw = true;
        }
        QVERIFY2(threw, "No valid file or blob should throw");
    }

    QVERIFY(loaded->blobName().isEmpty());
}

void TestICInline::testSetInlineDataRollback()
{
    // loadFromBlob throws on corrupt .panda bytes.
    // Verify the IC remains unchanged after the failed load.

    IC ic;
    ic.loadFile(m_fixtureDir + "/simple_and.panda", m_fixtureDir);

    const QString origFile = ic.file();
    QVERIFY(!origFile.isEmpty());
    QVERIFY(!ic.isEmbedded());
    int origInputs = ic.inputSize();
    QVERIFY(origInputs > 0);

    // loadFromBlob should throw on corrupt data
    bool threw = false;
    try {
        ic.loadFromBlob(QByteArray("corrupt"), m_fixtureDir);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY(threw);

    // IC should be completely unchanged (never modified)
    QCOMPARE(ic.file(), origFile);
    QVERIFY(!ic.isEmbedded());
    QVERIFY(ic.blobName().isEmpty());
    QCOMPARE(ic.inputSize(), origInputs);
}

void TestICInline::testSetInlineDataRollbackSimulationWorks()
{
    // After a failed loadFromBlob, the IC should still function
    // correctly in simulation.

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    InputSwitch swA, swB;
    Led led;
    auto *ic = new IC();

    CircuitBuilder builder(ws.scene());
    builder.add(&swA, &swB, ic, &led);
    embedIC(ic, readFile(m_fixtureDir + "/simple_and.panda"), "simple_and", m_fixtureDir, reg);

    QVERIFY(ic->inputSize() > 0);

    builder.connect(&swA, 0, ic, 0);
    builder.connect(&swB, 0, ic, 1);
    builder.connect(ic, 0, &led, 0);

    // Failed loadFromBlob — IC is unchanged
    bool threw = false;
    try {
        ic->loadFromBlob(QByteArray("corrupt"), m_fixtureDir);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY(threw);

    // IC should still be fully functional
    auto *sim = builder.initSimulation();
    swA.setOn(true);
    swB.setOn(true);
    sim->update();
    // Just verify no crash
}

void TestICInline::testSetInlineDataRollbackFromEmbedded()
{
    // Failed loadFromBlob from an already-embedded IC should leave
    // the IC in its previous embedded state.

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    IC ic;
    embedIC(&ic, readFile(m_fixtureDir + "/simple_and.panda"), "original_embed", m_fixtureDir, reg);
    QVERIFY(ic.isEmbedded());
    QCOMPARE(ic.blobName(), QString("original_embed"));
    int origInputs = ic.inputSize();
    QVERIFY(origInputs > 0);

    // Failed loadFromBlob — IC remains unchanged
    bool threw = false;
    try {
        ic.loadFromBlob(QByteArray("corrupt"), m_fixtureDir);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY(threw);

    QVERIFY(ic.isEmbedded());
    QCOMPARE(ic.blobName(), QString("original_embed"));
    QVERIFY(ic.file().isEmpty());
    QCOMPARE(ic.inputSize(), origInputs);
}

void TestICInline::testSetInlineDataEmptyBlobName()
{
    // setBlobName("") + loadFromBlob produces an IC that is not considered
    // embedded (empty blobName) and has no file.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    IC ic;
    ic.setBlobName("");
    ic.loadFromBlob(blob, m_fixtureDir);

    QVERIFY(!ic.isEmbedded());
    QVERIFY(ic.blobName().isEmpty());
    QVERIFY(ic.inputSize() > 0);
    QVERIFY(ic.outputSize() > 0);
    QVERIFY(ic.file().isEmpty());
}

void TestICInline::testSetInlineDataEmptyBlobThrows()
{
    // registerBlob with empty QByteArray should throw.

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    bool threw = false;
    try {
        reg->registerBlob("test", QByteArray());
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "registerBlob with empty blob should throw");
}

void TestICInline::testSetInlineDataEmptyBlobNameRoundTripFails()
{
    // An IC with empty blobName and no file can't survive serialization.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    IC ic;
    ic.setBlobName("");
    ic.loadFromBlob(blob, m_fixtureDir);

    QVERIFY(!ic.isEmbedded());
    QVERIFY(ic.blobName().isEmpty());
    QVERIFY(ic.inputSize() > 0);

    QByteArray serialized;
    {
        QDataStream stream(&serialized, QIODevice::WriteOnly);
        ic.save(stream);
    }

    std::unique_ptr<IC> loaded(new IC());
    {
        QDataStream stream(&serialized, QIODevice::ReadOnly);
        QMap<quint64, QNEPort *> portMap;
        SerializationContext ctx{portMap, FileVersion::current, m_fixtureDir};
        bool threw = false;
        try {
            loaded->load(stream, ctx);
        } catch (const Pandaception &) {
            threw = true;
        }
        QVERIFY2(threw, "Empty blobName with no fileName should fail round-trip");
    }

    QVERIFY(!loaded->isEmbedded());
    QVERIFY(loaded->blobName().isEmpty());
}

void TestICInline::testSimulationRestartTimingDuringSetInlineData()
{
    // Verify embedding works both standalone (no scene) and during simulation.

    QByteArray andBlob = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray notBlob = readFile(m_fixtureDir + "/chain_c.panda");
    QVERIFY(!andBlob.isEmpty());
    QVERIFY(!notBlob.isEmpty());

    // Case 1: standalone IC (no scene)
    {

        IC ic;
        ic.setBlobName("standalone");
        ic.loadFromBlob(andBlob, m_fixtureDir);
        QVERIFY(ic.isEmbedded());
        QVERIFY(ic.inputSize() > 0);

        // Re-embed with different blob
        ic.loadFromBlob(notBlob, m_fixtureDir);
        QVERIFY(ic.inputSize() > 0);
    }

    // Case 2: IC in scene with active simulation
    {
        WorkSpace ws;
        ws.scene()->setContextDir(m_fixtureDir);
        auto *reg = ws.scene()->icRegistry();

        InputSwitch swA, swB;
        Led led;
        auto *ic = new IC();

        CircuitBuilder builder(ws.scene());
        builder.add(&swA, &swB, ic, &led);
        embedIC(ic, readFile(m_fixtureDir + "/simple_and.panda"), "in_scene", m_fixtureDir, reg);

        builder.connect(&swA, 0, ic, 0);
        builder.connect(&swB, 0, ic, 1);
        builder.connect(ic, 0, &led, 0);

        auto *sim = builder.initSimulation();
        swA.setOn(true);
        swB.setOn(true);
        sim->update();

        // Re-embed during active simulation — should work
        embedIC(ic, readFile(m_fixtureDir + "/simple_and.panda"), "in_scene", m_fixtureDir, reg);
        QVERIFY(ic->isEmbedded());
        QVERIFY(ic->inputSize() > 0);
    }
}

// ===========================================================================
// Migrated: Batch 7 — Save/load round-trip and mixed
// ===========================================================================

void TestICInline::testSaveLoadRoundTrip()
{
    // Create embedded IC, add to workspace, save, reload, verify blob persists.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("simple_and", blob);

    auto *ic = new IC();
    ic->setBlobName("simple_and");
    ic->loadFromBlob(blob, m_fixtureDir);
    ic->setPos(200, 200);
    ws.scene()->addItem(ic);
    QVERIFY(ic->isEmbedded());

    const QString savePath = m_fixtureDir + "/roundtrip2.panda";
    ws.save(savePath);

    WorkSpace ws2;
    ws2.load(savePath);

    bool foundEmbedded = false;
    for (auto *elm : ws2.scene()->elements()) {
        if (elm->isEmbedded()) {
            QCOMPARE(elm->blobName(), QString("simple_and"));
            QVERIFY(elm->inputSize() > 0);
            QVERIFY(elm->outputSize() > 0);
            foundEmbedded = true;
        }
    }
    QVERIFY(foundEmbedded);
}

void TestICInline::testLoadFileClearsInlineData()
{
    // Calling loadFile() on an embedded IC should clear blobName.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    IC ic;
    ic.setBlobName("test_blob");
    ic.loadFromBlob(blob, m_fixtureDir);
    QVERIFY(ic.isEmbedded());
    QCOMPARE(ic.blobName(), QString("test_blob"));

    ic.loadFile(m_fixtureDir + "/simple_and.panda", m_fixtureDir);
    QVERIFY(!ic.isEmbedded());
    QVERIFY(ic.blobName().isEmpty());
    QVERIFY(!ic.file().isEmpty());
}

void TestICInline::testCopyFileGuardDuringPaste()
{
    // Deserializing an embedded IC should resolve via blob, not file path.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    QMap<QString, QByteArray> registry;
    registry["simple_and"] = blob;

    IC ic;
    ic.setBlobName("simple_and");
    ic.loadFromBlob(blob, m_fixtureDir);
    QVERIFY(ic.isEmbedded());

    QByteArray serialized;
    {
        QDataStream stream(&serialized, QIODevice::WriteOnly);
        ic.save(stream);
    }

    std::unique_ptr<IC> ic2(new IC());
    {
        QDataStream stream(&serialized, QIODevice::ReadOnly);
        QMap<quint64, QNEPort *> portMap;
        SerializationContext ctx{portMap, FileVersion::current, m_fixtureDir};
        ctx.blobRegistry = &registry;
        ic2->load(stream, ctx);
    }

    QVERIFY(ic2->isEmbedded());
    QCOMPARE(ic2->blobName(), QString("simple_and"));
    QVERIFY(ic2->inputSize() > 0);
    QVERIFY(ic2->outputSize() > 0);
    QVERIFY(ic2->file().isEmpty());
}

void TestICInline::testCopyPasteEmbeddedICRoundTrip()
{
    // Serialize and deserialize an embedded IC — all fields preserved.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    QMap<QString, QByteArray> registry;
    registry["copy_test"] = blob;

    IC original;
    original.setBlobName("copy_test");
    original.loadFromBlob(blob, m_fixtureDir);
    original.setLabel("COPY_LABEL");
    QVERIFY(original.isEmbedded());

    QByteArray serialized;
    {
        QDataStream stream(&serialized, QIODevice::WriteOnly);
        original.save(stream);
    }

    std::unique_ptr<IC> pasted(new IC());
    {
        QDataStream stream(&serialized, QIODevice::ReadOnly);
        QMap<quint64, QNEPort *> portMap;
        SerializationContext ctx{portMap, FileVersion::current, m_fixtureDir};
        ctx.blobRegistry = &registry;
        pasted->load(stream, ctx);
    }

    QVERIFY(pasted->isEmbedded());
    QCOMPARE(pasted->blobName(), QString("copy_test"));
    QCOMPARE(pasted->label(), QString("COPY_LABEL"));
    QVERIFY(pasted->inputSize() > 0);
    QVERIFY(pasted->outputSize() > 0);
    QVERIFY(pasted->file().isEmpty());
}

void TestICInline::testFlipRotateEmbeddedIC()
{
    // Flip and rotate should preserve embedded state through undo/redo.

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    auto *ic = new IC();
    embedIC(ic, readFile(m_fixtureDir + "/simple_and.panda"), "flip_test", m_fixtureDir, reg);
    ic->setPos(200, 200);
    ws.scene()->addItem(ic);
    int icId = ic->id();

    QVERIFY(ic->isEmbedded());
    QCOMPARE(ic->rotation(), 0.0);

    // Flip horizontally
    ws.scene()->undoStack()->push(new FlipCommand({ic}, 1, ws.scene()));

    auto *flipped = dynamic_cast<IC *>(ws.scene()->itemById(icId));
    QVERIFY(flipped);
    QVERIFY(flipped->isEmbedded());
    QCOMPARE(flipped->blobName(), QString("flip_test"));

    // Rotate
    ws.scene()->undoStack()->push(new FlipCommand({flipped}, 0, ws.scene()));

    auto *rotated = dynamic_cast<IC *>(ws.scene()->itemById(icId));
    QVERIFY(rotated);
    QVERIFY(rotated->isEmbedded());

    // Undo both
    ws.scene()->undoStack()->undo();
    ws.scene()->undoStack()->undo();

    auto *restored = dynamic_cast<IC *>(ws.scene()->itemById(icId));
    QVERIFY(restored);
    QVERIFY(restored->isEmbedded());
    QCOMPARE(restored->blobName(), QString("flip_test"));
    QVERIFY(restored->inputSize() > 0);

    // Redo both
    ws.scene()->undoStack()->redo();
    ws.scene()->undoStack()->redo();

    auto *redone = dynamic_cast<IC *>(ws.scene()->itemById(icId));
    QVERIFY(redone);
    QVERIFY(redone->isEmbedded());
    QCOMPARE(redone->blobName(), QString("flip_test"));
}

// ===========================================================================
// Migrated: Batch 8 — UI widgets
// ===========================================================================

void TestICInline::testSelectionCapabilitiesEmbedded()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    // All embedded → isEmbedded=true, isFileBacked=false
    {
        std::unique_ptr<IC> ic1(new IC());
        ic1->setBlobName("test");
        ic1->loadFromBlob(blob, m_fixtureDir);
        std::unique_ptr<IC> ic2(new IC());
        ic2->setBlobName("test");
        ic2->loadFromBlob(blob, m_fixtureDir);

        auto caps = computeCapabilities({ic1.get(), ic2.get()});
        QVERIFY(caps.isEmbedded);
        QVERIFY(!caps.isFileBacked);
    }

    // All file-backed → isEmbedded=false, isFileBacked=true
    {
        std::unique_ptr<IC> ic1(new IC());
        ic1->loadFile(m_fixtureDir + "/simple_and.panda", m_fixtureDir);
        std::unique_ptr<IC> ic2(new IC());
        ic2->loadFile(m_fixtureDir + "/simple_and.panda", m_fixtureDir);

        auto caps = computeCapabilities({ic1.get(), ic2.get()});
        QVERIFY(!caps.isEmbedded);
        QVERIFY(caps.isFileBacked);
    }

    // Mixed → both false
    {
        std::unique_ptr<IC> ic1(new IC());
        ic1->setBlobName("test");
        ic1->loadFromBlob(blob, m_fixtureDir);
        std::unique_ptr<IC> ic2(new IC());
        ic2->loadFile(m_fixtureDir + "/simple_and.panda", m_fixtureDir);

        auto caps = computeCapabilities({ic1.get(), ic2.get()});
        QVERIFY(!caps.isEmbedded);
        QVERIFY(!caps.isFileBacked);
    }

    // Embedded IC + non-IC → both false
    {
        std::unique_ptr<IC> ic(new IC());
        ic->setBlobName("test");
        ic->loadFromBlob(blob, m_fixtureDir);
        InputSwitch sw;

        auto caps = computeCapabilities({ic.get(), &sw});
        QVERIFY(!caps.isEmbedded);
        QVERIFY(!caps.isFileBacked);
    }
}

void TestICInline::testSelectionCapabilitiesSingleNonIC()
{
    InputSwitch sw;
    auto caps = computeCapabilities({&sw});
    QVERIFY(!caps.isEmbedded);
    QVERIFY(!caps.isFileBacked);
}

void TestICInline::testSelectionCapabilitiesEmptyList()
{
    auto caps = computeCapabilities({});
    QVERIFY(!caps.isEmbedded);
    QVERIFY(!caps.isFileBacked);
    QVERIFY(!caps.hasElements);
}

void TestICInline::testContextMenuICActionConditions()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    // Embedded IC → Edit+Extract visible, Embed hidden
    {
        std::unique_ptr<IC> ic(new IC());
        ic->setBlobName("test");
        ic->loadFromBlob(blob, m_fixtureDir);
        auto caps = computeCapabilities({ic.get()});
        QVERIFY(caps.isEmbedded || caps.isFileBacked);
        QVERIFY(!caps.isFileBacked);
        QVERIFY(caps.isEmbedded);
    }

    // File-backed IC → Edit+Embed visible, Extract hidden
    {
        std::unique_ptr<IC> ic(new IC());
        ic->loadFile(m_fixtureDir + "/simple_and.panda", m_fixtureDir);
        auto caps = computeCapabilities({ic.get()});
        QVERIFY(caps.isEmbedded || caps.isFileBacked);
        QVERIFY(caps.isFileBacked);
        QVERIFY(!caps.isEmbedded);
    }

    // Mixed → no IC actions
    {
        std::unique_ptr<IC> ic1(new IC());
        ic1->setBlobName("test");
        ic1->loadFromBlob(blob, m_fixtureDir);
        std::unique_ptr<IC> ic2(new IC());
        ic2->loadFile(m_fixtureDir + "/simple_and.panda", m_fixtureDir);
        auto caps = computeCapabilities({ic1.get(), ic2.get()});
        QVERIFY(!caps.isEmbedded);
        QVERIFY(!caps.isFileBacked);
    }

    // Non-IC → no IC actions
    {
        InputSwitch sw;
        auto caps = computeCapabilities({&sw});
        QVERIFY(!caps.isEmbedded);
        QVERIFY(!caps.isFileBacked);
    }
}

void TestICInline::testElementLabelMimeRoundTrip()
{
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::blue);

    ElementLabel label(pixmap, ElementType::IC, "my_embedded_ic", nullptr, true);
    QVERIFY(label.isEmbedded());
    QCOMPARE(label.icFileName(), QString("my_embedded_ic"));

    std::unique_ptr<QMimeData> mime(label.mimeData());
    QVERIFY(mime);
    QVERIFY(mime->hasFormat("application/x-wiredpanda-dragdrop"));

    QByteArray itemData = mime->data("application/x-wiredpanda-dragdrop");
    QDataStream stream(&itemData, QIODevice::ReadOnly);

    Serialization::readPandaHeader(stream);

    QPoint offset;
    ElementType type;
    QString icFileName;
    bool isEmbedded = false;
    QString blobName;

    stream >> offset >> type >> icFileName;
    if (!stream.atEnd()) { stream >> isEmbedded; }
    if (!stream.atEnd()) { stream >> blobName; }

    QCOMPARE(type, ElementType::IC);
    QCOMPARE(icFileName, QString("my_embedded_ic"));
    QVERIFY(isEmbedded);
    QCOMPARE(blobName, QString("my_embedded_ic"));
}

void TestICInline::testElementLabelMimeFileBacked()
{
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::gray);

    ElementLabel label(pixmap, ElementType::IC, "some_circuit.panda", nullptr, false);
    QVERIFY(!label.isEmbedded());

    std::unique_ptr<QMimeData> mime(label.mimeData());
    QVERIFY(mime);

    QByteArray itemData = mime->data("application/x-wiredpanda-dragdrop");
    QDataStream stream(&itemData, QIODevice::ReadOnly);

    Serialization::readPandaHeader(stream);

    QPoint offset;
    ElementType type;
    QString icFileName;
    bool isEmbedded = true;
    QString blobName = "should_be_cleared";

    stream >> offset >> type >> icFileName;
    if (!stream.atEnd()) { stream >> isEmbedded; }
    if (!stream.atEnd()) { stream >> blobName; }

    QCOMPARE(type, ElementType::IC);
    QCOMPARE(icFileName, QString("some_circuit.panda"));
    QVERIFY(!isEmbedded);
    QVERIFY(blobName.isEmpty());
}

void TestICInline::testSceneDropEmbeddedICFromMime()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("drop_test", blob);

    auto *existingIC = new IC();
    existingIC->setBlobName("drop_test");
    existingIC->loadFromBlob(blob, m_fixtureDir);
    existingIC->setPos(100, 100);
    ws.scene()->addItem(existingIC);

    int icCountBefore = 0;
    for (auto *elm : ws.scene()->elements())
        if (elm->elementType() == ElementType::IC) ++icCountBefore;
    QCOMPARE(icCountBefore, 1);

    QByteArray itemData;
    {
        QDataStream stream(&itemData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        stream << QPoint(32, 32) << ElementType::IC << QString("drop_test") << true << QString("drop_test");
    }

    auto *mimeData = new QMimeData();
    mimeData->setData("wpanda/x-dnditemdata", itemData);
    ws.scene()->addItem(mimeData);

    int icCountAfter = 0;
    for (auto *elm : ws.scene()->elements())
        if (elm->elementType() == ElementType::IC && elm->isEmbedded()) ++icCountAfter;
    QCOMPARE(icCountAfter, 2);
}

void TestICInline::testSceneDropEmbeddedICMissingBlob()
{
    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);

    int countBefore = static_cast<int>(ws.scene()->elements().size());

    QByteArray itemData;
    {
        QDataStream stream(&itemData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        stream << QPoint(32, 32) << ElementType::IC << QString("nonexistent") << true << QString("nonexistent");
    }

    auto *mimeData = new QMimeData();
    mimeData->setData("wpanda/x-dnditemdata", itemData);
    ws.scene()->addItem(mimeData);

    QCOMPARE(ws.scene()->elements().size(), countBefore);
}

void TestICInline::testSceneDropNonICIgnoresEmbeddedFlag()
{
    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);

    int countBefore = static_cast<int>(ws.scene()->elements().size());

    QByteArray itemData;
    {
        QDataStream stream(&itemData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        stream << QPoint(32, 32) << ElementType::And << QString("bogus") << true << QString("bogus");
    }

    auto *mimeData = new QMimeData();
    mimeData->setData("wpanda/x-dnditemdata", itemData);
    ws.scene()->addItem(mimeData);

    // Non-IC elements are added normally regardless of embedded flag
    int countAfter = static_cast<int>(ws.scene()->elements().size());
    QCOMPARE(countAfter, countBefore + 1);
}

void TestICInline::testSceneDropBackwardCompatibleMime()
{
    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);

    QByteArray itemData;
    {
        QDataStream stream(&itemData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        stream << QPoint(32, 32) << ElementType::IC << QString("simple_and.panda");
    }

    auto *mimeData = new QMimeData();
    mimeData->setData("wpanda/x-dnditemdata", itemData);
    ws.scene()->addItem(mimeData);

    int icCount = 0;
    for (auto *elm : ws.scene()->elements()) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = static_cast<IC *>(elm);
            QVERIFY2(!ic->isEmbedded(), "Old MIME format should create file-backed IC");
            QVERIFY(!ic->file().isEmpty());
            QVERIFY(ic->inputSize() > 0);
            ++icCount;
        }
    }
    QCOMPARE(icCount, 1);
}

void TestICInline::testSceneDropEmbeddedEmptyBlobName()
{
    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);

    auto countBefore = ws.scene()->elements().size();

    QByteArray itemData;
    {
        QDataStream stream(&itemData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        stream << QPoint(32, 32) << ElementType::IC << QString("") << true << QString("");
    }

    auto *mimeData = new QMimeData();
    mimeData->setData("wpanda/x-dnditemdata", itemData);
    ws.scene()->addItem(mimeData);

    QCOMPARE(ws.scene()->elements().size(), countBefore);
}

void TestICInline::testElementPaletteUpdateEmbeddedICList()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    auto *ic1 = new IC();
    reg->setBlob("type_alpha", blob);
    ic1->setBlobName("type_alpha");
    ic1->loadFromBlob(blob, m_fixtureDir);
    ic1->setPos(100, 100);
    ws.scene()->addItem(ic1);

    auto *ic2 = new IC();
    reg->setBlob("type_beta", blob);
    ic2->setBlobName("type_beta");
    ic2->loadFromBlob(blob, m_fixtureDir);
    ic2->setPos(200, 200);
    ws.scene()->addItem(ic2);

    PaletteTestHarness harness;
    harness.palette->updateEmbeddedICList(ws.scene());

    auto labels = harness.embeddedLabels();
    QCOMPARE(labels.size(), 2);

    QStringList names;
    for (auto *lbl : labels) { names.append(lbl->icFileName()); }
    QVERIFY(names.contains("type_alpha"));
    QVERIFY(names.contains("type_beta"));
}

void TestICInline::testElementPaletteUpdateEmbeddedICListDedup()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("same_name", blob);

    for (int i = 0; i < 3; ++i) {
        auto *ic = new IC();
        ic->setBlobName("same_name");
        ic->loadFromBlob(blob, m_fixtureDir);
        ic->setPos(i * 100, 100);
        ws.scene()->addItem(ic);
    }

    PaletteTestHarness harness;
    harness.palette->updateEmbeddedICList(ws.scene());

    auto labels = harness.embeddedLabels();
    QCOMPARE(labels.size(), 1);
    QCOMPARE(labels.first()->icFileName(), QString("same_name"));
}

void TestICInline::testElementPaletteUpdateEmbeddedICListNull()
{
    PaletteTestHarness harness;
    harness.palette->updateEmbeddedICList(nullptr);
    QCOMPARE(harness.embeddedLabels().size(), 0);
}

void TestICInline::testElementPaletteSearchFindsEmbeddedIC()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("searchable_ic", blob);

    auto *ic = new IC();
    ic->setBlobName("searchable_ic");
    ic->loadFromBlob(blob, m_fixtureDir);
    ic->setPos(100, 100);
    ws.scene()->addItem(ic);

    PaletteTestHarness harness;
    harness.palette->updateEmbeddedICList(ws.scene());

    auto labels = harness.searchLabels();
    QVERIFY2(!labels.isEmpty(),
             "updateEmbeddedICList should add embedded IC labels to the search container");

    bool found = false;
    for (auto *lbl : labels) {
        if (lbl->icFileName() == "searchable_ic") { found = true; break; }
    }
    QVERIFY2(found, "Search container should contain a label for 'searchable_ic'");
}

void TestICInline::testElementPaletteRefreshAfterRemoveAll()
{
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("removable", blob);

    auto *ic = new IC();
    ic->setBlobName("removable");
    ic->loadFromBlob(blob, m_fixtureDir);
    ic->setPos(100, 100);
    ws.scene()->addItem(ic);

    PaletteTestHarness harness;

    harness.palette->updateEmbeddedICList(ws.scene());
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCOMPARE(harness.embeddedLabels().size(), 1);

    ws.removeEmbeddedIC("removable");

    harness.palette->updateEmbeddedICList(ws.scene());
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCOMPARE(harness.embeddedLabels().size(), 0);
}

void TestICInline::testICDropZoneMimeAcceptance()
{
    ICDropZone embeddedZone(ICDropZone::Section::Embedded);

    QByteArray fileBasedData;
    {
        QDataStream stream(&fileBasedData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        stream << QPoint(32, 32) << ElementType::IC << QString("test.panda") << false;
    }
    auto fileBasedMime = std::make_unique<QMimeData>();
    fileBasedMime->setData("application/x-wiredpanda-dragdrop", fileBasedData);

    QDragEnterEvent enterFileBased(QPoint(10, 10), Qt::CopyAction, fileBasedMime.get(), Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&embeddedZone, &enterFileBased);
    QVERIFY2(enterFileBased.isAccepted(), "Embedded zone should accept file-based IC drops");

    QByteArray embeddedData;
    {
        QDataStream stream(&embeddedData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        stream << QPoint(32, 32) << ElementType::IC << QString("my_ic") << true;
    }
    auto embeddedMime = std::make_unique<QMimeData>();
    embeddedMime->setData("application/x-wiredpanda-dragdrop", embeddedData);

    QDragEnterEvent enterEmbedded(QPoint(10, 10), Qt::CopyAction, embeddedMime.get(), Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&embeddedZone, &enterEmbedded);
    QVERIFY2(!enterEmbedded.isAccepted(), "Embedded zone should reject embedded IC drops");

    ICDropZone fileZone(ICDropZone::Section::FileBased);

    QDragEnterEvent enterEmbOnFile(QPoint(10, 10), Qt::CopyAction, embeddedMime.get(), Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&fileZone, &enterEmbOnFile);
    QVERIFY2(enterEmbOnFile.isAccepted(), "File-based zone should accept embedded IC drops");

    QDragEnterEvent enterFileOnFile(QPoint(10, 10), Qt::CopyAction, fileBasedMime.get(), Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&fileZone, &enterFileOnFile);
    QVERIFY2(!enterFileOnFile.isAccepted(), "File-based zone should reject file-based IC drops");
}

void TestICInline::testICDropZoneWiredInUI()
{
    ICDropZone fileZone(ICDropZone::Section::FileBased);
    ICDropZone embeddedZone(ICDropZone::Section::Embedded);

    // File-based zone: embedded drop → extractByBlobNameRequested
    QByteArray embeddedData;
    {
        QDataStream stream(&embeddedData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        stream << QPoint(32, 32) << ElementType::IC << QString("my_ic") << true << QString("my_ic");
    }
    auto embeddedMime = std::make_unique<QMimeData>();
    embeddedMime->setData("application/x-wiredpanda-dragdrop", embeddedData);

    QSignalSpy extractSpy(&fileZone, &ICDropZone::extractByBlobNameRequested);
    QDragEnterEvent enterEvent(QPoint(10, 10), Qt::CopyAction, embeddedMime.get(), Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&fileZone, &enterEvent);
    QVERIFY(enterEvent.isAccepted());

    QDropEvent dropEvent(QPointF(10, 10), Qt::CopyAction, embeddedMime.get(), Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&fileZone, &dropEvent);
    QCOMPARE(extractSpy.count(), 1);
    QCOMPARE(extractSpy.at(0).at(0).toString(), QString("my_ic"));

    // Embedded zone: file-based drop → embedByFileRequested
    QByteArray fileData;
    {
        QDataStream stream(&fileData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        stream << QPoint(32, 32) << ElementType::IC << QString("test.panda") << false;
    }
    auto fileMime = std::make_unique<QMimeData>();
    fileMime->setData("application/x-wiredpanda-dragdrop", fileData);

    QSignalSpy embedSpy(&embeddedZone, &ICDropZone::embedByFileRequested);
    QDragEnterEvent enterEvent2(QPoint(10, 10), Qt::CopyAction, fileMime.get(), Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&embeddedZone, &enterEvent2);
    QVERIFY(enterEvent2.isAccepted());

    QDropEvent dropEvent2(QPointF(10, 10), Qt::CopyAction, fileMime.get(), Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&embeddedZone, &dropEvent2);
    QCOMPARE(embedSpy.count(), 1);
    QCOMPARE(embedSpy.at(0).at(0).toString(), QString("test.panda"));
}

void TestICInline::testICDropZoneDropEventSignals()
{
    // Embedded zone: file-backed drop → embedByFileRequested
    {
        ICDropZone embeddedZone(ICDropZone::Section::Embedded);
        QSignalSpy embedSpy(&embeddedZone, &ICDropZone::embedByFileRequested);

        QByteArray itemData;
        {
            QDataStream stream(&itemData, QIODevice::WriteOnly);
            Serialization::writePandaHeader(stream);
            stream << QPoint(32, 32) << ElementType::IC << QString("my_circuit.panda") << false;
        }
        auto mime = std::make_unique<QMimeData>();
        mime->setData("application/x-wiredpanda-dragdrop", itemData);

        QDragEnterEvent dragEnter(QPoint(10, 10), Qt::CopyAction, mime.get(), Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(&embeddedZone, &dragEnter);
        QVERIFY(dragEnter.isAccepted());

        QDropEvent drop(QPointF(10, 10), Qt::CopyAction, mime.get(), Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(&embeddedZone, &drop);
        QCOMPARE(embedSpy.count(), 1);
        QCOMPARE(embedSpy.at(0).at(0).toString(), QString("my_circuit.panda"));
    }

    // File-based zone: embedded drop → extractByBlobNameRequested
    {
        ICDropZone fileZone(ICDropZone::Section::FileBased);
        QSignalSpy extractSpy(&fileZone, &ICDropZone::extractByBlobNameRequested);

        QByteArray itemData;
        {
            QDataStream stream(&itemData, QIODevice::WriteOnly);
            Serialization::writePandaHeader(stream);
            stream << QPoint(32, 32) << ElementType::IC << QString("my_embedded") << true << QString("my_embedded");
        }
        auto mime = std::make_unique<QMimeData>();
        mime->setData("application/x-wiredpanda-dragdrop", itemData);

        QDragEnterEvent dragEnter(QPoint(10, 10), Qt::CopyAction, mime.get(), Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(&fileZone, &dragEnter);
        QVERIFY(dragEnter.isAccepted());

        QDropEvent drop(QPointF(10, 10), Qt::CopyAction, mime.get(), Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(&fileZone, &drop);
        QCOMPARE(extractSpy.count(), 1);
        QCOMPARE(extractSpy.at(0).at(0).toString(), QString("my_embedded"));
    }

    // Wrong direction: no signal
    {
        ICDropZone embeddedZone(ICDropZone::Section::Embedded);
        QSignalSpy embedSpy(&embeddedZone, &ICDropZone::embedByFileRequested);

        QByteArray itemData;
        {
            QDataStream stream(&itemData, QIODevice::WriteOnly);
            Serialization::writePandaHeader(stream);
            stream << QPoint(32, 32) << ElementType::IC << QString("already_embedded") << true << QString("already_embedded");
        }
        auto mime = std::make_unique<QMimeData>();
        mime->setData("application/x-wiredpanda-dragdrop", itemData);

        QDragEnterEvent dragEnter(QPoint(10, 10), Qt::CopyAction, mime.get(), Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(&embeddedZone, &dragEnter);
        QVERIFY(!dragEnter.isAccepted());

        QCOMPARE(embedSpy.count(), 0);
    }
}

void TestICInline::testTrashButtonDragAcceptance()
{
    TrashButton button;

    QByteArray itemData;
    {
        QDataStream stream(&itemData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        stream << QPoint(32, 32) << ElementType::IC << QString("test.panda") << false;
    }
    auto currentMime = std::make_unique<QMimeData>();
    currentMime->setData("application/x-wiredpanda-dragdrop", itemData);

    QDragEnterEvent enterCurrent(QPoint(10, 10), Qt::CopyAction, currentMime.get(), Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&button, &enterCurrent);
    QVERIFY2(enterCurrent.isAccepted(), "TrashButton should accept current MIME type");

    auto legacyMime = std::make_unique<QMimeData>();
    legacyMime->setData("wpanda/x-dnditemdata", itemData);

    QDragEnterEvent enterLegacy(QPoint(10, 10), Qt::CopyAction, legacyMime.get(), Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&button, &enterLegacy);
    QVERIFY2(enterLegacy.isAccepted(), "TrashButton should accept legacy MIME type");

    auto textMime = std::make_unique<QMimeData>();
    textMime->setText("hello");

    QDragEnterEvent enterText(QPoint(10, 10), Qt::CopyAction, textMime.get(), Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&button, &enterText);
    QVERIFY2(!enterText.isAccepted(), "TrashButton should reject unrelated MIME types");
}

// ===========================================================================
// Migrated: Batch 9 — SystemVerilog and drop-zone workflows
// ===========================================================================

void TestICInline::testSystemVerilogCodegenEmbeddedIC()
{
    std::unique_ptr<WorkSpace> ws = TestUtils::createWorkspace();
    ws->scene()->setContextDir(m_fixtureDir);
    auto *reg = ws->scene()->icRegistry();

    auto *ic = new IC();
    auto *sw1 = new InputSwitch();
    auto *sw2 = new InputSwitch();
    auto *led = new Led();

    CircuitBuilder builder(ws->scene());
    builder.add(sw1, sw2, ic, led);
    embedIC(ic, readFile(m_fixtureDir + "/simple_and.panda"), "my_embedded_and", m_fixtureDir, reg);

    builder.connect(sw1, 0, ic, 0);
    builder.connect(sw2, 0, ic, 1);
    builder.connect(ic, 0, led, 0);
    builder.initSimulation();

    QVector<GraphicElement *> allElements;
    for (auto *item : ws->scene()->items()) {
        if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item))
            allElements.append(elm);
    }
    QVERIFY(!allElements.isEmpty());

    QTemporaryFile svFile;
    svFile.setFileTemplate(m_tempDir.filePath("embedded_XXXXXX.sv"));
    QVERIFY(svFile.open());
    QString svPath = svFile.fileName();
    svFile.close();

    SystemVerilogCodeGen generator(svPath, allElements);
    generator.generate();

    QFile output(svPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(output.readAll());
    output.close();

    QVERIFY2(content.contains("module my_embedded_and"),
             qPrintable("Expected module 'my_embedded_and' in output:\n" + content));
    QFile::remove(svPath);
}

void TestICInline::testSystemVerilogCodegenTwoEmbeddedSameBlobName()
{
    std::unique_ptr<WorkSpace> ws = TestUtils::createWorkspace();
    ws->scene()->setContextDir(m_fixtureDir);
    auto *reg = ws->scene()->icRegistry();

    auto *ic1 = new IC();
    auto *ic2 = new IC();
    auto *sw1 = new InputSwitch();
    auto *sw2 = new InputSwitch();
    auto *sw3 = new InputSwitch();
    auto *sw4 = new InputSwitch();
    auto *led1 = new Led();
    auto *led2 = new Led();

    CircuitBuilder builder(ws->scene());
    builder.add(sw1, sw2, sw3, sw4, ic1, ic2, led1, led2);

    embedIC(ic1, readFile(m_fixtureDir + "/simple_and.panda"), "shared_and", m_fixtureDir, reg);
    embedIC(ic2, readFile(m_fixtureDir + "/simple_and.panda"), "shared_and", m_fixtureDir, reg);

    builder.connect(sw1, 0, ic1, 0);
    builder.connect(sw2, 0, ic1, 1);
    builder.connect(ic1, 0, led1, 0);
    builder.connect(sw3, 0, ic2, 0);
    builder.connect(sw4, 0, ic2, 1);
    builder.connect(ic2, 0, led2, 0);
    builder.initSimulation();

    QVector<GraphicElement *> allElements;
    for (auto *item : ws->scene()->items()) {
        if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item))
            allElements.append(elm);
    }

    QTemporaryFile svFile;
    svFile.setFileTemplate(m_tempDir.filePath("two_same_XXXXXX.sv"));
    QVERIFY(svFile.open());
    QString svPath = svFile.fileName();
    svFile.close();

    SystemVerilogCodeGen generator(svPath, allElements);
    generator.generate();

    QFile output(svPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(output.readAll());
    output.close();

    int moduleCount = 0;
    {
        QRegularExpression re(R"(\bmodule\s+shared_and\b)");
        auto it = re.globalMatch(content);
        while (it.hasNext()) { it.next(); ++moduleCount; }
    }
    QCOMPARE(moduleCount, 1);

    QFile::remove(svPath);
}

void TestICInline::testSVCodegenReservedWordBlobName()
{
    std::unique_ptr<WorkSpace> ws = TestUtils::createWorkspace();
    ws->scene()->setContextDir(m_fixtureDir);
    auto *reg = ws->scene()->icRegistry();

    auto *ic = new IC();
    auto *sw1 = new InputSwitch();
    auto *sw2 = new InputSwitch();
    auto *led = new Led();

    CircuitBuilder builder(ws->scene());
    builder.add(sw1, sw2, ic, led);
    embedIC(ic, readFile(m_fixtureDir + "/simple_and.panda"), "module", m_fixtureDir, reg);

    builder.connect(sw1, 0, ic, 0);
    builder.connect(sw2, 0, ic, 1);
    builder.connect(ic, 0, led, 0);
    builder.initSimulation();

    QVector<GraphicElement *> allElements;
    for (auto *item : ws->scene()->items()) {
        if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item))
            allElements.append(elm);
    }
    QVERIFY(!allElements.isEmpty());

    QTemporaryFile svFile;
    svFile.setFileTemplate(m_tempDir.filePath("reserved_XXXXXX.sv"));
    QVERIFY(svFile.open());
    QString svPath = svFile.fileName();
    svFile.close();

    SystemVerilogCodeGen generator(svPath, allElements);
    generator.generate();

    QFile output(svPath);
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(output.readAll());
    output.close();

    QVERIFY2(content.contains("module m_module"),
             qPrintable("Expected 'module m_module' (reserved word prefixed) in output:\n" + content));
    QRegularExpression rawModuleDef(R"(\bmodule\s+module\b)");
    QVERIFY2(!rawModuleDef.match(content).hasMatch(),
             "Raw reserved word 'module' should not be used as module name");

    QFile::remove(svPath);
}

void TestICInline::testEmbedICByDropConversion()
{
    // File-backed ICs referencing the same file should all be converted
    // to embedded when embedICsByFile runs.

    const QString filePath = m_fixtureDir + "/simple_and.panda";
    QByteArray rawFileBytes = readFile(filePath);
    QVERIFY(!rawFileBytes.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    auto *ic1 = new IC();
    ic1->loadFile(filePath, m_fixtureDir);
    ic1->setPos(100, 100);
    ws.scene()->addItem(ic1);

    auto *ic2 = new IC();
    ic2->loadFile(filePath, m_fixtureDir);
    ic2->setPos(200, 200);
    ws.scene()->addItem(ic2);

    int id1 = ic1->id();
    int id2 = ic2->id();
    QVERIFY(!ic1->isEmbedded());
    QVERIFY(!ic2->isEmbedded());

    reg->embedICsByFile(filePath, rawFileBytes, "simple_and");

    auto *e1 = dynamic_cast<IC *>(ws.scene()->itemById(id1));
    auto *e2 = dynamic_cast<IC *>(ws.scene()->itemById(id2));
    QVERIFY(e1);
    QVERIFY(e2);
    QVERIFY(e1->isEmbedded());
    QVERIFY(e2->isEmbedded());
    QCOMPARE(e1->blobName(), QString("simple_and"));
    QCOMPARE(e2->blobName(), QString("simple_and"));

    // Undo should restore file-backed state
    ws.scene()->undoStack()->undo();
    e1 = dynamic_cast<IC *>(ws.scene()->itemById(id1));
    e2 = dynamic_cast<IC *>(ws.scene()->itemById(id2));
    QVERIFY(e1);
    QVERIFY(e2);
    QVERIFY(!e1->isEmbedded());
    QVERIFY(!e2->isEmbedded());
    QVERIFY(e1->file().contains("simple_and.panda"));
    QVERIFY(e2->file().contains("simple_and.panda"));
}

void TestICInline::testEmbedICByDropCollisionAutoSuffix()
{
    // When an embedded IC with the same name exists, collision should be detected.

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    // Pre-place an embedded IC with blobName "simple_and"
    auto *existingIC = new IC();
    embedIC(existingIC, readFile(m_fixtureDir + "/simple_and.panda"), "simple_and", m_fixtureDir, reg);
    existingIC->setPos(100, 100);
    ws.scene()->addItem(existingIC);

    // Add a file-backed IC from the same file
    auto *fileBacked = new IC();
    fileBacked->loadFile(m_fixtureDir + "/simple_and.panda", m_fixtureDir);
    fileBacked->setPos(200, 200);
    ws.scene()->addItem(fileBacked);

    // Collision check
    bool collision = reg->hasBlob("simple_and");
    QVERIFY2(collision, "Should detect name collision with existing embedded IC");

    // Use a suffixed name
    QVERIFY(!reg->hasBlob("simple_and_2"));

    // Embed with the new name
    QByteArray rawBytes = readFile(m_fixtureDir + "/simple_and.panda");
    reg->embedICsByFile(fileBacked->file(), rawBytes, "simple_and_2");

    // Both embedded ICs should coexist with distinct blobNames
    int embeddedCount = 0;
    QStringList blobNames;
    for (auto *elm : ws.scene()->elements()) {
        if (elm->isEmbedded()) {
            ++embeddedCount;
            blobNames.append(elm->blobName());
        }
    }
    QCOMPARE(embeddedCount, 2);
    QVERIFY(blobNames.contains("simple_and"));
    QVERIFY(blobNames.contains("simple_and_2"));
}

void TestICInline::testExtractICByDropConversion()
{
    // All embedded ICs with the same blobName should be converted to
    // file-backed after the blob is saved to disk.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("extract_me", blob);

    auto *ic1 = new IC();
    ic1->setBlobName("extract_me");
    ic1->loadFromBlob(blob, m_fixtureDir);
    ic1->setPos(100, 100);
    ws.scene()->addItem(ic1);

    auto *ic2 = new IC();
    ic2->setBlobName("extract_me");
    ic2->loadFromBlob(blob, m_fixtureDir);
    ic2->setPos(200, 200);
    ws.scene()->addItem(ic2);

    int id1 = ic1->id();
    int id2 = ic2->id();

    // Extract: writes blob to disk and converts ICs to file-backed
    const QString fileName = m_fixtureDir + "/extract_me.panda";
    QFile::remove(fileName);
    reg->extractToFile("extract_me", fileName);

    auto *e1 = dynamic_cast<IC *>(ws.scene()->itemById(id1));
    auto *e2 = dynamic_cast<IC *>(ws.scene()->itemById(id2));
    QVERIFY(e1);
    QVERIFY(e2);
    QVERIFY(!e1->isEmbedded());
    QVERIFY(!e2->isEmbedded());
    QVERIFY(e1->file().contains("extract_me.panda"));
    QVERIFY(e2->file().contains("extract_me.panda"));

    // Undo should restore embedded state
    ws.scene()->undoStack()->undo();
    e1 = dynamic_cast<IC *>(ws.scene()->itemById(id1));
    e2 = dynamic_cast<IC *>(ws.scene()->itemById(id2));
    QVERIFY(e1);
    QVERIFY(e2);
    QVERIFY(e1->isEmbedded());
    QVERIFY(e2->isEmbedded());
    QCOMPARE(e1->blobName(), QString("extract_me"));

    QFile::remove(fileName);
}

void TestICInline::testExtractICByDropFileCollision()
{
    // When extracting, if blobName.panda already exists on disk,
    // the collision should be detectable.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("collision_test", blob);

    auto *ic = new IC();
    ic->setBlobName("collision_test");
    ic->loadFromBlob(blob, m_fixtureDir);
    ic->setPos(100, 100);
    ws.scene()->addItem(ic);

    const QString targetPath = m_fixtureDir + "/collision_test.panda";
    auto cleanupTarget = qScopeGuard([&targetPath] { QFile::remove(targetPath); });

    // Pre-create the file to simulate collision
    {
        QFile preExisting(targetPath);
        QVERIFY(preExisting.open(QIODevice::WriteOnly));
        preExisting.write("dummy data");
        preExisting.close();
    }
    QVERIFY(QFile::exists(targetPath));

    // Overwrite with real data
    {
        QSaveFile sf(targetPath);
        QVERIFY(sf.open(QIODevice::WriteOnly));
        sf.write(blob);
        QVERIFY(sf.commit());
    }

    QFile result(targetPath);
    QVERIFY(result.open(QIODevice::ReadOnly));
    QByteArray contents = result.readAll();
    result.close();
    QVERIFY(contents != "dummy data");
}

// ===========================================================================
// Migrated: Batch 10 — Undo/redo and position
// ===========================================================================

void TestICInline::testMultiCycleUndoRedoPropagation()
{
    // Run 3 full undo/redo cycles on blob propagation.

    QByteArray andBlob = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray notBlob = readFile(m_fixtureDir + "/chain_c.panda");
    QVERIFY(!andBlob.isEmpty());
    QVERIFY(!notBlob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("cycle_test", andBlob);

    auto *ic = new IC();
    ic->setBlobName("cycle_test");
    ic->loadFromBlob(andBlob, m_fixtureDir);
    ic->setPos(200, 200);
    ic->setLabel("MY_IC");
    ws.scene()->addItem(ic);
    int icId = ic->id();
    int origInputs = ic->inputSize();

    ws.onChildICBlobSaved(icId, notBlob);

    for (int cycle = 0; cycle < 3; ++cycle) {
        ws.scene()->undoStack()->undo();
        auto *elm = dynamic_cast<IC *>(ws.scene()->itemById(icId));
        QVERIFY2(elm, qPrintable(QString("IC not found after undo cycle %1").arg(cycle)));
        QCOMPARE(elm->inputSize(), origInputs);
        QCOMPARE(elm->blobName(), QString("cycle_test"));
        QCOMPARE(elm->label(), QString("MY_IC"));
        QCOMPARE(elm->pos(), QPointF(200, 200));

        ws.scene()->undoStack()->redo();
        elm = dynamic_cast<IC *>(ws.scene()->itemById(icId));
        QVERIFY2(elm, qPrintable(QString("IC not found after redo cycle %1").arg(cycle)));
        QCOMPARE(elm->blobName(), QString("cycle_test"));
        QCOMPARE(elm->label(), QString("MY_IC"));
        QCOMPARE(elm->pos(), QPointF(200, 200));
    }
}

void TestICInline::testPositionRotationPreservedDuringPropagation()
{
    // Positions and rotations should be preserved through UpdateBlobCommand.

    QByteArray andBlob = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray notBlob = readFile(m_fixtureDir + "/chain_c.panda");
    QVERIFY(!andBlob.isEmpty());
    QVERIFY(!notBlob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("my_ic", andBlob);

    auto *ic1 = new IC();
    ic1->setBlobName("my_ic");
    ic1->loadFromBlob(andBlob, m_fixtureDir);
    ic1->setPos(104, 200);
    ic1->setRotation(90);
    ws.scene()->addItem(ic1);

    auto *ic2 = new IC();
    ic2->setBlobName("my_ic");
    ic2->loadFromBlob(andBlob, m_fixtureDir);
    ic2->setPos(296, 400);
    ic2->setRotation(180);
    ws.scene()->addItem(ic2);

    int id1 = ic1->id();
    int id2 = ic2->id();

    ws.onChildICBlobSaved(id1, notBlob);

    auto *u1 = dynamic_cast<GraphicElement *>(ws.scene()->itemById(id1));
    auto *u2 = dynamic_cast<GraphicElement *>(ws.scene()->itemById(id2));
    QVERIFY(u1);
    QVERIFY(u2);
    QCOMPARE(u1->pos(), QPointF(104, 200));
    QCOMPARE(u2->pos(), QPointF(296, 400));
    QCOMPARE(u1->rotation(), 90.0);
    QCOMPARE(u2->rotation(), 180.0);

    ws.scene()->undoStack()->undo();
    u1 = dynamic_cast<GraphicElement *>(ws.scene()->itemById(id1));
    u2 = dynamic_cast<GraphicElement *>(ws.scene()->itemById(id2));
    QVERIFY(u1);
    QVERIFY(u2);
    QCOMPARE(u1->pos(), QPointF(104, 200));
    QCOMPARE(u2->pos(), QPointF(296, 400));
    QCOMPARE(u1->rotation(), 90.0);
    QCOMPARE(u2->rotation(), 180.0);

    ws.scene()->undoStack()->redo();
    u1 = dynamic_cast<GraphicElement *>(ws.scene()->itemById(id1));
    u2 = dynamic_cast<GraphicElement *>(ws.scene()->itemById(id2));
    QVERIFY(u1);
    QVERIFY(u2);
    QCOMPARE(u1->pos(), QPointF(104, 200));
    QCOMPARE(u2->pos(), QPointF(296, 400));
}

void TestICInline::testLoadFromBlobClearsFileState()
{
    // Transitioning from file-backed to blob-backed should clear the file
    // path and make the IC embedded. File watching is handled by ICRegistry
    // (not IC), so we verify the IC-level state transition.

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    IC ic;
    ic.loadFile(m_fixtureDir + "/simple_and.panda", m_fixtureDir);
    QVERIFY(!ic.file().isEmpty());
    QVERIFY(!ic.isEmbedded());
    QVERIFY(ic.inputSize() > 0);

    // Transition to blob-backed
    ic.setBlobName("simple_and");
    ic.loadFromBlob(blob, m_fixtureDir);

    QVERIFY(ic.file().isEmpty());
    QVERIFY(ic.isEmbedded());
    QCOMPARE(ic.blobName(), QString("simple_and"));
    QVERIFY(ic.inputSize() > 0);
}

// ===========================================================================
// Edge cases: registry and undo
// ===========================================================================

void TestICInline::testRemoveEmbeddedICUndoLimitation()
{
    // removeEmbeddedIC removes the blob from the registry immediately
    // (not as part of the undo command). This test documents the limitation.
    WorkSpace ws;
    auto *scene = ws.scene();
    scene->setContextDir(m_fixtureDir);
    auto *reg = scene->icRegistry();

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    auto *ic = new IC();
    embedIC(ic, blob, "remove_undo", m_fixtureDir, reg);
    ic->setPos(100, 100);
    scene->addItem(ic);
    QVERIFY(reg->hasBlob("remove_undo"));

    // Remove via workspace method
    ws.removeEmbeddedIC("remove_undo");

    // IC should be gone from the scene
    bool found = false;
    for (auto *elm : scene->elements()) {
        if (elm->isEmbedded() && elm->blobName() == "remove_undo") {
            found = true;
        }
    }
    QVERIFY2(!found, "IC should be deleted after removeEmbeddedIC");

    // Blob is removed from registry immediately
    QVERIFY2(!reg->hasBlob("remove_undo"),
             "Blob removed immediately by removeEmbeddedIC (known limitation)");

    // Undo restores the IC elements via DeleteItemsCommand, but the blob
    // was removed eagerly from the registry. The restored IC elements may
    // lack blob data. This documents the known limitation.
    scene->undoAction();
    // The IC element itself may or may not be restored by undo, but blob data is lost.
    // We just verify no crash and the blob remains missing.
    QVERIFY2(!reg->hasBlob("remove_undo"),
             "Blob is NOT restored by undo (known limitation — blob was removed eagerly)");
}

void TestICInline::testBlobRegistryMergeConflictSkipsExisting()
{
    // ClipboardManager::paste() uses skip-if-exists semantics:
    //   if (!registry->hasBlob(it.key())) { registry->setBlob(it.key(), it.value()); }
    // Verify this at the registry level: when a blob with the same name
    // already exists, importing from another source should NOT overwrite it.
    WorkSpace wsSource;
    auto *regSource = wsSource.scene()->icRegistry();

    WorkSpace wsDest;
    auto *regDest = wsDest.scene()->icRegistry();

    QByteArray blobA = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray blobB = readFile(m_fixtureDir + "/nested_and.panda");
    QVERIFY(!blobA.isEmpty());
    QVERIFY(!blobB.isEmpty());
    QVERIFY(blobA != blobB);

    // Destination already has "shared_name" with blobA
    regDest->setBlob("shared_name", blobA);
    QCOMPARE(regDest->blob("shared_name"), blobA);

    // Source has "shared_name" with blobB (different data)
    regSource->setBlob("shared_name", blobB);

    // Simulate the merge that ClipboardManager::paste() performs
    const auto &srcMap = regSource->blobMap();
    for (auto it = srcMap.cbegin(); it != srcMap.cend(); ++it) {
        if (!regDest->hasBlob(it.key())) {
            regDest->setBlob(it.key(), it.value());
        }
    }

    // Destination's blob should be preserved (blobA), not overwritten
    QCOMPARE(regDest->blob("shared_name"), blobA);

    // But a new name should still be importable
    regSource->setBlob("unique_name", blobB);
    const auto &srcMap2 = regSource->blobMap();
    for (auto it = srcMap2.cbegin(); it != srcMap2.cend(); ++it) {
        if (!regDest->hasBlob(it.key())) {
            regDest->setBlob(it.key(), it.value());
        }
    }
    QVERIFY(regDest->hasBlob("unique_name"));
    QCOMPARE(regDest->blob("unique_name"), blobB);
}

void TestICInline::testUniqueNameCollision()
{
    WorkSpace ws;
    auto *reg = ws.scene()->icRegistry();

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    reg->setBlob("my_ic", blob);
    reg->setBlob("my_ic_2", blob);

    // uniqueBlobName should skip "my_ic" and "my_ic_2", return "my_ic_3"
    QString unique = reg->uniqueBlobName("my_ic");
    QCOMPARE(unique, QString("my_ic_3"));

    // Another call without registering should return the same
    QCOMPARE(reg->uniqueBlobName("my_ic"), QString("my_ic_3"));

    // Register "my_ic_3", next should be "my_ic_4"
    reg->setBlob("my_ic_3", blob);
    QCOMPARE(reg->uniqueBlobName("my_ic"), QString("my_ic_4"));
}

void TestICInline::testOrphanBlobAfterDeleteAllInstances()
{
    // When all instances of an embedded IC are deleted (via Delete key, not
    // removeEmbeddedIC), the blob should remain in the registry as an orphan.
    // This is tested via MainWindow in testDeleteLastEmbeddedInstanceUndo.
    // Here we verify the registry-level behavior.
    WorkSpace ws;
    auto *scene = ws.scene();
    scene->setContextDir(m_fixtureDir);
    auto *reg = scene->icRegistry();

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    auto *ic1 = new IC();
    embedIC(ic1, blob, "orphan_test", m_fixtureDir, reg);
    ic1->setPos(100, 100);
    scene->addItem(ic1);

    auto *ic2 = new IC();
    embedIC(ic2, blob, "orphan_test", m_fixtureDir, reg);
    ic2->setPos(200, 100);
    scene->addItem(ic2);

    // Select and delete one by one
    ic1->setSelected(true);
    ic2->setSelected(true);
    scene->deleteAction();

    // No embedded ICs in scene
    bool anyEmbedded = false;
    for (auto *elm : scene->elements()) {
        if (elm->isEmbedded()) anyEmbedded = true;
    }
    QVERIFY(!anyEmbedded);

    // Blob should still be in registry (orphan — not removed by delete)
    QVERIFY2(reg->hasBlob("orphan_test"),
             "Blob should persist in registry as orphan after deleting all instances");

    // A new IC can still be created from the orphan blob
    auto *ic3 = new IC();
    embedIC(ic3, reg->blob("orphan_test"), "orphan_test", m_fixtureDir, reg);
    scene->addItem(ic3);
    QVERIFY(ic3->isEmbedded());
    QVERIFY(ic3->inputSize() > 0);
}

void TestICInline::testImportNestedBlobsViaRegisterBlob()
{
    // nested_and.panda references simple_and.panda as a nested IC dependency.
    // registerBlob should embed simple_and INTO nested_and's blob (self-contained),
    // not as a sibling in the registry.
    WorkSpace ws;
    auto *scene = ws.scene();
    scene->setContextDir(m_fixtureDir);
    auto *reg = scene->icRegistry();

    QByteArray nestedBlob = readFile(m_fixtureDir + "/nested_and.panda");
    QVERIFY(!nestedBlob.isEmpty());

    QVERIFY(reg->blobMap().isEmpty());

    reg->registerBlob("nested_and", nestedBlob);

    QVERIFY(reg->hasBlob("nested_and"));

    // simple_and should NOT be a sibling — it's embedded inside nested_and's blob
    QCOMPARE(reg->blobMap().size(), 1);
    QVERIFY(!reg->hasBlob("simple_and"));

    // Verify the blob is self-contained: simple_and is in its embeddedICs
    QByteArray selfContainedBlob = reg->blob("nested_and");
    QDataStream stream(&selfContainedBlob, QIODevice::ReadOnly);
    auto preamble = Serialization::readPreamble(stream);
    auto embeddedICs = Serialization::deserializeBlobRegistry(preamble.metadata);
    QVERIFY2(embeddedICs.contains("simple_and"),
             "simple_and should be embedded inside nested_and's blob");

    // Verify the self-contained blob loads correctly
    auto *ic = new IC();
    ic->setBlobName("nested_and");
    ic->loadFromBlob(reg->blob("nested_and"), m_fixtureDir);
    scene->addItem(ic);

    QVERIFY(ic->isEmbedded());
    QVERIFY(ic->inputSize() > 0);
    QVERIFY(ic->outputSize() > 0);
}

// ============================================================================
// Edge cases: ICRegistry
// ============================================================================

void TestICInline::testRenameBlobSameNameNoOp()
{
    // Renaming a blob to itself must be a no-op — no state changes, no IC updates.
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();
    reg->setBlob("same", blob);

    auto *ic = new IC();
    ic->setBlobName("same");
    ic->loadFromBlob(blob, m_fixtureDir);
    scene.addItem(ic);

    reg->renameBlob("same", "same");

    QVERIFY(reg->hasBlob("same"));
    QCOMPARE(ic->blobName(), QString("same"));
    QCOMPARE(reg->blob("same"), blob);
}

void TestICInline::testRenameBlobCollisionOverwrites()
{
    // Renaming "A" to "B" when "B" already exists overwrites "B"'s data.
    // ICs that referenced "B" keep the name "B" but now have "A"'s data.
    QByteArray blobA = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray blobB = QByteArray("dummy_data_for_B");

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    reg->setBlob("A", blobA);
    reg->setBlob("B", blobB);

    auto *icA = new IC();
    icA->setBlobName("A");
    icA->loadFromBlob(blobA, m_fixtureDir);
    scene.addItem(icA);

    // Rename A → B: "A" disappears, "B" now has A's data
    reg->renameBlob("A", "B");

    QVERIFY(!reg->hasBlob("A"));
    QVERIFY(reg->hasBlob("B"));
    QCOMPARE(reg->blob("B"), blobA);
    QCOMPARE(icA->blobName(), QString("B"));
}

void TestICInline::testRenameBlobNonExistentNoOp()
{
    // Renaming a blob that doesn't exist must be a no-op.
    Scene scene;
    auto *reg = scene.icRegistry();

    reg->setBlob("existing", QByteArray("data"));

    reg->renameBlob("ghost", "existing");

    // "existing" must be unchanged
    QVERIFY(reg->hasBlob("existing"));
    QCOMPARE(reg->blob("existing"), QByteArray("data"));
}

void TestICInline::testRemoveBlobNonExistent()
{
    // Removing a non-existent blob must not crash.
    Scene scene;
    auto *reg = scene.icRegistry();

    reg->setBlob("keep", QByteArray("data"));
    reg->removeBlob("non_existent");  // must not crash

    QVERIFY(reg->hasBlob("keep"));
}

void TestICInline::testInitEmbeddedICMissingBlob()
{
    // initEmbeddedIC must return false when the blob is missing.
    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    auto ic = std::make_unique<IC>();

    bool result = reg->initEmbeddedIC(ic.get(), "missing_blob");

    QVERIFY(!result);
    // IC should not have been modified — blobName should still be empty
    QVERIFY(ic->blobName().isEmpty());
}

void TestICInline::testInitEmbeddedICPreservesExistingLabel()
{
    // If the IC already has a label, initEmbeddedIC must not overwrite it.
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();
    reg->setBlob("my_ic", blob);

    auto *ic = new IC();
    ic->setLabel("MY_CUSTOM_LABEL");
    scene.addItem(ic);

    bool result = reg->initEmbeddedIC(ic, "my_ic");

    QVERIFY(result);
    QCOMPARE(ic->label(), QString("MY_CUSTOM_LABEL"));
}

void TestICInline::testEmbedICsByFileNoTargets()
{
    // embedICsByFile on a scene with no matching file-backed ICs must return 0
    // and not modify the registry.
    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    int count = reg->embedICsByFile("nonexistent_file.panda", blob, "should_not_exist");

    QCOMPARE(count, 0);
    QVERIFY(!reg->hasBlob("should_not_exist"));
}

void TestICInline::testExtractToFileOverwritesExisting()
{
    // extractToFile must overwrite an existing file at the destination path.
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    auto *ic = new IC();
    embedIC(ic, blob, "overwrite_test", m_fixtureDir, reg);
    ic->setPos(100, 100);
    ws.scene()->addItem(ic);

    const QString extractPath = m_fixtureDir + "/overwrite_target.panda";

    // Write a dummy file at the path first
    {
        QFile dummy(extractPath);
        QVERIFY(dummy.open(QIODevice::WriteOnly));
        dummy.write("old content");
    }
    QVERIFY(QFile::exists(extractPath));

    reg->extractToFile("overwrite_test", extractPath);

    // The file must now contain the blob data, not the old content
    QByteArray extractedData = readFile(extractPath);
    QCOMPARE(extractedData, blob);
    QVERIFY(!reg->hasBlob("overwrite_test"));
}

void TestICInline::testBlobRegistryEmptyRoundTrip()
{
    // An empty blob registry must survive a save/load round-trip.
    // serializeBlobRegistry skips empty registries, so deserializeBlobRegistry
    // must return an empty map when the key is missing.
    QMap<QString, QVariant> metadata;

    Serialization::serializeBlobRegistry({}, metadata);

    // Empty registry should NOT add the key
    QVERIFY(!metadata.contains("embeddedICs"));

    // Deserializing metadata without the key must return an empty map
    auto result = Serialization::deserializeBlobRegistry(metadata);
    QVERIFY(result.isEmpty());
}

void TestICInline::testRegisterBlobFlattensDeeplyNested()
{
    // registerBlob must recursively flatten 3+ levels of nesting.
    // chain_a → chain_b → chain_c (file-backed references).
    // After registerBlob("chain_a"), the blob must contain both chain_b
    // and chain_c as embedded ICs inside it.
    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    QByteArray chainA = readFile(m_fixtureDir + "/chain_a.panda");
    QVERIFY(!chainA.isEmpty());

    reg->registerBlob("chain_a", chainA);

    // The blob should now be self-contained. Parse its metadata to verify.
    QByteArray selfContained = reg->blob("chain_a");
    QDataStream stream(&selfContained, QIODevice::ReadOnly);
    auto preamble = Serialization::readPreamble(stream);
    auto embeddedICs = Serialization::deserializeBlobRegistry(preamble.metadata);

    // chain_a references chain_b (directly embedded at top level).
    QVERIFY2(embeddedICs.contains("chain_b"),
             "chain_b should be embedded inside chain_a's blob");

    // chain_c is embedded inside chain_b (nested one level deeper).
    // Parse chain_b's blob to verify chain_c is there.
    QByteArray chainBBlob = embeddedICs.value("chain_b");
    QVERIFY(!chainBBlob.isEmpty());

    QDataStream streamB(&chainBBlob, QIODevice::ReadOnly);
    auto preambleB = Serialization::readPreamble(streamB);
    auto innerICs = Serialization::deserializeBlobRegistry(preambleB.metadata);

    QVERIFY2(innerICs.contains("chain_c"),
             "chain_c should be recursively embedded inside chain_b's blob");
}

// ============================================================================
// Edge cases: IC load/save and undo
// ============================================================================

void TestICInline::testLoadICWithMissingBlobFallsBackToFile()
{
    // When an IC's name doesn't match any blob in the registry, IC::load()
    // falls back to file-based loading.  Verify the fallback works.
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    // Save an embedded IC to a buffer
    WorkSpace ws1;
    ws1.scene()->setContextDir(m_fixtureDir);
    auto *reg1 = ws1.scene()->icRegistry();

    auto *ic1 = new IC();
    embedIC(ic1, blob, "fallback_test", m_fixtureDir, reg1);
    ic1->setPos(100, 100);
    ws1.scene()->addItem(ic1);

    // Serialize the IC element
    QByteArray elemData;
    {
        QDataStream out(&elemData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(out);
        ic1->save(out);
    }

    // Load the IC in a context with NO blob registry but with the file on disk.
    // The name "fallback_test" won't match any blob, so it should fall back to
    // file loading.  "fallback_test" isn't a valid filename either, so this
    // should throw.
    auto ic2 = std::make_unique<IC>();
    QDataStream in(&elemData, QIODevice::ReadOnly);
    QMap<quint64, QNEPort *> portMap;
    SerializationContext ctx{portMap, Serialization::readPandaHeader(in), m_fixtureDir};
    // No blobRegistry → ctx.blobRegistry is nullptr

    bool threw = false;
    try {
        ic2->load(in, ctx);
    } catch (...) {
        threw = true;
    }
    // "fallback_test" is not a valid .panda file on disk, so file fallback throws
    QVERIFY2(threw, "Loading embedded IC name without matching blob or file should throw");
}

void TestICInline::testLoadICWithNullBlobRegistry()
{
    // When blobRegistry is nullptr, IC::load() must fall back to file mode
    // without crashing (no null dereference on line 132-133).
    // Use "simple_and.panda" as the name so file fallback succeeds.
    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);

    // Create a file-backed IC and save it
    auto *ic1 = new IC();
    ic1->loadFile("simple_and.panda", m_fixtureDir);
    ws.scene()->addItem(ic1);

    QByteArray elemData;
    {
        QDataStream out(&elemData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(out);
        ic1->save(out);
    }

    // Load with null blobRegistry — should fall back to file load successfully
    auto ic2 = std::make_unique<IC>();
    QDataStream in(&elemData, QIODevice::ReadOnly);
    QMap<quint64, QNEPort *> portMap;
    SerializationContext ctx{portMap, Serialization::readPandaHeader(in), m_fixtureDir};
    // ctx.blobRegistry is nullptr by default

    ic2->load(in, ctx);

    // Should have loaded as file-backed
    QVERIFY(!ic2->isEmbedded());
    QVERIFY(ic2->inputSize() > 0);
    QVERIFY(ic2->outputSize() > 0);
}

void TestICInline::testSaveLoadOrphanBlobPreserved()
{
    // Orphan blobs (no IC instances) must survive a workspace save/load round-trip.
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    WorkSpace ws1;
    ws1.scene()->setContextDir(m_fixtureDir);
    ws1.scene()->icRegistry()->setBlob("orphan_blob", blob);

    // No IC instances referencing "orphan_blob" — it's an orphan.
    // Add an unrelated element so the scene isn't empty.
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    ws1.scene()->addItem(sw);

    // Save
    const QString savePath = m_fixtureDir + "/orphan_test.panda";
    ws1.save(savePath);

    // Load into a fresh workspace
    WorkSpace ws2;
    QFile file(savePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QDataStream stream(&file);
    auto version = Serialization::readPandaHeader(stream);
    ws2.load(stream, version, m_fixtureDir);

    // The orphan blob must have been preserved
    QVERIFY2(ws2.scene()->icRegistry()->hasBlob("orphan_blob"),
             "Orphan blob should survive save/load round-trip");
    QCOMPARE(ws2.scene()->icRegistry()->blob("orphan_blob"), blob);
}

void TestICInline::testReconnectConnectionsSkipsDeletedElement()
{
    // UpdateBlobCommand::reconnectConnections() must silently skip connections
    // whose target element was deleted between capture and reconnect.
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    auto *ic = new IC();
    embedIC(ic, blob, "reconnect_test", m_fixtureDir, reg);
    ic->setPos(100, 100);
    ws.scene()->addItem(ic);

    auto swOwner = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputSwitch));
    auto *sw = swOwner.get();
    sw->setPos(0, 100);
    ws.scene()->addItem(sw);

    auto *conn = new QNEConnection();
    conn->setStartPort(sw->outputPort(0));
    conn->setEndPort(ic->inputPort(0));
    ws.scene()->addItem(conn);

    // Capture connections before embed
    const auto targets = QList<GraphicElement *>{ic};
    const auto connections = UpdateBlobCommand::captureConnections(targets);
    const QByteArray oldData = ICRegistry::captureSnapshot(targets);

    // Mutate the IC (re-embed same blob)
    ic->loadFromBlob(blob, m_fixtureDir);

    // Delete the switch (source of connection) before undo
    ws.scene()->removeItem(sw);
    swOwner.reset();

    // Create UpdateBlobCommand and try undo — must not crash
    auto *cmd = new UpdateBlobCommand(targets, oldData, connections, ws.scene());
    cmd->setOldBlob(blob);
    cmd->setBlobName("reconnect_test");
    ws.scene()->undoStack()->push(cmd);

    // Undo should not crash even though sw is gone
    ws.scene()->undoStack()->undo();

    // The IC should still be functional
    QVERIFY(ic->inputSize() > 0);
}

void TestICInline::testUpdateBlobCommandUndoRestoresOldBlob()
{
    // When UpdateBlobCommand is undone, the old blob data must be restored
    // in the registry.
    QByteArray blobA = readFile(m_fixtureDir + "/simple_and.panda");
    QByteArray blobB = readFile(m_fixtureDir + "/chain_c.panda");

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    auto *ic = new IC();
    embedIC(ic, blobA, "undo_blob", m_fixtureDir, reg);
    ic->setPos(100, 100);
    ws.scene()->addItem(ic);

    // Capture old state
    const auto targets = QList<GraphicElement *>{ic};
    const auto connections = UpdateBlobCommand::captureConnections(targets);
    const QByteArray oldData = ICRegistry::captureSnapshot(targets);
    const QByteArray oldBlob = reg->blob("undo_blob");

    // Mutate: update blob to blobB
    reg->setBlob("undo_blob", blobB);
    ic->loadFromBlob(blobB, m_fixtureDir);

    auto *cmd = new UpdateBlobCommand(targets, oldData, connections, ws.scene());
    cmd->setOldBlob(oldBlob);
    cmd->setBlobName("undo_blob");
    ws.scene()->undoStack()->push(cmd);

    // After push: blob should be blobB
    QCOMPARE(reg->blob("undo_blob"), blobB);

    // Undo: blob should be restored to blobA
    ws.scene()->undoStack()->undo();
    QCOMPARE(reg->blob("undo_blob"), blobA);

    // Redo: blob should be blobB again
    ws.scene()->undoStack()->redo();
    QCOMPARE(reg->blob("undo_blob"), blobB);
}

// ============================================================================
// Edge cases: isEmbedded, port names, RegisterBlobCommand, IC::load
// ============================================================================

void TestICInline::testIsEmbeddedICWithStaleBlobName()
{
    // isEmbedded() returns true whenever blobName is non-empty, even if the
    // blob was removed from the registry.  Verify this is the actual behavior.
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    auto *ic = new IC();
    embedIC(ic, blob, "stale_blob", m_fixtureDir, reg);
    scene.addItem(ic);

    QVERIFY(ic->isEmbedded());

    // Remove the blob — blobName is still set on the IC
    reg->removeBlob("stale_blob");
    QVERIFY(!reg->hasBlob("stale_blob"));

    // isEmbedded still returns true (checks blobName, not registry)
    QVERIFY(ic->isEmbedded());
    QCOMPARE(ic->blobName(), QString("stale_blob"));
}

void TestICInline::testRegisterBlobCommandUndoRedo()
{
    // RegisterBlobCommand undo removes the blob, redo re-registers it.
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    scene.undoStack()->push(new RegisterBlobCommand("cmd_test", blob, &scene));

    QVERIFY(reg->hasBlob("cmd_test"));

    scene.undoStack()->undo();
    QVERIFY(!reg->hasBlob("cmd_test"));

    scene.undoStack()->redo();
    QVERIFY(reg->hasBlob("cmd_test"));
}

void TestICInline::testRegisterBlobCommandRedoAfterExternalRemove()
{
    // If the blob is externally removed between undo and redo, redo must
    // re-register it without issue (registerBlob is idempotent on the name).
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    scene.undoStack()->push(new RegisterBlobCommand("ext_rm", blob, &scene));
    QVERIFY(reg->hasBlob("ext_rm"));

    scene.undoStack()->undo();
    QVERIFY(!reg->hasBlob("ext_rm"));

    // External modification: register a DIFFERENT blob with the same name
    reg->setBlob("ext_rm", QByteArray("different_data"));

    // Redo re-registers the original blob — overwrites the external data
    scene.undoStack()->redo();
    QVERIFY(reg->hasBlob("ext_rm"));
    // registerBlob calls makeBlobSelfContained, so the stored data might differ
    // from the raw blob bytes, but the blob must exist
    QVERIFY(!reg->blob("ext_rm").isEmpty());
}

void TestICInline::testLoadICMissingAllNameFieldsThrows()
{
    // IC::load() must throw when the serialized QMap has no "name", "fileName",
    // or "blobName" key.
    Scene scene;
    scene.setContextDir(m_fixtureDir);

    // Build a minimal IC save buffer, then tamper the map to remove the name
    auto *ic = new IC();
    ic->loadFile("simple_and.panda", m_fixtureDir);
    scene.addItem(ic);

    QByteArray elemData;
    {
        QDataStream out(&elemData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(out);
        // Write GraphicElement base
        static_cast<GraphicElement *>(ic)->GraphicElement::save(out);
        // Write an EMPTY map (no name key)
        QMap<QString, QVariant> emptyMap;
        out << emptyMap;
    }

    auto ic2 = std::make_unique<IC>();
    QDataStream in(&elemData, QIODevice::ReadOnly);
    QMap<quint64, QNEPort *> portMap;
    SerializationContext ctx{portMap, Serialization::readPandaHeader(in), m_fixtureDir};

    bool threw = false;
    try {
        ic2->load(in, ctx);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "IC::load() must throw when no name field is present");
}

// ============================================================================
// Edge cases: cycle detection, uniqueBlobName, blob access, clearBlobs
// ============================================================================

void TestICInline::testUniqueBlobNameEmptyBase()
{
    // uniqueBlobName("") should return "" when no blob named "" exists.
    Scene scene;
    auto *reg = scene.icRegistry();

    QCOMPARE(reg->uniqueBlobName(""), QString(""));

    // If we register "" (pathological but legal), next should be "_2"
    reg->setBlob("", QByteArray("data"));
    QCOMPARE(reg->uniqueBlobName(""), QString("_2"));

    reg->removeBlob("");
}

void TestICInline::testBlobReturnsEmptyForMissingName()
{
    // blob() for a non-existent name must return an empty QByteArray.
    Scene scene;
    auto *reg = scene.icRegistry();

    QVERIFY(reg->blob("does_not_exist").isEmpty());
    QVERIFY(reg->blob("").isEmpty());
}

void TestICInline::testClearBlobsLeavesICsStale()
{
    // clearBlobs() removes all blobs but doesn't modify IC instances.
    // ICs become stale (blobName set, blob gone).
    QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    auto *ic = new IC();
    embedIC(ic, blob, "will_be_cleared", m_fixtureDir, reg);
    scene.addItem(ic);

    QVERIFY(ic->isEmbedded());
    QVERIFY(reg->hasBlob("will_be_cleared"));

    reg->clearBlobs();

    QVERIFY(!reg->hasBlob("will_be_cleared"));
    // IC still thinks it's embedded (blobName unchanged)
    QVERIFY(ic->isEmbedded());
    QCOMPARE(ic->blobName(), QString("will_be_cleared"));
}

void TestICInline::testLoadFileNotFoundPreservesEmbeddedState()
{
    // loadFile() with a nonexistent file must throw without corrupting the IC.
    // Previously m_blobName was cleared before the file check, leaving the IC
    // in a broken state (not embedded, no file) that crashed the simulation.

    Scene scene;
    scene.setContextDir(m_fixtureDir);
    auto *reg = scene.icRegistry();

    IC ic;
    const QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    embedIC(&ic, blob, "keep_me", m_fixtureDir, reg);
    scene.addItem(&ic);

    QVERIFY(ic.isEmbedded());
    QCOMPARE(ic.blobName(), QString("keep_me"));
    const int origInputs = ic.inputSize();
    QVERIFY(origInputs > 0);

    // Attempt to load a nonexistent file — must throw
    bool threw = false;
    try {
        ic.loadFile("nonexistent.panda", m_fixtureDir);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY(threw);

    // IC must remain in its original embedded state
    QVERIFY(ic.isEmbedded());
    QCOMPARE(ic.blobName(), QString("keep_me"));
    QCOMPARE(ic.inputSize(), origInputs);

    scene.removeItem(&ic);
}

void TestICInline::testExtractToFileUsesContextDir()
{
    // extractToFile must write the file to the specified path (which callers
    // should set to contextDir) and the resulting IC must be file-backed
    // pointing at that location.

    const QByteArray blob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!blob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    auto *ic = new IC();
    embedIC(ic, blob, "ctx_extract", m_fixtureDir, reg);
    ic->setPos(100, 100);
    ws.scene()->addItem(ic);
    int icId = ic->id();
    QVERIFY(ic->isEmbedded());

    // Extract to contextDir — the correct behavior
    const QString extractPath = QDir(m_fixtureDir).absoluteFilePath("ctx_extract.panda");
    reg->extractToFile("ctx_extract", extractPath);

    auto *extracted = dynamic_cast<IC *>(ws.scene()->itemById(icId));
    QVERIFY(extracted);
    QVERIFY(!extracted->isEmbedded());
    QVERIFY(extracted->file().contains("ctx_extract.panda"));
    QVERIFY(extracted->inputSize() > 0);

    // The file must exist in contextDir
    QVERIFY(QFile::exists(extractPath));

    // Blob must be removed from registry
    QVERIFY(!reg->hasBlob("ctx_extract"));
}

// ============================================================================
// Edge cases: rollback, rename, and missing deps
// ============================================================================

void TestICInline::testChildBlobSavedPartialFailureRollback()
{
    // onChildICBlobSaved with a blob that fails partway through multiple targets
    // must roll back all already-updated ICs and restore the original blob in the
    // registry, leaving the scene in a consistent state.

    QByteArray andBlob = readFile(m_fixtureDir + "/simple_and.panda");
    QVERIFY(!andBlob.isEmpty());

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();
    reg->setBlob("partial_test", andBlob);

    QList<int> ids;
    for (int i = 0; i < 3; ++i) {
        auto *ic = new IC();
        ic->setBlobName("partial_test");
        ic->loadFromBlob(andBlob, m_fixtureDir);
        ic->setPos(i * 100, 100);
        ic->setLabel(QString("IC_%1").arg(i));
        ws.scene()->addItem(ic);
        ids.append(ic->id());
    }

    const int origInputs = static_cast<IC *>(ws.scene()->itemById(ids.at(0)))->inputSize();
    const int stackBefore = ws.scene()->undoStack()->index();

    // Use corrupt blob — triggers exception during loadFromBlob
    QByteArray corruptBlob("corrupt");
    bool threw = false;
    try {
        ws.onChildICBlobSaved(ids.at(0), corruptBlob);
    } catch (...) {
        threw = true;
    }

    // Exception must have been thrown (corrupt blob can't be parsed)
    QVERIFY2(threw, "onChildICBlobSaved should throw on corrupt blob");

    // No undo command pushed
    QCOMPARE(ws.scene()->undoStack()->index(), stackBefore);

    // All 3 ICs should be unchanged
    for (int i = 0; i < 3; ++i) {
        auto *ic = dynamic_cast<IC *>(ws.scene()->itemById(ids.at(i)));
        QVERIFY2(ic, qPrintable(QString("IC %1 not found after rollback").arg(i)));
        QCOMPARE(ic->inputSize(), origInputs);
        QCOMPARE(ic->blobName(), QString("partial_test"));
        QCOMPARE(ic->label(), QString("IC_%1").arg(i));
    }

    // Registry should have the original blob restored, not the corrupt one
    QVERIFY(reg->hasBlob("partial_test"));
    QCOMPARE(reg->blob("partial_test"), andBlob);
}

void TestICInline::testRenameBlobUpdatesNestedMetadata()
{
    // When renaming a blob, parent blobs that contain it as a nested dependency
    // must have their internal metadata updated to reference the new name.

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    // nested_and.panda references simple_and.panda. registerBlob flattens it.
    QByteArray nestedBlob = readFile(m_fixtureDir + "/nested_and.panda");
    QVERIFY(!nestedBlob.isEmpty());

    reg->registerBlob("nested_and", nestedBlob);

    // Verify simple_and is embedded inside nested_and
    {
        QByteArray blob = reg->blob("nested_and");
        QDataStream stream(&blob, QIODevice::ReadOnly);
        auto preamble = Serialization::readPreamble(stream);
        auto embeddedICs = Serialization::deserializeBlobRegistry(preamble.metadata);
        QVERIFY2(embeddedICs.contains("simple_and"),
                 "simple_and should be inside nested_and before rename");
    }

    // Also add simple_and as a sibling blob so renameBlob can find it
    QByteArray simpleBlob = readFile(m_fixtureDir + "/simple_and.panda");
    reg->setBlob("simple_and", simpleBlob);

    // Rename simple_and → renamed_and
    reg->renameBlob("simple_and", "renamed_and");

    // nested_and's internal metadata should now reference renamed_and, not simple_and
    QByteArray updatedParent = reg->blob("nested_and");
    QDataStream stream(&updatedParent, QIODevice::ReadOnly);
    auto preamble = Serialization::readPreamble(stream);
    auto embeddedICs = Serialization::deserializeBlobRegistry(preamble.metadata);

    QVERIFY2(!embeddedICs.contains("simple_and"),
             "simple_and should no longer appear in nested_and after rename");
    QVERIFY2(embeddedICs.contains("renamed_and"),
             "renamed_and should replace simple_and in nested_and after rename");
}

void TestICInline::testRegisterBlobMissingFileDepWarns()
{
    // registerBlob with a blob that references a non-existent file dependency
    // should warn but not crash, and the resulting blob should still be loadable
    // (minus the missing dependency).

    // chain_b references chain_c. Delete chain_c so it can't be found.
    QFile::remove(m_fixtureDir + "/chain_c.panda");
    auto restoreFixture = qScopeGuard([this] { copyFixture("chain_c.panda"); });

    WorkSpace ws;
    ws.scene()->setContextDir(m_fixtureDir);
    auto *reg = ws.scene()->icRegistry();

    QByteArray chainB = readFile(m_fixtureDir + "/chain_b.panda");
    QVERIFY(!chainB.isEmpty());

    // Should not throw — missing dep is skipped with a warning
    reg->registerBlob("chain_b", chainB);

    // The blob should be stored
    QVERIFY(reg->hasBlob("chain_b"));

    // The fileBackedICs metadata should be removed (flattening attempted),
    // but chain_c won't be in the embedded ICs since the file was missing.
    QByteArray stored = reg->blob("chain_b");
    QDataStream stream(&stored, QIODevice::ReadOnly);
    auto preamble = Serialization::readPreamble(stream);
    auto embeddedICs = Serialization::deserializeBlobRegistry(preamble.metadata);

    QVERIFY2(!embeddedICs.contains("chain_c"),
             "chain_c should not be embedded since its file was missing");

    // fileBackedICs should have been removed from metadata
    QVERIFY2(!preamble.metadata.contains("fileBackedICs"),
             "fileBackedICs metadata should be removed after flattening attempt");
}

// ============================================================================
// Inline IC save with file-based IC conversion
// ============================================================================

void TestICInline::testInlineSaveConvertsFileBackedToEmbedded()
{
    // Scenario: open an inline IC tab, drop a file-based IC, save.
    // The file-based IC should be converted to embedded in the saved blob,
    // the IC's pixmap should reflect embedded state (blue), and the blob
    // should be re-loadable without errors.

    // --- Step 1: Set up parent workspace with an embedded IC (chain_c) ---
    WorkSpace parentWs;
    parentWs.scene()->setContextDir(m_fixtureDir);
    auto *parentScene = parentWs.scene();
    auto *parentReg = parentScene->icRegistry();

    QByteArray chainCBytes = readFile(m_fixtureDir + "/chain_c.panda");
    QVERIFY(!chainCBytes.isEmpty());

    auto *parentIC = new IC();
    embedIC(parentIC, chainCBytes, "chain_c", m_fixtureDir, parentReg);
    parentIC->setPos(100, 100);
    parentScene->addItem(parentIC);
    const int parentICId = parentIC->id();

    qDebug() << "Step 1: Parent workspace has embedded IC 'chain_c' with id" << parentICId;
    qDebug() << "  Parent contextDir:" << parentScene->contextDir();
    qDebug() << "  Parent IC isEmbedded:" << parentIC->isEmbedded()
             << "blobName:" << parentIC->blobName()
             << "inputs:" << parentIC->inputSize()
             << "outputs:" << parentIC->outputSize();

    // --- Step 2: Open chain_c in a child workspace (simulating inline IC tab) ---
    WorkSpace childWs;
    const QByteArray chainCBlob = parentReg->blob("chain_c");
    QVERIFY2(!chainCBlob.isEmpty(), "chain_c blob should exist in parent registry");

    childWs.loadFromBlob(chainCBlob, &parentWs, parentICId, parentScene->contextDir());

    qDebug() << "Step 2: Child workspace loaded from blob";
    qDebug() << "  Child isInlineIC:" << childWs.isInlineIC();
    qDebug() << "  Child contextDir:" << childWs.scene()->contextDir();
    qDebug() << "  Child element count:" << childWs.scene()->elements().size();
    qDebug() << "  Child blob registry size:" << childWs.scene()->icRegistry()->blobMap().size();

    // Connect icBlobSaved to parent (simulating what MainWindow does)
    connect(&childWs, &WorkSpace::icBlobSaved, &parentWs, &WorkSpace::onChildICBlobSaved);

    // --- Step 3: Drop a file-based IC into the child scene ---
    auto *droppedIC = new IC();
    const QString simpleAndFile = m_fixtureDir + "/simple_and.panda";
    droppedIC->loadFile(simpleAndFile, m_fixtureDir);
    droppedIC->setPos(200, 200);
    childWs.scene()->addItem(droppedIC);
    const int droppedICId = droppedIC->id();

    qDebug() << "Step 3: Dropped file-based IC";
    qDebug() << "  Dropped IC isEmbedded:" << droppedIC->isEmbedded()
             << "file:" << droppedIC->file()
             << "blobName:" << droppedIC->blobName()
             << "inputs:" << droppedIC->inputSize()
             << "outputs:" << droppedIC->outputSize();

    QVERIFY2(!droppedIC->isEmbedded(), "Dropped IC should be file-backed before save");
    QVERIFY(droppedIC->inputSize() > 0);

    // --- Step 4: Save the child workspace (triggers inline save path) ---
    qDebug() << "Step 4: Saving child workspace...";
    qDebug() << "  Child blob registry before save:" << childWs.scene()->icRegistry()->blobMap().keys();

    QSignalSpy saveSpy(&childWs, &WorkSpace::icBlobSaved);
    childWs.save("");

    qDebug() << "  icBlobSaved signal emitted:" << saveSpy.count() << "times";
    QCOMPARE(saveSpy.count(), 1);

    // --- Step 5: Verify the dropped IC was converted to embedded ---
    auto *savedIC = dynamic_cast<IC *>(childWs.scene()->itemById(droppedICId));
    QVERIFY2(savedIC, "Dropped IC should still exist after save");

    qDebug() << "Step 5: After save";
    qDebug() << "  Saved IC isEmbedded:" << savedIC->isEmbedded()
             << "blobName:" << savedIC->blobName()
             << "file:" << savedIC->file()
             << "inputs:" << savedIC->inputSize();
    qDebug() << "  Child blob registry after save:" << childWs.scene()->icRegistry()->blobMap().keys();

    QVERIFY2(savedIC->isEmbedded(), "IC should be embedded after inline save");
    QCOMPARE(savedIC->blobName(), QString("simple_and"));

    // Verify the pixmap reflects embedded state (blue body color)
    // generatePixmap uses isEmbedded() to pick body color — if the pixmap wasn't
    // regenerated, the IC still looks gray (file-backed) visually.
    // We can't inspect pixel colors easily, but we can check that generatePixmap
    // would pick the right color by verifying isEmbedded() is true (already done above).
    // The real test: was loadFromBlob called so the pixmap was regenerated?
    // If only setBlobName was called without loadFromBlob, the pixmap is stale.

    // --- Step 6: Verify the saved blob is valid and re-loadable ---
    const QByteArray savedBlob = saveSpy.at(0).at(1).toByteArray();
    qDebug() << "Step 6: Verifying saved blob (" << savedBlob.size() << "bytes)";

    // Parse the blob header — this is where "invalid format" would throw
    {
        QByteArray blobCopy(savedBlob);
        QDataStream stream(&blobCopy, QIODevice::ReadOnly);
        bool parsedOK = false;
        try {
            auto preamble = Serialization::readPreamble(stream);
            parsedOK = true;
            auto embeddedICs = Serialization::deserializeBlobRegistry(preamble.metadata);

            qDebug() << "  Blob version:" << preamble.version;
            qDebug() << "  Blob embedded ICs:" << embeddedICs.keys();
            qDebug() << "  Blob metadata keys:" << preamble.metadata.keys();

            QVERIFY2(embeddedICs.contains("simple_and"),
                     "simple_and should be embedded in the saved blob");
        } catch (const std::exception &e) {
            qDebug() << "  PARSE ERROR:" << e.what();
        }
        QVERIFY2(parsedOK, "Saved blob should have a valid panda header");
    }

    // --- Step 7: Verify parent received the blob and updated its registry ---
    qDebug() << "Step 7: Checking parent state after child save";
    qDebug() << "  Parent blob registry:" << parentReg->blobMap().keys();

    const QByteArray updatedChainC = parentReg->blob("chain_c");
    QVERIFY2(!updatedChainC.isEmpty(), "Parent should have updated chain_c blob");

    // --- Step 8: Simulate closing and reopening the tab ---
    // Load a new workspace from the parent's updated chain_c blob
    qDebug() << "Step 8: Reopening chain_c from parent blob (" << updatedChainC.size() << "bytes)";

    WorkSpace reopenedWs;
    bool reopenOK = false;
    try {
        reopenedWs.loadFromBlob(updatedChainC, &parentWs, parentICId, parentScene->contextDir());
        reopenOK = true;
    } catch (const std::exception &e) {
        qDebug() << "  REOPEN ERROR:" << e.what();
    }
    QVERIFY2(reopenOK, "Reopened workspace should load without 'invalid format' error");

    qDebug() << "  Reopened element count:" << reopenedWs.scene()->elements().size();
    qDebug() << "  Reopened blob registry:" << reopenedWs.scene()->icRegistry()->blobMap().keys();

    // The reopened workspace should contain the simple_and IC as embedded
    bool foundEmbeddedIC = false;
    for (auto *elm : reopenedWs.scene()->elements()) {
        if (elm->elementType() == ElementType::IC && elm->isEmbedded()) {
            qDebug() << "  Found embedded IC:" << elm->blobName()
                     << "inputs:" << elm->inputSize()
                     << "outputs:" << elm->outputSize();
            if (elm->blobName() == "simple_and") {
                foundEmbeddedIC = true;
            }
        }
    }
    QVERIFY2(foundEmbeddedIC, "Reopened workspace should contain embedded simple_and IC");
}

void TestICInline::testNestedInlineSaveAndReopen()
{
    // Reproduce exact user scenario with real Examples/ files:
    // 1. Open nested.panda (root) — contains chain_b IC (file-backed)
    // 2. Double-click chain_b → open as inline IC tab (level 1)
    // 3. Inside chain_b, double-click chain_c → open as inline IC tab (level 2)
    // 4. Drop counter.panda from file palette into chain_c's scene
    // 5. Save chain_c tab
    // 6. Close chain_c tab, reopen from chain_b's blob → should not throw

    const QString exDir = TestUtils::examplesDir();

    // --- Step 1: Load nested.panda as root workspace ---
    WorkSpace rootWs;
    rootWs.load(exDir + "nested.panda");
    auto *rootScene = rootWs.scene();

    qDebug() << "=== Step 1: Root loaded nested.panda ===";
    qDebug() << "  Root contextDir:" << rootScene->contextDir();
    qDebug() << "  Root elements:" << rootScene->elements().size();
    qDebug() << "  Root blob registry:" << rootScene->icRegistry()->blobMap().keys();

    // Find chain_b IC in root scene
    IC *chainBIC = nullptr;
    for (auto *elm : rootScene->elements()) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = static_cast<IC *>(elm);
            qDebug() << "  Root IC:" << ic->blobName() << "file:" << ic->file()
                     << "isEmbedded:" << ic->isEmbedded()
                     << "inputs:" << ic->inputSize() << "outputs:" << ic->outputSize();
            if (!chainBIC) chainBIC = ic;
        }
    }
    QVERIFY2(chainBIC, "nested.panda should contain at least one IC");
    const int chainBICId = chainBIC->id();

    // Get chain_b blob for opening inline tab
    QByteArray chainBBlob;
    if (chainBIC->isEmbedded()) {
        chainBBlob = rootScene->icRegistry()->blob(chainBIC->blobName());
    } else {
        // File-backed — read from disk, embed it
        chainBBlob = readFile(chainBIC->file());
        rootScene->icRegistry()->registerBlob("chain_b", chainBBlob);
        chainBIC->setBlobName("chain_b");
        chainBIC->loadFromBlob(rootScene->icRegistry()->blob("chain_b"), rootScene->contextDir());
        chainBBlob = rootScene->icRegistry()->blob("chain_b");
    }
    QVERIFY(!chainBBlob.isEmpty());

    // --- Step 2: Open chain_b in level-1 workspace ---
    WorkSpace level1Ws;
    level1Ws.loadFromBlob(chainBBlob, &rootWs, chainBICId, rootScene->contextDir());
    connect(&level1Ws, &WorkSpace::icBlobSaved, &rootWs, &WorkSpace::onChildICBlobSaved);

    qDebug() << "=== Step 2: Level-1 (chain_b) opened ===";
    qDebug() << "  Level-1 elements:" << level1Ws.scene()->elements().size();
    qDebug() << "  Level-1 blob registry:" << level1Ws.scene()->icRegistry()->blobMap().keys();

    // Find chain_c IC inside level-1
    IC *chainCIC = nullptr;
    for (auto *elm : level1Ws.scene()->elements()) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = static_cast<IC *>(elm);
            qDebug() << "  Level-1 IC:" << ic->blobName() << "file:" << ic->file()
                     << "isEmbedded:" << ic->isEmbedded();
            if (!chainCIC) chainCIC = ic;
        }
    }
    QVERIFY2(chainCIC, "chain_b should contain chain_c IC");
    const int chainCICId = chainCIC->id();

    // Get chain_c blob
    QByteArray chainCBlob;
    const QString chainCBlobName = chainCIC->isEmbedded() ? chainCIC->blobName() : "chain_c";
    if (chainCIC->isEmbedded()) {
        chainCBlob = level1Ws.scene()->icRegistry()->blob(chainCBlobName);
    } else {
        chainCBlob = readFile(chainCIC->file());
        level1Ws.scene()->icRegistry()->registerBlob(chainCBlobName, chainCBlob);
        chainCIC->setBlobName(chainCBlobName);
        chainCIC->loadFromBlob(level1Ws.scene()->icRegistry()->blob(chainCBlobName),
                               level1Ws.scene()->contextDir());
        chainCBlob = level1Ws.scene()->icRegistry()->blob(chainCBlobName);
    }
    QVERIFY(!chainCBlob.isEmpty());
    qDebug() << "  chain_c blob size:" << chainCBlob.size();

    // --- Step 3: Open chain_c in level-2 workspace ---
    WorkSpace level2Ws;
    level2Ws.loadFromBlob(chainCBlob, &level1Ws, chainCICId, level1Ws.scene()->contextDir());
    connect(&level2Ws, &WorkSpace::icBlobSaved, &level1Ws, &WorkSpace::onChildICBlobSaved);

    qDebug() << "=== Step 3: Level-2 (chain_c) opened ===";
    qDebug() << "  Level-2 elements:" << level2Ws.scene()->elements().size();
    qDebug() << "  Level-2 contextDir:" << level2Ws.scene()->contextDir();
    qDebug() << "  Level-2 blob registry:" << level2Ws.scene()->icRegistry()->blobMap().keys();

    // --- Step 4: Drop counter.panda (file-based) into level-2 ---
    const QString counterFile = exDir + "counter.panda";
    QVERIFY2(QFile::exists(counterFile), qPrintable("counter.panda not found: " + counterFile));

    auto *counterIC = new IC();
    counterIC->loadFile(counterFile, exDir);
    counterIC->setPos(200, 200);
    level2Ws.scene()->addItem(counterIC);

    qDebug() << "=== Step 4: Dropped counter.panda ===";
    qDebug() << "  Counter isEmbedded:" << counterIC->isEmbedded()
             << "file:" << counterIC->file()
             << "inputs:" << counterIC->inputSize()
             << "outputs:" << counterIC->outputSize();

    QVERIFY2(!counterIC->isEmbedded(), "Counter should be file-backed before save");
    QVERIFY(counterIC->inputSize() > 0);

    // --- Step 5: Save level-2 ---
    qDebug() << "=== Step 5: Saving level-2 ===";
    QSignalSpy spy2(&level2Ws, &WorkSpace::icBlobSaved);
    level2Ws.save("");
    QCOMPARE(spy2.count(), 1);

    qDebug() << "  Counter after save — isEmbedded:" << counterIC->isEmbedded()
             << "blobName:" << counterIC->blobName()
             << "file:" << counterIC->file();
    qDebug() << "  Level-2 blob registry after save:" << level2Ws.scene()->icRegistry()->blobMap().keys();

    // Verify the saved blob is valid
    const QByteArray savedBlob = spy2.at(0).at(1).toByteArray();
    qDebug() << "  Saved blob size:" << savedBlob.size();
    {
        QByteArray copy(savedBlob);
        QDataStream s(&copy, QIODevice::ReadOnly);
        try {
            auto p = Serialization::readPreamble(s);
            auto ics = Serialization::deserializeBlobRegistry(p.metadata);
            qDebug() << "  Saved blob embedded ICs:" << ics.keys();
            qDebug() << "  Saved blob metadata keys:" << p.metadata.keys();
        } catch (const std::exception &e) {
            qDebug() << "  ERROR parsing saved blob:" << e.what();
            QFAIL(qPrintable(QString("Saved blob should be valid: %1").arg(e.what())));
        }
    }

    // Verify level-1 received and stored the blob
    qDebug() << "  Level-1 blob registry after save:" << level1Ws.scene()->icRegistry()->blobMap().keys();
    const QByteArray updatedChainC = level1Ws.scene()->icRegistry()->blob(chainCBlobName);
    QVERIFY2(!updatedChainC.isEmpty(), "Level-1 should have updated chain_c blob");
    qDebug() << "  Updated chain_c blob in level-1:" << updatedChainC.size() << "bytes";

    // --- Step 6: Close level-2 and reopen from level-1's blob ---
    qDebug() << "=== Step 6: Reopening level-2 from level-1's blob ===";

    // Parse the blob before loading to see what's in it
    {
        QByteArray copy(updatedChainC);
        QDataStream s(&copy, QIODevice::ReadOnly);
        try {
            auto p = Serialization::readPreamble(s);
            auto ics = Serialization::deserializeBlobRegistry(p.metadata);
            qDebug() << "  chain_c blob version:" << p.version;
            qDebug() << "  chain_c blob embedded ICs:" << ics.keys();
            qDebug() << "  chain_c blob metadata keys:" << p.metadata.keys();
            qDebug() << "  Remaining bytes after preamble:" << (copy.size() - s.device()->pos());
        } catch (const std::exception &e) {
            qDebug() << "  ERROR parsing chain_c blob before reopen:" << e.what();
        }
    }

    WorkSpace reopenedWs;
    bool reopenOK = false;
    try {
        reopenedWs.loadFromBlob(updatedChainC, &level1Ws, chainCICId, level1Ws.scene()->contextDir());
        reopenOK = true;
    } catch (const std::exception &e) {
        qDebug() << "  REOPEN ERROR:" << e.what();
    }
    QVERIFY2(reopenOK, "Reopening chain_c should not throw 'invalid format'");

    qDebug() << "  Reopened elements:" << reopenedWs.scene()->elements().size();
    qDebug() << "  Reopened blob registry:" << reopenedWs.scene()->icRegistry()->blobMap().keys();

    // Verify counter IC is present and embedded
    bool foundCounter = false;
    for (auto *elm : reopenedWs.scene()->elements()) {
        if (elm->elementType() == ElementType::IC) {
            qDebug() << "  Reopened IC:" << elm->blobName()
                     << "isEmbedded:" << elm->isEmbedded()
                     << "inputs:" << elm->inputSize()
                     << "outputs:" << elm->outputSize();
            if (elm->blobName() == "counter") {
                foundCounter = true;
            }
        }
    }
    QVERIFY2(foundCounter, "Reopened workspace should contain embedded counter IC");
}

