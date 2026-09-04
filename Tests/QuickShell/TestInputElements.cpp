// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestInputElements.h"

#include <memory>

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QMouseEvent>
#include <QTemporaryDir>
#include <QTest>

#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/InputButton.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/IO/SerializationContext.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/Wiring/Port.h"

// ============================================================================
// InputSwitch Tests
// ============================================================================

void TestInputElements::testInputSwitchConstructor()
{
    InputSwitch inputSwitch;

    // Verify element type
    QCOMPARE(inputSwitch.elementType(), ElementType::InputSwitch);

    // Verify element group is Input
    QCOMPARE(inputSwitch.elementGroup(), ElementGroup::Input);

    // Verify port configuration (0 inputs, 1 output)
    QCOMPARE(inputSwitch.inputSize(), 0);
    QCOMPARE(inputSwitch.outputSize(), 1);

    // Verify can change appearance
    QVERIFY(inputSwitch.canChangeAppearance());

    // Verify has label
    QVERIFY(inputSwitch.hasLabel());

    // Verify has trigger
    QVERIFY(inputSwitch.hasTrigger());

    // Verify cannot rotate
    QVERIFY(!inputSwitch.rotatesGraphic());
}

void TestInputElements::testInputSwitchInitialState()
{
    InputSwitch inputSwitch;

    // Initial state should be OFF
    QVERIFY(!inputSwitch.isOn());

    // Output port should be Inactive initially
    QCOMPARE(inputSwitch.outputPort(0)->status(), Status::Inactive);
}

void TestInputElements::testInputSwitchSetOnToggle()
{
    InputSwitch inputSwitch;

    // Start: OFF
    QVERIFY(!inputSwitch.isOn());

    // setOn() toggles from OFF to ON
    inputSwitch.setOn();
    QVERIFY(inputSwitch.isOn());

    // setOn() toggles from ON to OFF
    inputSwitch.setOn();
    QVERIFY(!inputSwitch.isOn());

    // setOn() toggles back to ON
    inputSwitch.setOn();
    QVERIFY(inputSwitch.isOn());
}

void TestInputElements::testInputSwitchSetOffToggle()
{
    InputSwitch inputSwitch;

    // Start: OFF
    QVERIFY(!inputSwitch.isOn());

    // setOff() toggles from OFF to ON
    inputSwitch.setOff();
    QVERIFY(inputSwitch.isOn());

    // setOff() toggles from ON to OFF
    inputSwitch.setOff();
    QVERIFY(!inputSwitch.isOn());

    // setOff() toggles back to ON
    inputSwitch.setOff();
    QVERIFY(inputSwitch.isOn());
}

void TestInputElements::testInputSwitchIsOn()
{
    InputSwitch inputSwitch;

    // Port parameter is ignored
    QVERIFY(!inputSwitch.isOn(0));
    QVERIFY(!inputSwitch.isOn(1));
    QVERIFY(!inputSwitch.isOn(-1));

    // After setOn
    inputSwitch.setOn(true);
    QVERIFY(inputSwitch.isOn());
    QVERIFY(inputSwitch.isOn(0));
    QVERIFY(inputSwitch.isOn(99));  // Port parameter ignored
}

void TestInputElements::testInputSwitchPortStatus()
{
    InputSwitch inputSwitch;

    // Initial: Inactive
    QCOMPARE(inputSwitch.outputPort(0)->status(), Status::Inactive);

    // After setOn(true)
    inputSwitch.setOn(true);
    QCOMPARE(inputSwitch.outputPort(0)->status(), Status::Active);

    // After setOn(false)
    inputSwitch.setOn(false);
    QCOMPARE(inputSwitch.outputPort(0)->status(), Status::Inactive);
}

void TestInputElements::testInputSwitchSaveState()
{
    InputSwitch inputSwitch;
    inputSwitch.setOn(true);

    // Save to stream
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    inputSwitch.save(stream, {.purpose = SerializationPurpose::PortableFile});

    // Verify data was written
    QVERIFY(data.size() > 0);
}

