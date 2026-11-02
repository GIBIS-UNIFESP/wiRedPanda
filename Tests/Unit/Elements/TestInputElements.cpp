// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestInputElements.h"

#include <memory>

#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QScopeGuard>
#include <QTemporaryDir>
#include <QTest>
#include <QTranslator>

#include "App/Element/ElementFactory.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

// ============================================================================
// InputSwitch Tests
// ============================================================================

void TestInputElements::testInputSwitchConstructor()
{
    Element inputSwitch{ElementType::InputSwitch};

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
    Element inputSwitch{ElementType::InputSwitch};

    // Initial state should be OFF
    QVERIFY(!inputSwitch.isOn());

    // Output port should be Inactive initially
    QCOMPARE(inputSwitch.outputPort(0)->status(), Status::Inactive);
}

void TestInputElements::testInputSwitchPressToggles()
{
    Element inputSwitch{ElementType::InputSwitch};

    // Start: OFF
    QVERIFY(!inputSwitch.isOn());

    // Each press toggles the latched level: on, off, on
    inputSwitch.operate(Operation::Press);
    QVERIFY(inputSwitch.isOn());
    inputSwitch.operate(Operation::Press);
    QVERIFY(!inputSwitch.isOn());
    inputSwitch.operate(Operation::Press);
    QVERIFY(inputSwitch.isOn());
}

void TestInputElements::testInputSwitchSetOnSetsTheLevel()
{
    Element inputSwitch{ElementType::InputSwitch};

    // A level is set, not toggled: the same level twice leaves it so
    inputSwitch.setOn(true);
    QVERIFY(inputSwitch.isOn());
    inputSwitch.setOn(true);
    QVERIFY(inputSwitch.isOn());
    inputSwitch.setOn(false);
    QVERIFY(!inputSwitch.isOn());
    inputSwitch.setOn(false);
    QVERIFY(!inputSwitch.isOn());
}

void TestInputElements::testInputSwitchIsOn()
{
    Element inputSwitch{ElementType::InputSwitch};

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
    Element inputSwitch{ElementType::InputSwitch};

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
    Element inputSwitch{ElementType::InputSwitch};
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
    auto inputSwitch1 = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::InputSwitch));
    inputSwitch1->setOn(true);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    inputSwitch1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});
    inputSwitch1.reset();

    // Load with old version (< 4.1) - will try to read old format
    auto inputSwitch2 = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::InputSwitch));

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
    auto inputSwitch1 = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::InputSwitch));
    inputSwitch1->setOn(true);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    inputSwitch1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});
    inputSwitch1.reset();

    // Load with new version (>= 4.1)
    auto inputSwitch2 = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::InputSwitch));

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
    Element inputSwitch{ElementType::InputSwitch};
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
    Scene scene;
    auto *inputSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    inputSwitch->setPos(0, 0);
    inputSwitch->setLocked(true);
    scene.addItem(inputSwitch);

    QVERIFY(!inputSwitch->isOn());

    QGraphicsSceneMouseEvent pressEvent(QEvent::GraphicsSceneMousePress);
    pressEvent.setScenePos(inputSwitch->scenePos());
    pressEvent.setButton(Qt::LeftButton);
    pressEvent.setButtons(Qt::LeftButton);
    QCoreApplication::sendEvent(&scene, &pressEvent);

    QVERIFY2(!inputSwitch->isOn(), "mousePressEvent() must not toggle a locked InputSwitch");
}

void TestInputElements::testInputSwitchAppearanceChange()
{
    Element inputSwitch{ElementType::InputSwitch};

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
    Element inputButton{ElementType::InputButton};

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
    Element inputButton{ElementType::InputButton};

    // Initial state should be OFF (set in constructor)
    QVERIFY(!inputButton.isOn());

    // Output port should be Inactive initially
    QCOMPARE(inputButton.outputPort(0)->status(), Status::Inactive);
}

void TestInputElements::testInputButtonSetOn()
{
    Element inputButton{ElementType::InputButton};

    // Start: OFF
    QVERIFY(!inputButton.isOn());

    // A high level sets it on
    inputButton.setOn(true);
    QVERIFY(inputButton.isOn());

    // Again: still on
    inputButton.setOn(true);
    QVERIFY(inputButton.isOn());
}

