// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestICUnit.h"

#include <limits>
#include <memory>

#include <QFile>
#include <QKeySequence>
#include <QPointF>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Element/ICRenderer.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Versions.h"
#include "App/Wiring/Port.h"
#include "Tests/QuickShell/IC/QuickTestUtils.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

void TestICUnit::testICLoadFromFile()
{
    // A never-loaded IC's elementType() is trivially IC by construction; this actually
    // loads a real sub-circuit (the established QuickCircuitBuilder + IC::loadFile() idiom
    // used throughout this file) and checks the load actually took effect.
    QTemporaryDir subDir;
    QVERIFY(subDir.isValid());

    QuickCircuitBuilder subBuilder;
    InputSwitch sw;
    Led led;
    subBuilder.add(&sw, &led);
    subBuilder.connect(&sw, 0, &led, 0);
    const QString subPath = subDir.path() + "/load_from_file_probe.panda";
    QuickTestUtils::savePandaFile(subBuilder, subPath);

    IC ic;
    ic.loadFile(subPath, subDir.path());

    QCOMPARE(ic.elementType(), ElementType::IC);
    QCOMPARE(ic.file(), subPath);
    QCOMPARE(ic.inputSize(), 1);
    QCOMPARE(ic.outputSize(), 1);
}

void TestICUnit::testICPortLabelResolution()
{
    // buildPortLabels() (ICLoader.cpp) composes each boundary port's name from its internal
    // element's label(); verify a real loaded IC actually resolves that label onto the
    // IC's own port name, not just that an unloaded IC has zero ports.
    QTemporaryDir subDir;
    QVERIFY(subDir.isValid());

    QuickCircuitBuilder subBuilder;
    InputSwitch sw;
    sw.setLabel("DataIn");
    Led led;
    led.setLabel("DataOut");
    subBuilder.add(&sw, &led);
    subBuilder.connect(&sw, 0, &led, 0);
    const QString subPath = subDir.path() + "/port_label_probe.panda";
    QuickTestUtils::savePandaFile(subBuilder, subPath);

    IC ic;
    ic.loadFile(subPath, subDir.path());

    QVERIFY(!ic.inputs().isEmpty());
    QVERIFY(!ic.outputs().isEmpty());
    QVERIFY2(ic.inputPort(0)->name().contains("DataIn"), qPrintable(ic.inputPort(0)->name()));
    QVERIFY2(ic.outputPort(0)->name().contains("DataOut"), qPrintable(ic.outputPort(0)->name()));
}

void TestICUnit::testICNestedSaveLoad()
{
    // A genuinely nested sub-circuit: the inner IC (innerPath) is itself embedded as one of
    // the outer sub-circuit's elements, so loading outerPath produces an IC whose own
    // internal elements include another IC. Exercises both halves of "save then load" --
    // ic.save() on the real loaded state, and reloading that same file into a second IC.
    QTemporaryDir subDir;
    QVERIFY(subDir.isValid());

    QuickCircuitBuilder innerBuilder;
    InputSwitch innerSw;
    Led innerLed;
    innerBuilder.add(&innerSw, &innerLed);
    innerBuilder.connect(&innerSw, 0, &innerLed, 0);
    const QString innerPath = subDir.path() + "/nested_inner.panda";
    QuickTestUtils::savePandaFile(innerBuilder, innerPath);

    QuickCircuitBuilder outerBuilder;
    InputSwitch outerSw;
    auto *innerIc = new IC;
    innerIc->loadFile(innerPath, subDir.path());
    Led outerLed;
    outerBuilder.add(&outerSw, &outerLed);
    outerBuilder.addOwnedElement(innerIc);
    outerBuilder.connect(&outerSw, 0, innerIc, 0);
    outerBuilder.connect(innerIc, 0, &outerLed, 0);
    const QString outerPath = subDir.path() + "/nested_outer.panda";
    QuickTestUtils::savePandaFile(outerBuilder, outerPath);

    IC ic;
    ic.loadFile(outerPath, subDir.path());

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    ic.save(stream, {.purpose = SerializationPurpose::PortableFile});
    QVERIFY(!data.isEmpty());
    QCOMPARE(ic.inputSize(), 1);
    QCOMPARE(ic.outputSize(), 1);

    // Load the same file into a second, independent IC and confirm it reaches the same
    // structural state.
    IC ic2;
    ic2.loadFile(outerPath, subDir.path());
    QCOMPARE(ic2.file(), ic.file());
    QCOMPARE(ic2.inputSize(), ic.inputSize());
    QCOMPARE(ic2.outputSize(), ic.outputSize());
}

