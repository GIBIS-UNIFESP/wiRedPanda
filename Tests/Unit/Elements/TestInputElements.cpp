// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestInputElements.h"

#include <memory>

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#include "App/Element/GraphicElements/InputButton.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

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
    InputSwitch inputSwitch;

    // Locking is handled by parent class, we just verify state can be changed
    inputSwitch.setOn(false);
    QVERIFY(!inputSwitch.isOn());

    inputSwitch.setOn(true);
    QVERIFY(inputSwitch.isOn());

    // Verify locking doesn't affect setOn(bool) directly
    inputSwitch.setOn(false);
    QVERIFY(!inputSwitch.isOn());
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
    InputButton inputButton;

    // Locking is handled by parent class, we just verify state can be changed
    inputButton.setOff();
    QVERIFY(!inputButton.isOn());

    inputButton.setOn();
    QVERIFY(inputButton.isOn());

    // Verify locking doesn't affect setOn/setOff directly
    inputButton.setOff();
    QVERIFY(!inputButton.isOn());

    inputButton.setOn();
    QVERIFY(inputButton.isOn());
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
    // Tests/Unit/Core/TestExternalFilePath.cpp for the resolution mechanics themselves.
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
    // See Tests/Unit/Core/TestExternalFilePath.cpp for the exhaustive separator matrix --
    // this just confirms the appearance-restore path actually exercises that fallback.
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
    auto *firstSwitch = new InputSwitch();
    workspace.scene()->addItem(firstSwitch);
    firstSwitch->setAppearance(false, path);
    const QSizeF firstSize = firstSwitch->boundingRect().size();

    writeSvg(path, 200);
    auto *secondSwitch = new InputSwitch();
    workspace.scene()->addItem(secondSwitch);
    secondSwitch->setAppearance(false, path);
    const QSizeF secondSize = secondSwitch->boundingRect().size();

    QVERIFY2(firstSize != secondSize,
             "A fresh element loading the same path after the file changed on disk must not "
             "reuse another element's stale cached pixmap");
}