void TestInputElements::testInputButtonSetOff()
{
    Element inputButton{ElementType::InputButton};

    // Set to ON first
    inputButton.setOn(true);
    QVERIFY(inputButton.isOn());

    // A low level sets it off
    inputButton.setOn(false);
    QVERIFY(!inputButton.isOn());

    // Again: still off
    inputButton.setOn(false);
    QVERIFY(!inputButton.isOn());
}

void TestInputElements::testInputButtonIsOn()
{
    Element inputButton{ElementType::InputButton};

    // Port parameter is ignored
    QVERIFY(!inputButton.isOn(0));
    QVERIFY(!inputButton.isOn(1));
    QVERIFY(!inputButton.isOn(-1));

    // After setOn
    inputButton.setOn(true);
    QVERIFY(inputButton.isOn());
    QVERIFY(inputButton.isOn(0));
    QVERIFY(inputButton.isOn(99));  // Port parameter ignored
}

void TestInputElements::testInputButtonPortStatus()
{
    Element inputButton{ElementType::InputButton};

    // Initial: Inactive
    QCOMPARE(inputButton.outputPort(0)->status(), Status::Inactive);

    // After setOn
    inputButton.setOn(true);
    QCOMPARE(inputButton.outputPort(0)->status(), Status::Active);

    // After a low level
    inputButton.setOn(false);
    QCOMPARE(inputButton.outputPort(0)->status(), Status::Inactive);
}

void TestInputElements::testInputButtonSaveState()
{
    Element inputButton{ElementType::InputButton};

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
    auto inputButton1 = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::InputButton));
    inputButton1->setOn(true);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    inputButton1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});
    inputButton1.reset();

    // Load with old version (3.1 - 4.0)
    auto inputButton2 = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::InputButton));

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
    auto inputButton1 = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::InputButton));
    inputButton1->setOn(true);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    inputButton1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});
    inputButton1.reset();

    // Load with new version (>= 4.1)
    auto inputButton2 = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::InputButton));

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
    // setOn() is the direct/programmatic API and deliberately ignores the lock --
    // only mousePressEvent()/mouseReleaseEvent()'s interactive paths check the lock. Actually
    // engage the lock here so that claim is demonstrated, not just asserted by a comment.
    Element inputButton{ElementType::InputButton};
    inputButton.setLocked(true);
    QVERIFY(inputButton.isLocked());

    inputButton.setOn(false);
    QVERIFY(!inputButton.isOn());

    inputButton.setOn(true);
    QVERIFY(inputButton.isOn());

    inputButton.setOn(false);
    QVERIFY(!inputButton.isOn());

    inputButton.setLocked(false);
    QVERIFY(!inputButton.isLocked());
}

void TestInputElements::testInputButtonMousePressReleaseWhileLockedDoesNotToggle()
{
    Scene scene;
    auto *inputButton = ElementFactory::buildElement(ElementType::InputButton);
    inputButton->setPos(0, 0);
    inputButton->setLocked(true);
    scene.addItem(inputButton);

    QVERIFY(!inputButton->isOn());

    QGraphicsSceneMouseEvent pressEvent(QEvent::GraphicsSceneMousePress);
    pressEvent.setScenePos(inputButton->scenePos());
    pressEvent.setButton(Qt::LeftButton);
    pressEvent.setButtons(Qt::LeftButton);
    QCoreApplication::sendEvent(&scene, &pressEvent);
    QVERIFY2(!inputButton->isOn(), "mousePressEvent() must not turn on a locked InputButton");

    QGraphicsSceneMouseEvent releaseEvent(QEvent::GraphicsSceneMouseRelease);
    releaseEvent.setScenePos(inputButton->scenePos());
    releaseEvent.setButton(Qt::LeftButton);
    QCoreApplication::sendEvent(&scene, &releaseEvent);
    QVERIFY2(!inputButton->isOn(), "mouseReleaseEvent() must not change a locked InputButton");
}

