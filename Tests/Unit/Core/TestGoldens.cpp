// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestGoldens.h"

#include <memory>
#include <utility>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaEnum>
#include <QSet>
#include <QTemporaryDir>
#include <QTest>
#include <QTransform>

#include "App/Core/Application.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

namespace {

constexpr char kRegenerate[] = "GENERATE_EXPECTED_SNAPSHOTS";

/// Sets the regeneration switch to \a value for the guard's lifetime, then restores what the process had.
struct ScopedRegeneration {
    const bool wasSet = qEnvironmentVariableIsSet(kRegenerate);
    const QByteArray previous = qgetenv(kRegenerate);

    explicit ScopedRegeneration(const QByteArray &value)
    {
        qputenv(kRegenerate, value);
    }

    ~ScopedRegeneration()
    {
        if (wasSet) {
            qputenv(kRegenerate, previous);
        } else {
            qunsetenv(kRegenerate);
        }
    }
};

/// Writes \a bytes to \a path.
bool writeGoldenFixture(const QString &path, const QByteArray &bytes)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) && (file.write(bytes) == bytes.size());
}

/// Reads the bytes of \a path.
QByteArray readGoldenFixture(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

/// A directory no test creates, for the missing-input row.
QString missingDirectory()
{
    return QDir::temp().filePath("wiredpanda-goldens-no-such-directory");
}

} // namespace

void TestGoldens::testTextGoldenMatchesOrNamesTheLine()
{
    QVERIFY(!TestUtils::regeneratingGoldens());
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString golden = dir.filePath("dump.txt");
    QVERIFY(writeGoldenFixture(golden, "first\nsecond\nthird\n"));

    QCOMPARE(TestUtils::compareWithGolden(golden, "first\nsecond\nthird\n"), QString());

    const QString difference = TestUtils::compareWithGolden(golden, "first\nchanged\nthird\n");
    QVERIFY2(difference.contains(golden), qPrintable(difference));
    QVERIFY2(difference.contains("line 2"), qPrintable(difference));
    QVERIFY2(difference.contains("expected: second"), qPrintable(difference));
    QVERIFY2(difference.contains("actual:   changed"), qPrintable(difference));

    // A dump that stops early names the golden's next line against the end of the dump.
    const QString shorter = TestUtils::compareWithGolden(golden, "first");
    QVERIFY2(shorter.contains("<end of file>"), qPrintable(shorter));
}

void TestGoldens::testByteGoldenMatchesOrNamesTheOffset()
{
    QVERIFY(!TestUtils::regeneratingGoldens());
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString golden = dir.filePath("payload.bin");
    const QByteArray bytes("\x00\x01\x02\x03", 4);
    QVERIFY(writeGoldenFixture(golden, bytes));

    QCOMPARE(TestUtils::compareWithGoldenBytes(golden, bytes), QString());

    QByteArray changed = bytes;
    changed[2] = '\x7f';
    const QString difference = TestUtils::compareWithGoldenBytes(golden, changed);
    QVERIFY2(difference.contains(golden), qPrintable(difference));
    QVERIFY2(difference.contains("first difference at byte 2"), qPrintable(difference));
}

void TestGoldens::testMissingGoldenFails()
{
    // A missing golden fails rather than being written, in both forms, and names the file.
    QVERIFY(!TestUtils::regeneratingGoldens());
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString golden = dir.filePath("absent.txt");

    const QString text = TestUtils::compareWithGolden(golden, "anything\n");
    QVERIFY2(text.contains("missing") && text.contains(golden), qPrintable(text));
    const QString bytes = TestUtils::compareWithGoldenBytes(golden, "anything");
    QVERIFY2(bytes.contains("missing") && bytes.contains(golden), qPrintable(bytes));
    QVERIFY(!QFile::exists(golden));
}

void TestGoldens::testRegenerationWritesOverAnExistingGolden()
{
    // Under the switch both forms write the golden even when one exists, creating its directory when it is new.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString textGolden = dir.filePath("dump.txt");
    const QString byteGolden = dir.filePath("payload.bin");
    const QString newGolden = dir.filePath("new/directory/dump.txt");
    QVERIFY(writeGoldenFixture(textGolden, "old\n"));
    QVERIFY(writeGoldenFixture(byteGolden, "old bytes"));

    {
        const ScopedRegeneration regeneration("1");
        QCOMPARE(TestUtils::compareWithGolden(textGolden, "new\n"), QString());
        QCOMPARE(TestUtils::compareWithGoldenBytes(byteGolden, "new bytes"), QString());
        QCOMPARE(TestUtils::compareWithGolden(newGolden, "created\n"), QString());
    }

    QCOMPARE(readGoldenFixture(textGolden), QByteArray("new\n"));
    QCOMPARE(readGoldenFixture(byteGolden), QByteArray("new bytes"));
    QCOMPARE(readGoldenFixture(newGolden), QByteArray("created\n"));
    QCOMPARE(TestUtils::compareWithGolden(textGolden, "new\n"), QString());
    QCOMPARE(TestUtils::compareWithGoldenBytes(byteGolden, "new bytes"), QString());
}