void TestICUnit::testICInvalidFile()
{
    // A never-loaded IC's file() being empty is trivially true by construction; this
    // actually attempts to load a genuinely nonexistent path and confirms both the throw
    // (ICLoader::loadFile()'s exists()/isFile() guard) and that m_file is left untouched.
    IC ic;
    const QString badPath = QStringLiteral("/no/such/directory/does_not_exist.panda");

    bool threw = false;
    try {
        ic.loadFile(badPath, QString());
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(threw, "Loading a nonexistent file must throw");
    QVERIFY(ic.file().isEmpty());
}

void TestICUnit::testUnloadedIcSimulationMethodsAreNoOps()
{
    // A freshly-constructed, never-loaded IC has 0 internal elements -- ICSimulation::initialize()/
    // update()/resettle() must all recognize this and return immediately rather than operate on
    // an empty m_sortedInternalElements/m_internalElements.
    IC ic;
    ic.initializeSimulation();
    ic.updateLogic();
    ic.resettleCombinational();

    QCOMPARE(ic.inputSize(), 0);
    QCOMPARE(ic.outputSize(), 0);
}

void TestICUnit::testLoadedIcWithDisconnectedInputIsUnknown()
{
    // Build a tiny real sub-circuit (the established QuickCircuitBuilder + IC::loadFile()
    // technique) so the loaded IC has genuine internal elements and m_sortedInternalElements is
    // non-empty, then drive it with its own input port left disconnected, exercising
    // ICSimulation::update()'s/resettle()'s "!simUpdateInputsAllowUnknown()" guard.
    QTemporaryDir subDir;
    QVERIFY(subDir.isValid());

    QuickCircuitBuilder subBuilder;
    InputSwitch sw;
    Led led;
    subBuilder.add(&sw, &led);
    subBuilder.connect(&sw, 0, &led, 0);

    const QString subPath = subDir.path() + "/disconnected_input_probe.panda";
    QuickTestUtils::savePandaFile(subBuilder, subPath);

    auto ic = std::make_unique<IC>();
    ic->loadFile(subPath, subDir.path());
    QVERIFY(ic->inputSize() >= 1);
    QVERIFY(ic->outputSize() >= 1);

    // ICLoader derives an input port's default status from its internal boundary element's
    // status *at load time* (Inactive here, since InputSwitch defaults off) -- force it to
    // Unknown directly so the disconnected port genuinely fails simUpdateInputsAllowUnknown(),
    // rather than "successfully" resolving to a definite level nothing actually drove.
    ic->inputPort(0)->setDefaultStatus(Status::Unknown);

    QuickTestUtils::initElm(*ic); // allocate the IC's own sim vectors without connecting its ports
    ic->initializeSimulation();

    ic->updateLogic();
    QCOMPARE(ic->outputValue(0), Status::Unknown);

    ic->resettleCombinational();
    QCOMPARE(ic->outputValue(0), Status::Unknown);
}

void TestICUnit::testGeneratePixmapWithNonFiniteBoundsIsNoOp()
{
    // Defense-in-depth: a non-finite port position makes renderBodyBounds() NaN, and
    // QSizeF::toSize() asserts (!qIsNaN) on that -> process abort. The load-side guards
    // (validateFinitePos/validateFiniteAngle) reject this on a real load, so it's forced
    // directly via the port's own public setPos() to exercise the guard deterministically.
    QTemporaryDir subDir;
    QVERIFY(subDir.isValid());
    QuickCircuitBuilder subBuilder;
    InputSwitch sw;
    Led led;
    subBuilder.add(&sw, &led);
    subBuilder.connect(&sw, 0, &led, 0);
    const QString subPath = subDir.path() + "/nonfinite_pixmap_probe.panda";
    QuickTestUtils::savePandaFile(subBuilder, subPath);

    auto ic = std::make_unique<IC>();
    ic->loadFile(subPath, subDir.path());
    QVERIFY(ic->inputSize() >= 1);

    ic->inputPort(0)->setPos(std::numeric_limits<qreal>::quiet_NaN(), 0);
    ICRenderer::generatePixmap(*ic); // must return early rather than abort
}

void TestICUnit::testGeneratePreviewPixmapWithNonFiniteBoundsIsNoOp()
{
    IC ic;
    auto elm = std::make_unique<And>();
    elm->setPos(std::numeric_limits<qreal>::quiet_NaN(), 0);

    QList<ItemWithId *> items{elm.get()};
    ICRenderer::generatePreviewPixmap(ic, items);

    QVERIFY2(ic.previewPixmap().isNull(), "a non-finite item bounding rect must reset the preview to null, not abort");
}

void TestICUnit::testGeneratePreviewPixmapWithDegenerateAspectRatioIsEmpty()
{
    // An extremely wide/thin circuit layout scales down to a QSize with one dimension
    // rounding to 0 (empty), which must reset the preview to null rather than construct a
    // degenerate QPixmap.
    IC ic;
    auto elm1 = std::make_unique<And>();
    auto elm2 = std::make_unique<And>();
    elm1->setPos(0, 0);
    elm2->setPos(100000000, 0);

    QList<ItemWithId *> items{elm1.get(), elm2.get()};
    ICRenderer::generatePreviewPixmap(ic, items);

    QVERIFY(ic.previewPixmap().isNull());
}

void TestICUnit::testReloadWithFewerPortsEvictsStalePortMapKeys()
{
    // Fuzz-hardening (surfaced by libFuzzer, see TestSerialization's bugB_ic_blob_shrink
    // fixture): if a re-load's sub-circuit reference resolves to fewer ports than the IC
    // currently has, IC::load() must evict the now-dangling portMap keys rather than leave
    // them pointing at ports loadFile() just deleted (a later Connection::load() would
    // otherwise dereference a freed pointer). Reproduced directly: load a real 2-input/
    // 1-output sub-circuit, then feed IC::load() a stream whose own input/output lists match
    // that current port count (so both get captured into savedInputKeys/savedOutputKeys), but
    // whose IC-specific "name" field points at a smaller, 1-input/1-output sub-circuit.
    QTemporaryDir subDir;
    QVERIFY(subDir.isValid());

    QuickCircuitBuilder bigBuilder;
    InputSwitch bigSw1, bigSw2;
    And andGate;
    Led bigLed1, bigLed2;
    bigBuilder.add(&bigSw1, &bigSw2, &andGate, &bigLed1, &bigLed2);
    bigBuilder.connect(&bigSw1, 0, &andGate, 0);
    bigBuilder.connect(&bigSw2, 0, &andGate, 1);
    bigBuilder.connect(&andGate, 0, &bigLed1, 0);
    bigBuilder.connect(&andGate, 0, &bigLed2, 0);
    const QString bigPath = subDir.path() + "/big.panda";
    QuickTestUtils::savePandaFile(bigBuilder, bigPath);

    QuickCircuitBuilder smallBuilder;
    InputSwitch smallSw;
    Led smallLed;
    smallBuilder.add(&smallSw, &smallLed);
    smallBuilder.connect(&smallSw, 0, &smallLed, 0);
    const QString smallPath = subDir.path() + "/small.panda";
    QuickTestUtils::savePandaFile(smallBuilder, smallPath);

    auto ic = std::make_unique<IC>();
    ic->loadFile(bigPath, subDir.path());
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(ic->outputSize(), 2);

    // New-format stream: empty properties map, 2 input entries + 1 output entry (matching the
    // IC's current port count, using Qt's own QMap/QList serialization -- wire-compatible with
    // readBoundedMetadata()/readPortList()'s manual readers), then the IC-specific "name" map
    // pointing at the smaller sub-circuit.
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << QMap<QString, QVariant>(); // properties: empty

    QMap<QString, QVariant> in0, in1;
    in0["serialId"] = QVariant::fromValue<quint64>(100);
    in1["serialId"] = QVariant::fromValue<quint64>(101);
    writeStream << QList<QMap<QString, QVariant>>{in0, in1};

    QMap<QString, QVariant> out0, out1;
    out0["serialId"] = QVariant::fromValue<quint64>(200);
    out1["serialId"] = QVariant::fromValue<quint64>(201);
    writeStream << QList<QMap<QString, QVariant>>{out0, out1};

    writeStream << QList<QMap<QString, QVariant>>{}; // appearances: none (IC has no appearance slots)

    QMap<QString, QVariant> icMap;
    icMap["name"] = QStringLiteral("small.panda");
    writeStream << icMap;

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, subDir.path()};

    QDataStream readStream(data);
    ic->load(readStream, context);

    QCOMPARE(readStream.status(), QDataStream::Ok);
    QCOMPARE(ic->inputSize(), 1);
    QCOMPARE(ic->outputSize(), 1);

    // Keys 101 and 201 (the second input/output, which no longer exist after the shrink) must
    // have been evicted; keys 100 and 200 (still valid, now pointing at the sole remaining
    // ports) must remain.
    QVERIFY(portMap.contains(100));
    QVERIFY(!portMap.contains(101));
    QVERIFY(portMap.contains(200));
    QVERIFY(!portMap.contains(201));
}