void TestInputElements::testInputSwitchLoadOldVersion()
{
    // Create and save InputSwitch with version 4.1+ format
    auto inputSwitch1 = std::unique_ptr<InputSwitch>(new InputSwitch());
    inputSwitch1->setOn(true);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    inputSwitch1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});
    inputSwitch1.reset();

    // Load with old version (< 4.1) - will try to read old format
    auto inputSwitch2 = std::unique_ptr<InputSwitch>(new InputSwitch());

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    // For versions < 4.1, loadOldFormat reads positional fields.  The saved data
    // is in QMap format, so readBoundedString rejects the map-count bytes as an
    // oversized string → PANDACEPTION is the expected result of this format mismatch.
    SerializationContext context = {portMap, QVersionNumber(3, 0), SerializationPurpose::PortableFile, {}};
    bool threw = false;
    try {
        inputSwitch2->load(loadStream, context);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY(threw); // format mismatch now throws rather than silently reading garbage
    // Element type is always set by the constructor, not the stream
    QVERIFY2(inputSwitch2->elementType() == ElementType::InputSwitch, "Element type should be preserved");
}

void TestInputElements::testInputSwitchLoadNewVersion()
{
    // Create and save InputSwitch
    auto inputSwitch1 = std::unique_ptr<InputSwitch>(new InputSwitch());
    inputSwitch1->setOn(true);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    inputSwitch1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});
    inputSwitch1.reset();

    // Load with new version (>= 4.1)
    auto inputSwitch2 = std::unique_ptr<InputSwitch>(new InputSwitch());

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, QVersionNumber(4, 1), SerializationPurpose::PortableFile, {}};

    inputSwitch2->load(loadStream, context);

    // State should be preserved
    QVERIFY(inputSwitch2->isOn());
}

void TestInputElements::testInputSwitchLockingMechanism()
{
    // setOn(bool) is the direct/programmatic API and deliberately ignores m_locked -- only
    // mousePressEvent()'s interactive path checks the lock. Actually engage the lock here so
    // the "doesn't affect setOn(bool) directly" claim is demonstrated, not just asserted by
    // a comment.
    InputSwitch inputSwitch;
    inputSwitch.setLocked(true);
    QVERIFY(inputSwitch.isLocked());

    inputSwitch.setOn(false);
    QVERIFY(!inputSwitch.isOn());

    inputSwitch.setOn(true);
    QVERIFY(inputSwitch.isOn());

    inputSwitch.setOn(false);
    QVERIFY(!inputSwitch.isOn());

    inputSwitch.setLocked(false);
    QVERIFY(!inputSwitch.isLocked());
}

void TestInputElements::testInputSwitchMousePressWhileLockedDoesNotToggle()
{
    // Scene + QGraphicsSceneMouseEvent (Widgets) -> CanvasItem + a real QMouseEvent sent via
    // QCoreApplication::sendEvent(), matching TestInputRotary.cpp's own mouse-press tests.
    // CanvasAddItemsCommand (not a bare addItem()) is what populates the spatial index
    // mousePressEvent()'s hit test reads -- see TestInputRotary.cpp's comment on that.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *inputSwitch = new InputSwitch;
    inputSwitch->setPos(0, 0);
    inputSwitch->setLocked(true);
    canvas.receiveCommand(new CanvasAddItemsCommand({inputSwitch}, &canvas));

    QVERIFY(!inputSwitch->isOn());

    const QPointF worldPos = inputSwitch->pos() + QPointF(10, 10);
    const QPointF screenPos = canvas.worldToScreen(worldPos);
    QMouseEvent pressEvent(QEvent::MouseButtonPress, screenPos, screenPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&canvas, &pressEvent);

    QVERIFY2(!inputSwitch->isOn(), "mousePressEvent() must not toggle a locked InputSwitch");
}

void TestInputElements::testInputSwitchAppearanceChange()
{
    InputSwitch inputSwitch;

    // Switch is OFF initially
    inputSwitch.setOn(false);

    // Reset to default appearance (should not crash)
    inputSwitch.setAppearance(true, "");

    // Verify state is preserved
    QVERIFY(!inputSwitch.isOn());

    // Switch to ON
    inputSwitch.setOn(true);
    QVERIFY(inputSwitch.isOn());

    // Reset to default appearance again
    inputSwitch.setAppearance(true, "");

    // Verify state still ON
    QVERIFY(inputSwitch.isOn());
}