void TestInputElements::testInputButtonAppearanceChange()
{
    Element inputButton{ElementType::InputButton};

    // Button is OFF initially
    inputButton.setOn(false);

    // Reset to default appearance (should not crash)
    inputButton.setAppearance(true, "");

    // Verify state is preserved
    QVERIFY(!inputButton.isOn());

    // Turn button ON
    inputButton.setOn(true);
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
    Element elm{ElementType::InputButton};
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
    Element elm{ElementType::InputSwitch};
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
    // Tests/Unit/Core/TestExternalFilePath.cpp for the resolution mechanics themselves.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString appearanceFullPath = tempDir.path() + "/custom_appearance.svg";
    QVERIFY(QFile::copy(":/Components/Input/switchOff.svg", appearanceFullPath));

    Element inputSwitch{ElementType::InputSwitch};

    bool threw = false;
    try {
        inputSwitch.setAppearance(false, appearanceFullPath);
    } catch (const Pandaception &) {
        threw = true;
    }

    QVERIFY2(!threw, "setAppearance should accept a direct, existing absolute path");
}

void TestInputElements::testAppearanceWithNonExistentFileShowsTheDefault()
{
    Element inputSwitch{ElementType::InputSwitch};
    inputSwitch.setAppearance(false, "/some/empty/directory/nonexistent_appearance_12345.svg");

    const Element fresh{ElementType::InputSwitch};
    QCOMPARE(SnapshotAccess::pixmapIdentity(&inputSwitch), SnapshotAccess::pixmapIdentity(&fresh));
}

void TestInputElements::testLoadResolvesAppearanceBareFilenameAgainstContextDir()
{
    // Integration check that ElementSerializer's appearance-restore loop is wired
    // to ExternalFilePath::forReading(): a PortableFile save() strips a non-resource path
    // to a bare filename; loading it back with contextDir pointing at that file's
    // directory must resolve the full path and actually reload the pixmap.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString appearanceFullPath = tempDir.path() + "/custom_appearance.svg";
    QVERIFY(QFile::copy(":/Components/Input/switchOff.svg", appearanceFullPath));

    Element inputSwitch1{ElementType::InputSwitch};
    inputSwitch1.setAppearance(false, appearanceFullPath);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    inputSwitch1.save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    Element inputSwitch2{ElementType::InputSwitch};
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
    // See Tests/Unit/Core/TestExternalFilePath.cpp for the exhaustive separator matrix --
    // this just confirms the appearance-restore path actually exercises that fallback.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString appearanceFileName = "custom_appearance.svg";
    const QString appearanceFullPath = tempDir.path() + "/" + appearanceFileName;
    QVERIFY(QFile::copy(":/Components/Input/switchOff.svg", appearanceFullPath));

    Element inputSwitch1{ElementType::InputSwitch};
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

    Element inputSwitch2{ElementType::InputSwitch};
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
    // first-loaded pixmap out of that cache. The picture library keys each version on
    // path + mtime + size, so a genuinely
    // different file at the same path is picked up. Uses two separate elements rather than
    // re-setting the same path twice on one element, since appearanceChanged() already skips
    // a same-path re-set on the *same* element (a different, unrelated optimization) — the bug
    // this guards against is about a fresh load seeing stale cross-element cache data, not
    // about redundant re-sets on one element. Sizes with different digit counts (9 vs 200) so
    // the two files differ in byte length too — Qt's own load() cache keys on path + mtime +
    // size, and two writes landing within the same filesystem mtime tick would otherwise defeat
    // the test by coincidence if both files happened to be the same size.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = tempDir.path() + "/reloadable.svg";

    WorkSpace workspace;
    workspace.scene()->setContextDir(tempDir.path());

    auto writeSvg = [](const QString &filePath, int side) {
        QFile file(filePath);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        const QByteArray svg = QString("<svg xmlns=\"http://www.w3.org/2000/svg\" "
                                        "width=\"%1\" height=\"%1\"><rect width=\"%1\" height=\"%1\" "
                                        "fill=\"red\"/></svg>").arg(side).toUtf8();
        QCOMPARE(file.write(svg), svg.size());
    };

    writeSvg(path, 9);
    auto *firstSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    workspace.scene()->addItem(firstSwitch);
    firstSwitch->setAppearance(false, path);
    const QSizeF firstSize = firstSwitch->boundingRect().size();

    writeSvg(path, 200);
    auto *secondSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    workspace.scene()->addItem(secondSwitch);
    secondSwitch->setAppearance(false, path);
    const QSizeF secondSize = secondSwitch->boundingRect().size();

    QVERIFY2(firstSize != secondSize,
             "A fresh element loading the same path after the file changed on disk must not "
             "reuse another element's stale cached pixmap");
}