void TestICUnit::testDisplayNameForEmbeddedIc()
{
    // An embedded (blob-backed) IC's displayName() is its blob name, taking priority over any
    // file path -- reached only via the real IC::load() blob-registry resolution path, since
    // loadFromBlob() itself doesn't set m_blobName (its caller, IC::load(), does).
    QTemporaryDir subDir;
    QVERIFY(subDir.isValid());
    QuickCircuitBuilder subBuilder;
    InputSwitch sw;
    Led led;
    subBuilder.add(&sw, &led);
    subBuilder.connect(&sw, 0, &led, 0);
    const QString subPath = subDir.path() + "/embedded_probe.panda";
    QuickTestUtils::savePandaFile(subBuilder, subPath);

    QFile blobFile(subPath);
    QVERIFY(blobFile.open(QIODevice::ReadOnly));
    const QByteArray blobBytes = blobFile.readAll();
    blobFile.close();

    QMap<QString, QByteArray> blobRegistry;
    blobRegistry["my_embedded_ic"] = blobBytes;

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << QMap<QString, QVariant>();          // properties: empty
    writeStream << QList<QMap<QString, QVariant>>{};   // input ports: none (fresh IC has 0)
    writeStream << QList<QMap<QString, QVariant>>{};   // output ports: none
    writeStream << QList<QMap<QString, QVariant>>{};   // appearances: none
    QMap<QString, QVariant> icMap;
    icMap["name"] = QStringLiteral("my_embedded_ic");
    writeStream << icMap;

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, subDir.path(), {}, 1, &blobRegistry};

    QDataStream readStream(data);
    IC ic;
    ic.load(readStream, context);

    QCOMPARE(readStream.status(), QDataStream::Ok);
    QCOMPARE(ic.displayName(), QString("my_embedded_ic"));
}