// ============================================================================
// InputButton Tests
// ============================================================================

void TestInputElements::testInputButtonConstructor()
{
    InputButton inputButton;

    // Verify element type
    QCOMPARE(inputButton.elementType(), ElementType::InputButton);

    // Verify element group is Input
    QCOMPARE(inputButton.elementGroup(), ElementGroup::Input);

    // Verify port configuration (0 inputs, 1 output)
    QCOMPARE(inputButton.inputSize(), 0);
    QCOMPARE(inputButton.outputSize(), 1);

    // Verify can change appearance
    QVERIFY(inputButton.canChangeAppearance());

    // Verify has label
    QVERIFY(inputButton.hasLabel());

    // Verify has trigger
    QVERIFY(inputButton.hasTrigger());

    // Verify cannot rotate
    QVERIFY(!inputButton.rotatesGraphic());
}

void TestInputElements::testInputButtonInitialState()
{
    InputButton inputButton;

    // Initial state should be OFF (set in constructor)
    QVERIFY(!inputButton.isOn());

    // Output port should be Inactive initially
    QCOMPARE(inputButton.outputPort(0)->status(), Status::Inactive);
}

void TestInputElements::testInputButtonSetOn()
{
    InputButton inputButton;

    // Start: OFF
    QVERIFY(!inputButton.isOn());

    // setOn() sets to ON (no toggle)
    inputButton.setOn();
    QVERIFY(inputButton.isOn());

    // setOn() again - still ON (no toggle)
    inputButton.setOn();
    QVERIFY(inputButton.isOn());
}

void TestInputElements::testInputButtonSetOff()
{
    InputButton inputButton;

    // Set to ON first
    inputButton.setOn();
    QVERIFY(inputButton.isOn());

    // setOff() sets to OFF (no toggle)
    inputButton.setOff();
    QVERIFY(!inputButton.isOn());

    // setOff() again - still OFF (no toggle)
    inputButton.setOff();
    QVERIFY(!inputButton.isOn());
}

void TestInputElements::testInputButtonIsOn()
{
    InputButton inputButton;

    // Port parameter is ignored
    QVERIFY(!inputButton.isOn(0));
    QVERIFY(!inputButton.isOn(1));
    QVERIFY(!inputButton.isOn(-1));

    // After setOn
    inputButton.setOn();
    QVERIFY(inputButton.isOn());
    QVERIFY(inputButton.isOn(0));
    QVERIFY(inputButton.isOn(99));  // Port parameter ignored
}

void TestInputElements::testInputButtonPortStatus()
{
    InputButton inputButton;

    // Initial: Inactive
    QCOMPARE(inputButton.outputPort(0)->status(), Status::Inactive);

    // After setOn
    inputButton.setOn();
    QCOMPARE(inputButton.outputPort(0)->status(), Status::Active);

    // After setOff
    inputButton.setOff();
    QCOMPARE(inputButton.outputPort(0)->status(), Status::Inactive);
}

void TestInputElements::testInputButtonSaveState()
{
    InputButton inputButton;

    // Save to stream
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    inputButton.save(stream, {.purpose = SerializationPurpose::PortableFile});

    // Verify data was written
    QVERIFY(data.size() > 0);
}

void TestInputElements::testInputButtonLoadOldVersion()
{
    // Create and save InputButton
    auto inputButton1 = std::unique_ptr<InputButton>(new InputButton());
    inputButton1->setOn();

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    inputButton1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});
    inputButton1.reset();

    // Load with old version (3.1 - 4.0)
    auto inputButton2 = std::unique_ptr<InputButton>(new InputButton());

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    // Save wrote QMap format; loadOldFormat reads positional fields → format mismatch.
    // readBoundedString rejects the map-count bytes as an oversized string → throws.
    SerializationContext context = {portMap, QVersionNumber(3, 5), SerializationPurpose::PortableFile, {}};
    bool threw = false;
    try {
        inputButton2->load(loadStream, context);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY(threw); // format mismatch now throws rather than silently reading garbage
    QVERIFY2(inputButton2->elementType() == ElementType::InputButton, "Element type should be preserved");
}