void TestGoldens::testRegenerationSwitchCountsWhenEmpty()
{
    QVERIFY(!TestUtils::regeneratingGoldens());
    const ScopedRegeneration regeneration("");
#if defined(Q_OS_WIN)
    // Windows keeps no variable with an empty value: setting one removes it, so there the switch is off.
    QVERIFY(!TestUtils::regeneratingGoldens());
#else
    QVERIFY(TestUtils::regeneratingGoldens());
#endif
}

void TestGoldens::testFileRows_data()
{
    QTest::addColumn<QString>("path");
    TestUtils::addFileRows(TestUtils::examplesDir(), {"*.panda"});
    TestUtils::addFileRows(TestUtils::backwardCompatibilityDir(), {"*.panda"}, true, "compat/");
    TestUtils::addFileRows(missingDirectory(), {"*.panda"});
}

void TestGoldens::testFileRows()
{
    // Each example is a row of its own; the backward-compatibility files, found in their version subdirectories, are
    // rows named by the prefix and their relative path; the missing directory is one row, which fails, naming it.
    QFETCH(QString, path);
    const QString error = TestUtils::requireInputFile(path);
    const QString tag(QTest::currentDataTag());
    if (path == missingDirectory()) {
        QCOMPARE(tag, "missing directory " + missingDirectory());
        QVERIFY2(error.contains(missingDirectory()), qPrintable(error));
    } else if (tag.startsWith("compat/")) {
        QVERIFY2(error.isEmpty(), qPrintable(error));
        const QString relative = QDir(TestUtils::backwardCompatibilityDir()).relativeFilePath(path);
        QVERIFY2(relative.contains('/'), qPrintable(relative));
        QCOMPARE(tag, "compat/" + relative);
    } else {
        QVERIFY2(error.isEmpty(), qPrintable(error));
        QVERIFY(path.endsWith(".panda"));
        QCOMPARE(QString(QTest::currentDataTag()), QFileInfo(path).fileName());
    }
}

void TestGoldens::testLoadCircuit()
{
    // A file in an older format loads in full, and nothing is written beside it: no backup, no re-save, even with
    // migration on.
    const QString source = TestUtils::backwardCompatibilityDir() + "v1.8-beta/input.panda";
    QFile original(source);
    QVERIFY2(original.open(QIODevice::ReadOnly), qPrintable(source));
    const QByteArray bytes = original.readAll();
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString copy = dir.filePath("input.panda");
    QVERIFY(QFile::copy(source, copy));

    const bool migration = Application::migrationEnabled;
    Application::migrationEnabled = true;
    WorkSpace workspace;
    TestUtils::loadCircuit(workspace, copy);
    Application::migrationEnabled = migration;

    QVERIFY(!TestUtils::elementsById(workspace.scene()).isEmpty());
    QCOMPARE(QDir(dir.path()).entryList(QDir::Files), QStringList{"input.panda"});
    QFile reread(copy);
    QVERIFY(reread.open(QIODevice::ReadOnly));
    QCOMPARE(reread.readAll(), bytes);
}