void TestICUnit::testLoadFileDirectlyOpenFailureThrows()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot simulate an unreadable file on Windows (uses ACLs, not Unix permission bits)");
#else
    // ICLoader::loadFileDirectly()'s QFile::open(ReadOnly) guard: a path that passes the outer
    // loadFile()'s exists()/isFile() check (the file is really there) but cannot be reopened for
    // reading (permission-stripped) must throw cleanly instead of dereferencing a closed device.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = tempDir.path() + "/unreadable.panda";

    {
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("irrelevant -- open() itself must fail before any of this is parsed");
    }
    QVERIFY(QFile::setPermissions(path, QFileDevice::Permissions()));

    // Sanity: confirm the file really can't be opened for reading under this permission set
    // on this system before relying on it below.
    {
        QFile probe(path);
        QVERIFY(!probe.open(QIODevice::ReadOnly));
    }

    auto ic = std::make_unique<IC>();
    bool threw = false;
    QString message;
    try {
        ic->loadFile("unreadable.panda", tempDir.path());
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);

    QVERIFY2(threw, "Loading a file that cannot be opened for reading must throw");
    QVERIFY2(message.contains("Error opening file"), qPrintable(message));
#endif
}

void TestICUnit::testLoadFileDetectsCircularSelfReference()
{
    // loadFileDirectly()'s cycle-detection guard: a sub-circuit file that references itself
    // must throw instead of recursing until the call stack overflows. Reproduced by having a
    // real IC element load a small circuit as its own sub-circuit, then re-saving over that same
    // file -- IC::save() writes ExternalFilePath::forWriting(m_file, ...), which strips to the
    // bare filename, so the re-saved file's "name" entry becomes a genuine self-reference.
    QTemporaryDir subDir;
    QVERIFY(subDir.isValid());
    const QString cyclePath = subDir.path() + "/cycle.panda";

    QuickCircuitBuilder baseBuilder;
    InputSwitch sw;
    Led led;
    baseBuilder.add(&sw, &led);
    baseBuilder.connect(&sw, 0, &led, 0);
    QuickTestUtils::savePandaFile(baseBuilder, cyclePath);

    QuickCircuitBuilder selfRefBuilder;
    auto *ic = new IC;
    selfRefBuilder.addOwnedElement(ic);
    ic->loadFile(cyclePath, subDir.path());
    QVERIFY(ic->inputSize() >= 1);

    QuickTestUtils::savePandaFile(selfRefBuilder, cyclePath);

    auto ic2 = std::make_unique<IC>();
    bool threw = false;
    QString message;
    try {
        ic2->loadFile(cyclePath, subDir.path());
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }

    QVERIFY2(threw, "A self-referencing IC file must be rejected as a circular reference, not stack-overflow");
    QVERIFY2(message.contains("Circular IC reference"), qPrintable(message));
}