void TestInputElements::testInputButtonLoadNewVersion()
{
    // Create and save InputButton
    auto inputButton1 = std::unique_ptr<InputButton>(new InputButton());
    inputButton1->setOn();

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    inputButton1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});
    inputButton1.reset();

    // Load with new version (>= 4.1)
    auto inputButton2 = std::unique_ptr<InputButton>(new InputButton());

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, QVersionNumber(4, 1), SerializationPurpose::PortableFile, {}};

    inputButton2->load(loadStream, context);

    // Verify the element is in a valid state after loading
    QVERIFY2(inputButton2->outputSize() >= 1, "InputButton must have at least 1 output after load");
    QVERIFY2(inputButton2->elementType() == ElementType::InputButton, "Element type should be preserved");
}

void TestInputElements::testInputButtonLockingMechanism()
{
    // setOn()/setOff() are the direct/programmatic API and deliberately ignore m_locked --
    // only mousePressEvent()/mouseReleaseEvent()'s interactive paths check the lock. Actually
    // engage the lock here so that claim is demonstrated, not just asserted by a comment.
    InputButton inputButton;
    inputButton.setLocked(true);
    QVERIFY(inputButton.isLocked());

    inputButton.setOff();
    QVERIFY(!inputButton.isOn());

    inputButton.setOn();
    QVERIFY(inputButton.isOn());

    inputButton.setOff();
    QVERIFY(!inputButton.isOn());

    inputButton.setLocked(false);
    QVERIFY(!inputButton.isLocked());
}

void TestInputElements::testInputButtonMousePressReleaseWhileLockedDoesNotToggle()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *inputButton = new InputButton;
    inputButton->setPos(0, 0);
    inputButton->setLocked(true);
    canvas.receiveCommand(new CanvasAddItemsCommand({inputButton}, &canvas));

    QVERIFY(!inputButton->isOn());

    const QPointF worldPos = inputButton->pos() + QPointF(10, 10);
    const QPointF screenPos = canvas.worldToScreen(worldPos);

    QMouseEvent pressEvent(QEvent::MouseButtonPress, screenPos, screenPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&canvas, &pressEvent);
    QVERIFY2(!inputButton->isOn(), "mousePressEvent() must not turn on a locked InputButton");

    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, screenPos, screenPos, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&canvas, &releaseEvent);
    QVERIFY2(!inputButton->isOn(), "mouseReleaseEvent() must not change a locked InputButton");
}

void TestInputElements::testInputButtonAppearanceChange()
{
    InputButton inputButton;

    // Button is OFF initially
    inputButton.setOff();

    // Reset to default appearance (should not crash)
    inputButton.setAppearance(true, "");

    // Verify state is preserved
    QVERIFY(!inputButton.isOn());

    // Turn button ON
    inputButton.setOn();
    QVERIFY(inputButton.isOn());

    // Reset to default appearance again
    inputButton.setAppearance(true, "");

    // Verify state still ON
    QVERIFY(inputButton.isOn());
}

// ============================================================================
// Basic Input Element Tests (migrated from testelements.cpp)
// ============================================================================

void TestInputElements::testInputButton()
{
    InputButton elm;
    QCOMPARE(elm.inputSize(), 0);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.elementType(), ElementType::InputButton);

    // Test initial state (button starts off)
    QCOMPARE(elm.isOn(), false);

    // Test state changes with explicit value
    elm.setOn(true);
    QCOMPARE(elm.isOn(), true);
    QCOMPARE(elm.outputPort()->status(), Status::Active);

    elm.setOn(false);
    QCOMPARE(elm.isOn(), false);
    QCOMPARE(elm.outputPort()->status(), Status::Inactive);
}

void TestInputElements::testInputSwitch()
{
    InputSwitch elm;
    QCOMPARE(elm.inputSize(), 0);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.elementType(), ElementType::InputSwitch);

    // Test initial state
    QCOMPARE(elm.isOn(), false);

    // Test state changes with explicit value
    elm.setOn(true);
    QCOMPARE(elm.isOn(), true);
    QCOMPARE(elm.outputPort()->status(), Status::Active);

    elm.setOn(false);
    QCOMPARE(elm.isOn(), false);
    QCOMPARE(elm.outputPort()->status(), Status::Inactive);
}