void TestInputElements::testRotarySelectionIsResetAndRestoredWithSimState()
{
    auto *rotary = ElementFactory::buildElement(ElementType::InputRotary);
    QVERIFY(rotary != nullptr);
    rotary->setOutputSize(4);
    rotary->initSimulationVectors(rotary->inputSize(), rotary->outputSize());

    rotary->setOn(true, 2);                 // as a user click would
    // A live tick is what puts the selection into the SIMULATION outputs; setOn() only writes
    // the selection and the port statuses. sweep() snapshots after the simulation has been
    // running, so this mirrors the real ordering rather than saving an empty state.
    rotary->updateSourceOutputs();
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

// ============================================================================
// Input operations as undoable commands
// ============================================================================

namespace TestInputElementsHelpers {

void sendMouse(Scene &scene, const QEvent::Type type, const Element &element)
{
    QGraphicsSceneMouseEvent event(type);
    event.setScenePos(element.sceneBoundingRect().center());
    event.setButton(Qt::LeftButton);
    event.setButtons((type == QEvent::GraphicsSceneMouseRelease) ? Qt::NoButton : Qt::LeftButton);
    QCoreApplication::sendEvent(&scene, &event);
}

void sendMouseAt(Scene &scene, const QEvent::Type type, const QPointF &pos, const Qt::MouseButton button,
                 const Qt::MouseButtons buttons)
{
    QGraphicsSceneMouseEvent event(type);
    event.setScenePos(pos);
    event.setButton(button);
    event.setButtons(buttons);
    QCoreApplication::sendEvent(&scene, &event);
}

void click(Scene &scene, const Element &element)
{
    sendMouse(scene, QEvent::GraphicsSceneMousePress, element);
    sendMouse(scene, QEvent::GraphicsSceneMouseRelease, element);
}

void sendKey(Scene &scene, const QEvent::Type type, const Qt::Key key)
{
    QKeyEvent event(type, key, Qt::NoModifier);
    QCoreApplication::sendEvent(&scene, &event);
}

} // namespace TestInputElementsHelpers

void TestInputElements::testSwitchClickIsOneUndoableCommand()
{
    Scene scene;
    auto *inputSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    scene.addItem(inputSwitch);

    TestInputElementsHelpers::click(scene, *inputSwitch);

    QVERIFY(inputSwitch->isOn());
    QCOMPARE(scene.undoStack()->count(), 1);
    QVERIFY(!scene.undoStack()->isClean());

    scene.undoStack()->undo();
    QVERIFY(!inputSwitch->isOn());
    scene.undoStack()->redo();
    QVERIFY(inputSwitch->isOn());
}

void TestInputElements::testADoubleClickOnAnotherSwitchIsOneUndoableCommand()
{
    // A double-click on a switch other than the one that last held the mouse is a press of it: one undoable command,
    // as a click is. One on the switch just clicked edits its label, which records nothing
    Scene scene;
    auto *first = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *second = ElementFactory::buildElement(ElementType::InputSwitch);
    second->setPos(0, 128);
    scene.addItem(first);
    scene.addItem(second);
    const auto doubleClick = [&scene](const auto &element) {
        for (const QEvent::Type type : {QEvent::GraphicsSceneMouseDoubleClick, QEvent::GraphicsSceneMouseRelease}) {
            QGraphicsSceneMouseEvent event(type);
            event.setScenePos(element.sceneBoundingRect().center());
            event.setButton(Qt::LeftButton);
            event.setButtons((type == QEvent::GraphicsSceneMouseRelease) ? Qt::NoButton : Qt::LeftButton);
            QCoreApplication::sendEvent(&scene, &event);
        }
    };

    TestInputElementsHelpers::click(scene, *first);
    QCOMPARE(scene.undoStack()->count(), 1);
    doubleClick(*second);
    QVERIFY(second->isOn());
    QCOMPARE(scene.undoStack()->count(), 2);

    TestInputElementsHelpers::click(scene, *second);
    doubleClick(*second);
    QVERIFY(!second->isOn());
    QCOMPARE(scene.undoStack()->count(), 3);

    scene.undoStack()->undo();
    QVERIFY(second->isOn());
    scene.undoStack()->undo();
    QVERIFY(!second->isOn());
    QVERIFY(first->isOn());
}

void TestInputElements::testRotaryClickIsOneUndoableCommand()
{
    Scene scene;
    auto *rotary = ElementFactory::buildElement(ElementType::InputRotary);
    scene.addItem(rotary);

    TestInputElementsHelpers::click(scene, *rotary);

    QCOMPARE(rotary->param<int>(ParamId::Position), 1);
    QCOMPARE(scene.undoStack()->count(), 1);
    QVERIFY(!scene.undoStack()->isClean());

    scene.undoStack()->undo();
    QCOMPARE(rotary->param<int>(ParamId::Position), 0);
    scene.undoStack()->redo();
    QCOMPARE(rotary->param<int>(ParamId::Position), 1);
}

void TestInputElements::testRotaryClickThroughAZoomedViewIsOneUndoableCommand()
{
    // Through a view Qt delivers a press to the items under the pointer's pixel, mapped through the view's transform
    Scene scene;
    auto *rotary = ElementFactory::buildElement(ElementType::InputRotary);
    rotary->setPos(200, 120);
    scene.addItem(rotary);
    GraphicsView view;
    view.setScene(&scene);
    view.resize(400, 300);
    view.zoomIn();
    view.zoomIn();
    view.centerOn(rotary);

    const QPoint pixel = view.mapFromScene(rotary->sceneBoundingRect().center());
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, pixel);

    QCOMPARE(rotary->param<int>(ParamId::Position), 1);
    QCOMPARE(scene.undoStack()->count(), 1);
    scene.undoStack()->undo();
    QCOMPARE(rotary->param<int>(ParamId::Position), 0);
}