void TestICUnit::testMigrateFileOpenForWriteFailureThrowsAndCleansUpItems()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot make a directory unwritable on Windows (uses ACLs, not Unix permission bits)");
#else
    // migrateFile()'s QSaveFile::open(WriteOnly) guard, plus loadFileDirectly()'s itemsGuard
    // cleanup of a still-live Connection when migration throws before processLoadedItems() ever
    // runs (items is still fully populated at that point -- nothing has been drained yet).
    // Pre-create the versioned backup so createVersionedBackup() no-ops (its own QFile::copy
    // would otherwise throw first, from an unrelated line in Serialization.cpp), then lock the
    // directory so only the QSaveFile::open() this test targets fails.
    const QString srcDir = QuickTestUtils::backwardCompatibilityDir() + "v4.2.0/";
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString subName = "dlatch.panda";
    const QString subDst = tempDir.path() + "/" + subName;
    QVERIFY(QFile::copy(srcDir + subName, subDst));

    QVersionNumber oldVersion;
    {
        QFile f(subDst);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QDataStream s(&f);
        oldVersion = Serialization::readPandaHeader(s);
        QVERIFY2(oldVersion < FormatRev::current, "Fixture must be older than current version for this test");
    }

    const QString backupPath = tempDir.path() + "/dlatch.v" + oldVersion.toString() + ".panda";
    QVERIFY(QFile::copy(subDst, backupPath));

    QVERIFY(QFile::setPermissions(tempDir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));

    // Sanity: confirm QSaveFile really can't open under this directory on this system.
    {
        QSaveFile probe(subDst);
        QVERIFY(!probe.open(QIODevice::WriteOnly));
    }

    Application::migrationEnabled = true;
    auto ic = std::make_unique<IC>();
    bool threw = false;
    QString message;
    try {
        ic->loadFile(subName, tempDir.path());
    } catch (const Pandaception &e) {
        threw = true;
        message = e.englishMessage();
    }
    Application::migrationEnabled = false;

    QFile::setPermissions(tempDir.path(),
        QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    QVERIFY2(threw, "Migration must throw when the sub-circuit file cannot be reopened for writing");
    QVERIFY2(message.contains("cannot open file for writing"), qPrintable(message));
#endif
}