void TestGoldens::testPrinters()
{
    QCOMPARE(TestUtils::snapshotNumber(1.5), QString("1.5000"));
    QCOMPARE(TestUtils::snapshotNumber(-0.0), QString("0.0000"));
    QCOMPARE(TestUtils::snapshotNumber(-0.00001), QString("0.0000"));
    QCOMPARE(TestUtils::snapshotNumber(-2.25), QString("-2.2500"));
    QCOMPARE(TestUtils::snapshotPoint(QPointF(-0.0, 32)), QString("0.0000,32.0000"));
    QCOMPARE(TestUtils::snapshotTransform(QTransform(0, 1, -1, -0.0, 2.5, -8)),
             QString("0.0000,1.0000,-1.0000,0.0000,2.5000,-8.0000"));
    QCOMPARE(TestUtils::snapshotQuoted(" a\"b\\c\nd\te "), QString(R"(" a\"b\\c\nd\te ")"));
    QCOMPARE(TestUtils::snapshotStatus(Status::Unknown), QString("Unknown"));
    QCOMPARE(TestUtils::snapshotStatus(Status::Inactive), QString("Inactive"));
    QCOMPARE(TestUtils::snapshotStatus(Status::Active), QString("Active"));
    QCOMPARE(TestUtils::snapshotStatus(Status::Error), QString("Error"));
    QCOMPARE(TestUtils::snapshotType(ElementType::DFlipFlop), QString("DFlipFlop"));
}

void TestGoldens::testOrientations()
{
    const auto orientations = TestUtils::orientations();
    QCOMPARE(orientations.size(), 16);
    QSet<QString> names;
    for (const auto &orientation : orientations) {
        names.insert(TestUtils::snapshotOrientation(orientation));
    }
    QCOMPARE(names.size(), 16);
    QVERIFY(names.contains("r0"));
    QVERIFY(names.contains("r270xy"));

    And gate;
    TestUtils::orient(&gate, {180, true, false});
    QCOMPARE(gate.rotation(), 180.0);
    QVERIFY(gate.isFlippedX());
    QVERIFY(!gate.isFlippedY());
}

void TestGoldens::testElementsById()
{
    // The order a walk of the scene gives is the scene's own; the dumps list elements by id instead.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *led = new Led();
    auto *sw = new InputSwitch();
    auto *gate = new And();
    scene->addItem(led);
    scene->addItem(sw);
    scene->addItem(gate);
    scene->removeItem(led);
    scene->addItem(led);

    const auto elements = TestUtils::elementsById(scene);
    QCOMPARE(elements.size(), 3);
    QCOMPARE(elements.at(0), led);
    QCOMPARE(elements.at(1), sw);
    QCOMPARE(elements.at(2), gate);
}

void TestGoldens::testScopedRendering()
{
    // The switch is on inside the scope and back to what it was after, whichever value that was.
    const bool original = Application::renderingEnabled;
    for (const bool before : {false, true}) {
        Application::renderingEnabled = before;
        {
            const TestUtils::ScopedRendering rendering;
            QVERIFY(Application::renderingEnabled);
        }
        QCOMPARE(Application::renderingEnabled, before);
    }
    Application::renderingEnabled = original;
}

void TestGoldens::testPortCountsAndOrientationPasses()
{
    // A Demux's inputs follow its outputs, so only its output count is set; other elements take both.
    And gate;
    TestUtils::applyPortCounts(&gate, 3, 1);
    QCOMPARE(gate.inputSize(), 3);
    Demux demux;
    TestUtils::applyPortCounts(&demux, 7, 4);
    QCOMPARE(demux.outputSize(), 4);
    QCOMPARE(demux.inputSize(), 3);

    // One line per orientation under the header, then the summary: the same text when the order changes nothing, and
    // the differing descriptions after the other text when it does.
    WorkSpace workspace;
    const TestUtils::ElementBuilder build = [](Scene *scene) {
        auto element = std::make_unique<And>();
        scene->addItem(element.get());
        return std::unique_ptr<GraphicElement>(std::move(element));
    };
    const auto rotation = [](const GraphicElement *element) {
        return TestUtils::snapshotNumber(element->rotation());
    };
    const auto same = TestUtils::describeInOrientations(workspace.scene(), "header", build, [](GraphicElement *) {},
                                                        rotation, "same", "different");
    QCOMPARE(same.size(), 18);
    QCOMPARE(same.first(), QString("header"));
    QCOMPARE(same.last(), QString("same"));

    // A port pass that undoes the rotation makes every oriented-first description differ but the unrotated ones.
    const auto different = TestUtils::describeInOrientations(workspace.scene(), "header", build,
                                                             [](GraphicElement *element) { element->setRotation(0); },
                                                             rotation, "same", "different");
    QCOMPARE(different.size(), 1 + 16 + 1 + 12);
    QCOMPARE(different.at(17), QString("different"));
}

void TestGoldens::testLeftOutTypes()
{
    // The types the dumps leave out are named in one list. JKLatch and Unknown have no creator; an IC has one, but a
    // bare IC has no file, so ICs are dumped through the circuits that hold them. Every other type has a creator, so
    // a dump never skips one.
    const QMetaEnum metaEnum = QMetaEnum::fromType<ElementType>();
    QList<ElementType> leftOut;
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(metaEnum.value(i));
        if (TestUtils::leftOutOfDumps(type)) {
            leftOut.append(type);
        }
    }
    QCOMPARE(leftOut, (QList<ElementType>{ElementType::IC, ElementType::JKLatch, ElementType::Unknown}));
    QVERIFY(!ElementFactory::hasCreator(ElementType::JKLatch));
    QVERIFY(!ElementFactory::hasCreator(ElementType::Unknown));

    const auto dumped = TestUtils::dumpedTypes();
    QCOMPARE(dumped.size() + leftOut.size(), metaEnum.keyCount());
    for (const ElementType type : dumped) {
        QVERIFY2(ElementFactory::hasCreator(type), qPrintable(TestUtils::snapshotType(type) + ": " + TestUtils::kNoCreator));
    }
}

void TestGoldens::testPixmapIdentity()
{
    // A gate shows its resource picture, the same rotated, since its picture has no text to keep upright. A flip-flop's
    // picture has text, so rotated an oriented variant replaces it. A Mux draws a body to fit its ports.
    And gate;
    const QString upright = SnapshotAccess::pixmapIdentity(&gate);
    QVERIFY2(upright.startsWith("64x64::/"), qPrintable(upright));
    gate.setRotation(90);
    QCOMPARE(SnapshotAccess::pixmapIdentity(&gate), upright);

    DFlipFlop flipFlop;
    const QString unrotated = SnapshotAccess::pixmapIdentity(&flipFlop);
    QVERIFY2(!unrotated.endsWith(":oriented"), qPrintable(unrotated));
    flipFlop.setRotation(90);
    const QString rotated = SnapshotAccess::pixmapIdentity(&flipFlop);
    QVERIFY2(rotated.endsWith(":oriented"), qPrintable(rotated));

    Mux mux;
    QCOMPARE(SnapshotAccess::pixmapIdentity(&mux), QString("footprint"));
}