void TestInputElements::testTriggerKeyIsOneUndoableCommandKeepingTheSelection()
{
    Scene scene;
    auto *first = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *second = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *rotary = ElementFactory::buildElement(ElementType::InputRotary);
    first->setTrigger(QKeySequence(Qt::Key_A));
    second->setTrigger(QKeySequence(Qt::Key_A));
    rotary->setTrigger(QKeySequence(Qt::Key_B));
    second->setPos(0, 128);
    rotary->setPos(0, 256);
    scene.addItem(first);
    scene.addItem(second);
    scene.addItem(rotary);
    first->setSelected(true);

    // Both switches with the key toggle as one command, and the selection stays as it was
    TestInputElementsHelpers::sendKey(scene, QEvent::KeyPress, Qt::Key_A);
    TestInputElementsHelpers::sendKey(scene, QEvent::KeyRelease, Qt::Key_A);
    QVERIFY(first->isOn());
    QVERIFY(second->isOn());
    QCOMPARE(scene.undoStack()->count(), 1);
    QVERIFY(first->isSelected());
    QVERIFY(!second->isSelected());

    TestInputElementsHelpers::sendKey(scene, QEvent::KeyPress, Qt::Key_B);
    QCOMPARE(rotary->param<int>(ParamId::Position), 1);
    QCOMPARE(scene.undoStack()->count(), 2);
    QVERIFY(!rotary->isSelected());

    scene.undoStack()->undo();
    QCOMPARE(rotary->param<int>(ParamId::Position), 0);
    scene.undoStack()->undo();
    QVERIFY(!first->isOn());
    QVERIFY(!second->isOn());
}