void TestICUnit::testMigrateFileCommitFailureThrows()
{
#ifdef Q_OS_WIN
    QSKIP("RLIMIT_FSIZE (used to force a deferred QSaveFile write failure) has no Windows equivalent");
#else
    // Same fixture/backup setup as testMigrateFileOpenForWriteFailureThrowsAndCleansUpItems()
    // above, but forces the write to fail after open() succeeds (ScopedTinyFsizeLimit) instead
    // of denying open() itself -- exercises migrateFile()'s commit()-failure guard.
    const QString srcDir = QuickTestUtils::backwardCompatibilityDir() + "v4.2.0/";
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString subName = "dlatch.panda";
    const QString subDst = tempDir.path() + "/" + subName;
    QVERIFY(QFile::copy(srcDir + subName, subDst));

    QVersionNumber oldVersion;
    {
        QFile f(subDst);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QDataStream s(&f);
        oldVersion = Serialization::readPandaHeader(s);
        QVERIFY2(oldVersion < FormatRev::current, "Fixture must be older than current version for this test");
    }

    const QString backupPath = tempDir.path() + "/dlatch.v" + oldVersion.toString() + ".panda";
    QVERIFY(QFile::copy(subDst, backupPath));

    Application::migrationEnabled = true;
    auto ic = std::make_unique<IC>();
    bool threw = false;
    QString message;
    {
        QuickTestUtils::ScopedTinyFsizeLimit tinyLimit;
        try {
            ic->loadFile(subName, tempDir.path());
        } catch (const Pandaception &e) {
            threw = true;
            message = e.englishMessage();
        }
    }
    Application::migrationEnabled = false;

    QVERIFY2(threw, "Migration must throw when the re-saved file's commit() fails");
    QVERIFY2(message.contains("failed to commit re-saved file"), qPrintable(message));
#endif
}

void TestICUnit::testLoadBoundaryElementProxiesMultiOutputInputPortNames()
{
    // loadBoundaryElement()'s per-port name proxying (nodeInput->setName(srcPort->name())) only
    // runs for a boundary input element with more than one port. InputRotary defaults to 2
    // outputs (minOutputSize == 2), each explicitly named "0"/"1" at construction -- the
    // matching case for a multi-input boundary *output* (Display7/14/16) is already covered
    // elsewhere.
    QTemporaryDir subDir;
    QVERIFY(subDir.isValid());

    QuickCircuitBuilder subBuilder;
    InputRotary rotary;
    QCOMPARE(rotary.outputSize(), 2);
    subBuilder.add(&rotary);
    const QString subPath = subDir.path() + "/rotary_probe.panda";
    QuickTestUtils::savePandaFile(subBuilder, subPath);

    auto ic = std::make_unique<IC>();
    ic->loadFile(subPath, subDir.path());

    QCOMPARE(ic->inputSize(), 2);
    QVERIFY2(ic->inputPort(0)->name().endsWith("0"), qPrintable(ic->inputPort(0)->name()));
    QVERIFY2(ic->inputPort(1)->name().endsWith("1"), qPrintable(ic->inputPort(1)->name()));
}