void TestInputElements::testAppearanceWithSameOsAbsolutePath()
{
    // setAppearance() takes a directly-usable path as-is -- no contextDir resolution of
    // its own (that now happens once, at load time, via ExternalFilePath::forReading());
    // see testLoadResolvesAppearanceBareFilenameAgainstContextDir() below and
    // TestExternalFilePath.cpp for the resolution mechanics themselves.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString appearanceFullPath = tempDir.path() + "/custom_appearance.svg";
    QVERIFY(QFile::copy(":/Components/Input/switchOff.svg", appearanceFullPath));

    InputSwitch inputSwitch;

    bool threw = false;
    try {
        inputSwitch.setAppearance(false, appearanceFullPath);
    } catch (const Pandaception &) {
        threw = true;
    }

    QVERIFY2(!threw, "setAppearance should accept a direct, existing absolute path");
}

void TestInputElements::testAppearanceWithNonExistentFileFallback()
{
    InputSwitch inputSwitch;

    bool threw = false;
    try {
        inputSwitch.setAppearance(false, "/some/empty/directory/nonexistent_appearance_12345.svg");
    } catch (const Pandaception &) {
        threw = true;
    }

    QVERIFY2(threw, "setAppearance should throw when the given path does not exist");
}

void TestInputElements::testLoadResolvesAppearanceBareFilenameAgainstContextDir()
{
    // Integration check that GraphicElementSerializer's appearance-restore loop is wired
    // to ExternalFilePath::forReading(): a PortableFile save() strips a non-resource path
    // to a bare filename; loading it back with contextDir pointing at that file's
    // directory must resolve the full path and actually reload the pixmap.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString appearanceFullPath = tempDir.path() + "/custom_appearance.svg";
    QVERIFY(QFile::copy(":/Components/Input/switchOff.svg", appearanceFullPath));

    InputSwitch inputSwitch1;
    inputSwitch1.setAppearance(false, appearanceFullPath);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    inputSwitch1.save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    InputSwitch inputSwitch2;
    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, QVersionNumber(4, 1), SerializationPurpose::PortableFile, tempDir.path()};
    inputSwitch2.load(loadStream, context);

    QCOMPARE(inputSwitch2.externalFiles(), QStringList{appearanceFullPath});
}

void TestInputElements::testLoadResolvesAppearanceForeignPathViaBareFilenameFallback()
{
    // A stored Windows-style absolute path (saved on a different OS/machine) doesn't
    // exist locally; load() must fall back to a bare-filename lookup in contextDir.
    // See TestExternalFilePath.cpp for the exhaustive separator matrix -- this just
    // confirms the appearance-restore path actually exercises that fallback.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString appearanceFileName = "custom_appearance.svg";
    const QString appearanceFullPath = tempDir.path() + "/" + appearanceFileName;
    QVERIFY(QFile::copy(":/Components/Input/switchOff.svg", appearanceFullPath));

    InputSwitch inputSwitch1;
    inputSwitch1.setAppearance(false, appearanceFullPath);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    inputSwitch1.save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Rewrite the stored appearance skinName to a foreign, locally-nonexistent
    // Windows-style absolute path with the same bare filename.
    QMap<QString, QVariant> propsMap;
    QList<QMap<QString, QVariant>> inputPorts, outputPorts, appearancesMap;
    QMap<QString, QVariant> extraMap;
    {
        QDataStream readStream(data);
        readStream >> propsMap >> inputPorts >> outputPorts >> appearancesMap >> extraMap;
    }
    QVERIFY(!appearancesMap.isEmpty());
    appearancesMap[0]["skinName"] = "C:\\Users\\alice\\project\\" + appearanceFileName;

    QByteArray rewritten;
    {
        QDataStream writeStream(&rewritten, QIODevice::WriteOnly);
        writeStream << propsMap << inputPorts << outputPorts << appearancesMap << extraMap;
    }

    InputSwitch inputSwitch2;
    QDataStream loadStream(rewritten);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, QVersionNumber(4, 1), SerializationPurpose::PortableFile, tempDir.path()};
    inputSwitch2.load(loadStream, context);

    QCOMPARE(inputSwitch2.externalFiles(), QStringList{appearanceFullPath});
}