void TestInputElements::testButtonPressIsNotRecorded()
{
    // A held button is not saved, so pressing it changes nothing a file or an undo holds
    Scene scene;
    auto *button = ElementFactory::buildElement(ElementType::InputButton);
    button->setTrigger(QKeySequence(Qt::Key_A));
    scene.addItem(button);

    TestInputElementsHelpers::sendMouse(scene, QEvent::GraphicsSceneMousePress, *button);
    QVERIFY(button->isOn());
    TestInputElementsHelpers::sendMouse(scene, QEvent::GraphicsSceneMouseRelease, *button);
    QVERIFY(!button->isOn());

    TestInputElementsHelpers::sendKey(scene, QEvent::KeyPress, Qt::Key_A);
    QVERIFY(button->isOn());
    TestInputElementsHelpers::sendKey(scene, QEvent::KeyRelease, Qt::Key_A);
    QVERIFY(!button->isOn());

    QCOMPARE(scene.undoStack()->count(), 0);
    QVERIFY(scene.undoStack()->isClean());
}

void TestInputElements::testMiddlePressLeavesTheSwitchNotHoldingTheMouse()
{
    // Only a left press makes an element hold the mouse (Qt keeps another button's press on an item only when the
    // item is movable, and the scene drags elements itself), so a left press made away from a switch after a middle
    // press on it toggles nothing and records nothing
    Scene scene;
    auto *inputSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    scene.addItem(inputSwitch);

    QGraphicsSceneMouseEvent middle(QEvent::GraphicsSceneMousePress);
    middle.setScenePos(inputSwitch->sceneBoundingRect().center());
    middle.setButton(Qt::MiddleButton);
    middle.setButtons(Qt::MiddleButton);
    QCoreApplication::sendEvent(&scene, &middle);
    QCOMPARE(scene.mouseGrabberItem(), nullptr);

    QGraphicsSceneMouseEvent left(QEvent::GraphicsSceneMousePress);
    left.setScenePos(inputSwitch->sceneBoundingRect().bottomRight() + QPointF(200, 200));
    left.setButton(Qt::LeftButton);
    left.setButtons(Qt::LeftButton | Qt::MiddleButton);
    QCoreApplication::sendEvent(&scene, &left);

    QVERIFY(!inputSwitch->isOn());
    QCOMPARE(scene.undoStack()->count(), 0);
}

void TestInputElements::testMiddlePressOnAHeldSwitchTogglesNothing()
{
    // Qt gives a middle press made while the left button holds a switch to the switch, and only a left press operates it
    Scene scene;
    auto *inputSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    scene.addItem(inputSwitch);
    const QPointF at = inputSwitch->sceneBoundingRect().center();

    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMousePress, at, Qt::LeftButton, Qt::LeftButton);
    QVERIFY(inputSwitch->isOn());
    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMousePress, at, Qt::MiddleButton, Qt::LeftButton | Qt::MiddleButton);
    QCOMPARE(scene.mouseGrabberItem(), inputSwitch);
    QVERIFY(inputSwitch->isOn());
    QCOMPARE(scene.undoStack()->count(), 1);
}

void TestInputElements::testMiddleReleaseLeavesAHeldButtonPressed()
{
    // Only the left button's release lets a held button go
    Scene scene;
    auto *button = ElementFactory::buildElement(ElementType::InputButton);
    scene.addItem(button);
    const QPointF at = button->sceneBoundingRect().center();

    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMousePress, at, Qt::LeftButton, Qt::LeftButton);
    QVERIFY(button->isOn());
    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMouseRelease, at, Qt::MiddleButton, Qt::LeftButton);
    QVERIFY(button->isOn());
    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMouseRelease, at, Qt::LeftButton, Qt::NoButton);
    QVERIFY(!button->isOn());
}

void TestInputElements::testAButtonLockedWhileHeldStaysPressed()
{
    // A release is no operation of a locked input: a button locked while held stays pressed
    Scene scene;
    auto *button = ElementFactory::buildElement(ElementType::InputButton);
    scene.addItem(button);

    TestInputElementsHelpers::sendMouse(scene, QEvent::GraphicsSceneMousePress, *button);
    QVERIFY(button->isOn());
    button->setLocked(true);
    TestInputElementsHelpers::sendMouse(scene, QEvent::GraphicsSceneMouseRelease, *button);
    QVERIFY(button->isOn());
}