void TestICUnit::testLoadBoundaryPortsLogsSummaryAtVerbosity()
{
    // loadBoundaryPorts()'s qCDebug(three) summary is gated behind logging category "three",
    // which the shared test setup disables by default (Comment::setVerbosity(-1) in
    // TestUtils.cpp) -- bump verbosity just for this load so the line actually executes, then
    // restore it regardless of outcome.
    QTemporaryDir subDir;
    QVERIFY(subDir.isValid());

    QuickCircuitBuilder subBuilder;
    InputSwitch sw;
    Led led;
    subBuilder.add(&sw, &led);
    subBuilder.connect(&sw, 0, &led, 0);
    const QString subPath = subDir.path() + "/verbosity_probe.panda";
    QuickTestUtils::savePandaFile(subBuilder, subPath);

    Comment::setVerbosity(4); // enables categories 0-3, including "three"
    auto ic = std::make_unique<IC>();
    try {
        ic->loadFile(subPath, subDir.path());
    } catch (...) {
        Comment::setVerbosity(-1);
        throw;
    }
    Comment::setVerbosity(-1);

    QCOMPARE(ic->inputSize(), 1);
    QCOMPARE(ic->outputSize(), 1);
}

void TestICUnit::testDeserializeAndLoadEnforcesNestingDepthLimit()
{
    // ICLoader::deserializeAndLoad()'s own nesting-depth guard -- the blob-cache path's
    // counterpart to loadFileDirectly()'s identical-purpose check. Build a chain of blobs,
    // each embedding only its immediate predecessor's bytes directly into a local blob map
    // (bypassing ICRegistry::registerBlob()'s own, separate self-containment depth cap --
    // which would otherwise reject a chain this long before it ever reaches ICLoader; there is
    // no Scene-owned ICRegistry available here, so this test manages the blob map itself
    // instead).
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QuickCircuitBuilder leafBuilder;
    InputSwitch sw;
    Led led;
    leafBuilder.add(&sw, &led);
    leafBuilder.connect(&sw, 0, &led, 0);
    const QString leafPath = tempDir.path() + "/level0.panda";
    QuickTestUtils::savePandaFile(leafBuilder, leafPath);

    QFile leafFile(leafPath);
    QVERIFY(leafFile.open(QIODevice::ReadOnly));
    QByteArray prevBytes = leafFile.readAll();
    leafFile.close();
    QString prevKey = QStringLiteral("level0");

    constexpr int kChainLength = 20; // > ICLoader's kMaxICNestingDepth (16)
    bool threw = false;
    QString message;

    for (int level = 1; level <= kChainLength; ++level) {
        QuickCircuitBuilder levelBuilder;
        auto *ic = new IC;
        levelBuilder.addOwnedElement(ic);

        try {
            ic->loadFromBlob(prevBytes, tempDir.path());
        } catch (const Pandaception &e) {
            threw = true;
            message = e.englishMessage();
            break;
        }
        ic->setBlobName(prevKey);

        const QString levelPath = tempDir.path() + QString("/level%1.panda").arg(level);
        QMap<QString, QByteArray> blobRegistry;
        blobRegistry[prevKey] = prevBytes;
        QuickTestUtils::savePandaFile(levelBuilder, levelPath, blobRegistry);

        QFile levelFile(levelPath);
        QVERIFY(levelFile.open(QIODevice::ReadOnly));
        prevBytes = levelFile.readAll();
        levelFile.close();
        prevKey = QString("level%1").arg(level);
    }

    QVERIFY2(threw, "A chain of 20 nested embedded-IC blobs must eventually throw a "
                     "nesting-depth exception, not crash or succeed unbounded.");
    QVERIFY2(message.contains("nesting depth", Qt::CaseInsensitive), qPrintable(message));
}