void TestInputElements::testAppearanceReloadsAfterFileModified()
{
    // Regression: ElementAppearance used to keep its own base-pixmap cache keyed on the bare
    // resolved path, so a *different* element loading the same path after the file's content
    // changed on disk (e.g. a fresh element, or a reopened project) kept serving the stale,
    // first-loaded pixmap out of that cache. setPixmap() now relies entirely on
    // QPixmap::load()'s own internal caching, which keys on path + mtime + size, so a genuinely
    // different file at the same path is picked up. Uses two separate elements rather than
    // re-setting the same path twice on one element, since setPixmap() already short-circuits
    // a same-path re-set on the *same* element (a different, unrelated optimization) — the bug
    // this guards against is about a fresh load seeing stale cross-element cache data, not
    // about redundant re-sets on one element. Sizes with different digit counts (9 vs 200) so
    // the two files differ in byte length too — Qt's own load() cache keys on path + mtime +
    // size, and two writes landing within the same filesystem mtime tick would otherwise defeat
    // the test by coincidence if both files happened to be the same size.
    //
    // WorkSpace+Scene (Widgets) -> CanvasItem directly: it owns contextDir()/setContextDir()
    // and addItem() itself, with no separate workspace object needed.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = tempDir.path() + "/reloadable.svg";

    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setContextDir(tempDir.path());

    auto writeSvg = [](const QString &filePath, int side) {
        QFile file(filePath);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        const QByteArray svg = QString("<svg xmlns=\"http://www.w3.org/2000/svg\" "
                                        "width=\"%1\" height=\"%1\"><rect width=\"%1\" height=\"%1\" "
                                        "fill=\"red\"/></svg>").arg(side).toUtf8();
        QCOMPARE(file.write(svg), svg.size());
    };

    writeSvg(path, 9);
    auto *firstSwitch = new InputSwitch();
    canvas.addItem(firstSwitch);
    firstSwitch->setAppearance(false, path);
    const QSizeF firstSize = firstSwitch->boundingRect().size();

    writeSvg(path, 200);
    auto *secondSwitch = new InputSwitch();
    canvas.addItem(secondSwitch);
    secondSwitch->setAppearance(false, path);
    const QSizeF secondSize = secondSwitch->boundingRect().size();

    QVERIFY2(firstSize != secondSize,
             "A fresh element loading the same path after the file changed on disk must not "
             "reuse another element's stale cached pixmap");
}

void TestInputElements::testRotarySelectionIsResetAndRestoredWithSimState()
{
    auto *rotary = qobject_cast<InputRotary *>(ElementFactory::buildElement(ElementType::InputRotary));
    QVERIFY(rotary != nullptr);
    rotary->setOutputSize(4);
    rotary->initSimulationVectors(rotary->inputSize(), rotary->outputSize());

    rotary->setOn(true, 2);                 // as a user click would
    // A live tick is what puts the selection into the SIMULATION outputs; setOn() only writes
    // m_currentPort and the port statuses. sweep() snapshots after the simulation has been
    // running, so this mirrors the real ordering rather than saving an empty state.
    rotary->updateOutputs();
    QVERIFY(rotary->isOn(2));
    QVERIFY(!rotary->isOn(0));

    // Snapshot / reset / restore, exactly the order WaveformSimulator::sweep() uses.
    QVector<Status> saved;
    rotary->saveSimState(saved);

    rotary->resetSimState();
    QVERIFY2(rotary->isOn(0),
             "the sweep's reset must return the rotary to its power-on position, or every run "
             "inherits the live one");
    QVERIFY(!rotary->isOn(2));

    int cursor = 0;
    rotary->restoreSimState(saved, cursor);
    QVERIFY2(rotary->isOn(2),
             "and the restore must put the live selection back, or a sweep leaves the canvas "
             "rotated to wherever its last column landed");
    QVERIFY(!rotary->isOn(0));

    delete rotary;
}