void TestInputElements::testLockedInputsIgnoreTheirTriggerKeys()
{
    Scene scene;
    auto *inputSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *button = ElementFactory::buildElement(ElementType::InputButton);
    inputSwitch->setTrigger(QKeySequence(Qt::Key_A));
    button->setTrigger(QKeySequence(Qt::Key_B));
    button->setPos(0, 128);
    scene.addItem(inputSwitch);
    scene.addItem(button);

    // A locked switch's key toggles nothing
    inputSwitch->setLocked(true);
    TestInputElementsHelpers::sendKey(scene, QEvent::KeyPress, Qt::Key_A);
    QVERIFY(!inputSwitch->isOn());
    QCOMPARE(scene.undoStack()->count(), 0);

    // A button locked while its key is held stays pressed when the key is let go
    TestInputElementsHelpers::sendKey(scene, QEvent::KeyPress, Qt::Key_B);
    QVERIFY(button->isOn());
    button->setLocked(true);
    TestInputElementsHelpers::sendKey(scene, QEvent::KeyRelease, Qt::Key_B);
    QVERIFY(button->isOn());
}

void TestInputElements::testTheReleaseGoesToTheElementHoldingTheMouse()
{
    // A button held by the mouse lets go when the left button is released anywhere
    Scene scene;
    auto *button = ElementFactory::buildElement(ElementType::InputButton);
    scene.addItem(button);

    TestInputElementsHelpers::sendMouse(scene, QEvent::GraphicsSceneMousePress, *button);
    QVERIFY(button->isOn());
    const QPointF away = button->sceneBoundingRect().bottomRight() + QPointF(300, 300);
    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMouseRelease, away, Qt::LeftButton, Qt::NoButton);
    QVERIFY(!button->isOn());
}

void TestInputElements::testADoubleClickOnTheElementJustPressedEditsItsLabel()
{
    // Qt sends press, release, double-click, release: the switch toggles once, and the double-click on the element
    // that held the mouse asks to edit its label
    Scene scene;
    auto *inputSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    scene.addItem(inputSwitch);
    const QPointF at = inputSwitch->sceneBoundingRect().center();

    TestInputElementsHelpers::click(scene, *inputSwitch);
    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMouseDoubleClick, at, Qt::LeftButton, Qt::LeftButton);
    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMouseRelease, at, Qt::LeftButton, Qt::NoButton);
    QVERIFY(inputSwitch->isOn());
    QVERIFY(TestUtils::inlineLabelEditor(&scene));
}

void TestInputElements::testADoubleClickAfterAPressNothingTookEditsTheLabel()
{
    // A press no item takes leaves no element having last held the mouse, so Qt gives a double-click that follows to
    // the element under it as a double-click: it edits the label and operates nothing
    Scene scene;
    auto *inputSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    scene.addItem(inputSwitch);
    const QPointF at = inputSwitch->sceneBoundingRect().center();
    const QPointF away = inputSwitch->sceneBoundingRect().bottomRight() + QPointF(200, 200);

    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMousePress, away, Qt::LeftButton, Qt::LeftButton);
    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMouseRelease, away, Qt::LeftButton, Qt::NoButton);
    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMouseDoubleClick, at, Qt::LeftButton, Qt::LeftButton);
    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMouseRelease, at, Qt::LeftButton, Qt::NoButton);

    QVERIFY(!inputSwitch->isOn());
    QCOMPARE(scene.undoStack()->count(), 0);
    QVERIFY(TestUtils::inlineLabelEditor(&scene));

}

void TestInputElements::testADoubleClickAfterTheLastHolderLeftEditsTheLabel()
{
    // Qt forgets the element that last held the mouse when it leaves the scene, as a deletion kept for undo does, so a
    // double-click on another element is a double-click of it: it edits the label and operates nothing
    Scene scene;
    auto *first = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *second = ElementFactory::buildElement(ElementType::InputSwitch);
    second->setPos(0, 128);
    scene.addItem(first);
    scene.addItem(second);
    const QPointF at = second->sceneBoundingRect().center();

    TestInputElementsHelpers::click(scene, *first);
    QVERIFY(first->isOn());
    // Through QGraphicsScene's own removeItem(), as Qt's code removes an item, past Scene::removeItem()
    static_cast<QGraphicsScene &>(scene).removeItem(first);
    const std::unique_ptr<Element> removed(first);
    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMouseDoubleClick, at, Qt::LeftButton, Qt::LeftButton);
    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMouseRelease, at, Qt::LeftButton, Qt::NoButton);

    QVERIFY(!second->isOn());
    QCOMPARE(scene.undoStack()->count(), 1);
    QVERIFY(TestUtils::inlineLabelEditor(&scene));
}

void TestInputElements::testADoubleClickAfterTheLastHolderCameBackEditsTheLabel()
{
    // An element that left the scene and came back, as a deletion undone does, is not the one that last held the
    // mouse, which Qt forgot when it left: a double-click on another element is a double-click of it
    Scene scene;
    auto *first = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *second = ElementFactory::buildElement(ElementType::InputSwitch);
    second->setPos(0, 128);
    scene.addItem(first);
    scene.addItem(second);
    const QPointF at = second->sceneBoundingRect().center();

    TestInputElementsHelpers::click(scene, *first);
    QVERIFY(first->isOn());
    scene.removeItem(first);
    scene.addItem(first);
    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMouseDoubleClick, at, Qt::LeftButton, Qt::LeftButton);
    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMouseRelease, at, Qt::LeftButton, Qt::NoButton);

    QVERIFY(!second->isOn());
    QCOMPARE(scene.undoStack()->count(), 1);
    QVERIFY(TestUtils::inlineLabelEditor(&scene));
}

void TestInputElements::testADoubleClickOnAnotherElementPressesIt()
{
    // A double-click on an element other than the one that last held the mouse is a press of it: it operates, as one
    // undoable step, and edits no label
    Scene scene;
    auto *first = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *second = ElementFactory::buildElement(ElementType::InputSwitch);
    second->setPos(0, 128);
    scene.addItem(first);
    scene.addItem(second);
    const QPointF at = second->sceneBoundingRect().center();

    TestInputElementsHelpers::click(scene, *first);
    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMouseDoubleClick, at, Qt::LeftButton, Qt::LeftButton);
    TestInputElementsHelpers::sendMouseAt(scene, QEvent::GraphicsSceneMouseRelease, at, Qt::LeftButton, Qt::NoButton);
    QVERIFY(first->isOn());
    QVERIFY(second->isOn());
    QVERIFY(!TestUtils::inlineLabelEditor(&scene));
    QCOMPARE(scene.undoStack()->count(), 2);
    scene.undoStack()->undo();
    QVERIFY(!second->isOn());
}

void TestInputElements::testEachLevelInputNamesItsStatesInItsContext()
{
    // A translator that names each string's context shows that every state is looked up in its type's
    class ContextTagger : public QTranslator
    {
    public:
        QString translate(const char *context, const char *sourceText, const char *disambiguation, int n) const override
        {
            Q_UNUSED(disambiguation)
            Q_UNUSED(n)
            return QStringLiteral("[%1 %2]").arg(QString::fromUtf8(context), QString::fromUtf8(sourceText));
        }

        bool isEmpty() const override { return false; }
    };

    struct Named {
        ElementType type;
        QString context;
        QString low;
        QString high;
    };
    const QList<Named> inputs{
        {ElementType::InputSwitch, "InputSwitch", "Off", "On"},
        {ElementType::InputButton, "InputButton", "Released", "Pressed"},
        {ElementType::Clock, "Clock", "Low", "High"},
    };

    ContextTagger tagger;
    QCoreApplication::installTranslator(&tagger);
    const auto removeTagger = qScopeGuard([&tagger] { QCoreApplication::removeTranslator(&tagger); });
    for (const Named &input : inputs) {
        const Element element(input.type);
        const QList<std::pair<int, QString>> expected{
            {0, QStringLiteral("[%1 %2]").arg(input.context, input.low)},
            {1, QStringLiteral("[%1 %2]").arg(input.context, input.high)},
        };
        QCOMPARE(element.appearanceStates(), expected);
    }
}
