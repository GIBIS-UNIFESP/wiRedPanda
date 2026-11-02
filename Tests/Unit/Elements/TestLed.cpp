// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestLed.h"

#include <memory>

#include <QDataStream>
#include <QFile>
#include <QImage>
#include <QSize>
#include <QTemporaryDir>
#include <QTest>

#include "App/Core/Common.h"
#include "App/Element/Element.h"
#include "App/IO/SerializationContext.h"
#include "App/Presentation/ElementPictures.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

// ============================================================================
// Constructor and Configuration Tests
// ============================================================================

void TestLED::testConstructorInitialization()
{
    Element led{ElementType::Led};

    // Verify element type
    QCOMPARE(led.elementType(), ElementType::Led);

    // Verify element group is Output
    QCOMPARE(led.elementGroup(), ElementGroup::Output);

    // Verify port configuration (1-4 inputs, 0 outputs)
    QVERIFY(led.inputSize() >= 1);
    QVERIFY(led.inputSize() <= 4);
    QCOMPARE(led.outputSize(), 0);

    // Verify LED has colors property for single input
    if (led.inputSize() == 1) {
        QVERIFY(led.offersParam(ParamId::Colour));
    }

    // Verify has label
    QVERIFY(led.hasLabel());

    // Verify cannot rotate
    QVERIFY(!led.rotatesGraphic());

    // Verify can change appearance
    QVERIFY(led.canChangeAppearance());
}

void TestLED::testPortConfiguration()
{
    Element led{ElementType::Led};

    // Verify port counts within valid range
    int inputSize = led.inputSize();
    QVERIFY(inputSize >= 1);
    QVERIFY(inputSize <= 4);

    // Verify min/max sizes match
    QCOMPARE(led.minInputSize(), 1);
    QCOMPARE(led.maxInputSize(), 4);

    // Verify no output ports
    QCOMPARE(led.minOutputSize(), 0);
    QCOMPARE(led.maxOutputSize(), 0);
}

void TestLED::testInputPortNames()
{
    Element led{ElementType::Led};

    // Verify input ports are named with their index (0, 1, 2, 3)
    for (int i = 0; i < led.inputSize(); ++i) {
        auto *port = led.inputPort(i);
        QVERIFY(port != nullptr);

        // Port name should be the index as string
        QCOMPARE(port->name(), QString::number(i + 1));

        // Ports should not be required (optional)
        QVERIFY(!port->isRequired());
    }
}

// ============================================================================
// Color Management Tests
// ============================================================================

void TestLED::testDefaultColor()
{
    Element led{ElementType::Led};

    // Default color should be White
    QCOMPARE(led.param<QString>(ParamId::Colour), QString("White"));
}

void TestLED::testSetColorWhite()
{
    Element led{ElementType::Led};

    led.setParam(ParamId::Colour, QStringLiteral("White"));
    QCOMPARE(led.param<QString>(ParamId::Colour), QString("White"));
}

void TestLED::testSetColorRed()
{
    Element led{ElementType::Led};

    led.setParam(ParamId::Colour, QStringLiteral("Red"));
    QCOMPARE(led.param<QString>(ParamId::Colour), QString("Red"));
}

void TestLED::testSetColorGreen()
{
    Element led{ElementType::Led};

    led.setParam(ParamId::Colour, QStringLiteral("Green"));
    QCOMPARE(led.param<QString>(ParamId::Colour), QString("Green"));
}

void TestLED::testSetColorBlue()
{
    Element led{ElementType::Led};

    led.setParam(ParamId::Colour, QStringLiteral("Blue"));
    QCOMPARE(led.param<QString>(ParamId::Colour), QString("Blue"));
}

void TestLED::testSetColorPurple()
{
    Element led{ElementType::Led};

    led.setParam(ParamId::Colour, QStringLiteral("Purple"));
    QCOMPARE(led.param<QString>(ParamId::Colour), QString("Purple"));
}

void TestLED::testGetColor()
{
    Element led{ElementType::Led};

    // Test color persistence across changes
    led.setParam(ParamId::Colour, QStringLiteral("Red"));
    QString color1 = led.param<QString>(ParamId::Colour);
    QCOMPARE(color1, QString("Red"));

    led.setParam(ParamId::Colour, QStringLiteral("Green"));
    QString color2 = led.param<QString>(ParamId::Colour);
    QCOMPARE(color2, QString("Green"));

    led.setParam(ParamId::Colour, QStringLiteral("Blue"));
    QString color3 = led.param<QString>(ParamId::Colour);
    QCOMPARE(color3, QString("Blue"));
}

// ============================================================================
// Shown Slot Tests
// ============================================================================

void TestLED::testShownSlotSingleInput()
{
    auto led = std::make_unique<Element>(ElementType::Led);
    QCOMPARE(led->inputSize(), 1);
    QCOMPARE(led->outputSize(), 0);

    // A single input shows its colour's off slot, or the next one when on: White's at 0, Red's at 2, Green's at 4,
    // Blue's at 6, Purple's at 8
    const QList<std::pair<QString, int>> colours{{"White", 0}, {"Red", 2}, {"Green", 4}, {"Blue", 6}, {"Purple", 8}};
    for (const auto &[colour, offset] : colours) {
        led->setParam(ParamId::Colour, colour);
        for (const Status level : {Status::Inactive, Status::Active}) {
            led->inputPort(0)->setStatus(level);
            led->refresh();
            QCOMPARE(led->param<int>(ParamId::Lit), offset + ((level == Status::Active) ? 1 : 0));
        }
    }
}

void TestLED::testShownSlotTwoInputs()
{
    // For 2-input LED, color mapping is different from 1-input
    // Special case: 0b11 (both inputs high) → index 25 (white)
    // Otherwise: index = 18 + binary_value (18-21 for 0b00-0b11, but 0b11 maps to 25)
    auto led = std::make_unique<Element>(ElementType::Led);
    led->setInputSize(2);
    QCOMPARE(led->inputSize(), 2);

    auto *port0 = led->inputPort(0);
    auto *port1 = led->inputPort(1);
    QVERIFY2(port0 != nullptr && port1 != nullptr, "2-input LED should have 2 input ports");

    // The slot a refresh shows (ParamId::Lit, which the presenter draws), whatever the colour.
    port0->setStatus(Status::Inactive);
    port1->setStatus(Status::Inactive);
    led->refresh();
    QCOMPARE(led->param<int>(ParamId::Lit), 18);

    port0->setStatus(Status::Active);
    port1->setStatus(Status::Inactive);
    led->refresh();
    QCOMPARE(led->param<int>(ParamId::Lit), 19);

    // The special case: both inputs high (0b11) maps to 25 (white), not 21.
    port0->setStatus(Status::Active);
    port1->setStatus(Status::Active);
    led->refresh();
    QCOMPARE(led->param<int>(ParamId::Lit), 25);
}

void TestLED::testShownSlotThreeInputs()
{
    // For 3-input LED, color mapping uses slots 18-25: slot = 18 + binary_value.
    auto led = std::make_unique<Element>(ElementType::Led);
    led->setInputSize(3);
    QCOMPARE(led->inputSize(), 3);

    for (int value = 0; value < 8; ++value) {
        for (int i = 0; i < 3; ++i) {
            led->inputPort(i)->setStatus((value & (1 << i)) ? Status::Active : Status::Inactive);
        }
        led->refresh();
        QCOMPARE(led->param<int>(ParamId::Lit), 18 + value);
    }
}

void TestLED::testShownSlotFourInputs()
{
    // For 4-input LED, full 16-color palette mapping: slot = 10 + binary_value.
    auto led = std::make_unique<Element>(ElementType::Led);
    led->setInputSize(4);
    QCOMPARE(led->inputSize(), 4);

    for (int value = 0; value < 16; ++value) {
        for (int i = 0; i < 4; ++i) {
            led->inputPort(i)->setStatus((value & (1 << i)) ? Status::Active : Status::Inactive);
        }
        led->refresh();
        QCOMPARE(led->param<int>(ParamId::Lit), 10 + value);
    }
}

void TestLED::testShownSlotWithTwoDriversOnAnInput()
{
    // Two wires into one input are no level the LED can show: it shows its first slot, whatever its port reads
    Element first{ElementType::InputSwitch};
    Element second{ElementType::InputSwitch};
    Element led{ElementType::Led};
    Connection one;
    one.setStartPort(first.outputPort());
    one.setEndPort(led.inputPort());
    led.inputPort()->setStatus(Status::Active);
    led.refresh();
    QCOMPARE(led.param<int>(ParamId::Lit), 1);

    Connection other;
    other.setStartPort(second.outputPort());
    other.setEndPort(led.inputPort());
    led.inputPort()->setStatus(Status::Active);
    led.refresh();
    QCOMPARE(led.param<int>(ParamId::Lit), 0);
}

// ============================================================================
// Port Properties Tests
// ============================================================================

void TestLED::testUpdatePortsProperties()
{
    // Test with 1-input LED
    Element led1{ElementType::Led};
    QCOMPARE(led1.inputSize(), 1);
    led1.updatePortsProperties();
    QVERIFY(led1.inputSize() > 0);
    QVERIFY2(led1.offersParam(ParamId::Colour), "1-input LED should have colors");

    // Test with 2-input LED (multi-input - no colors)
    Element led2{ElementType::Led};
    led2.setInputSize(2);
    QCOMPARE(led2.inputSize(), 2);
    led2.updatePortsProperties();
    QVERIFY(led2.inputSize() > 0);
    QVERIFY2(!led2.offersParam(ParamId::Colour), "2-input LED should NOT have individual color controls");

    // Test with 3-input LED (multi-input - no colors)
    Element led3{ElementType::Led};
    led3.setInputSize(3);
    QCOMPARE(led3.inputSize(), 3);
    led3.updatePortsProperties();
    QVERIFY(led3.inputSize() > 0);
    QVERIFY2(!led3.offersParam(ParamId::Colour), "3-input LED should NOT have individual color controls");

    // Test with 4-input LED (multi-input - no colors)
    Element led4{ElementType::Led};
    led4.setInputSize(4);
    QCOMPARE(led4.inputSize(), 4);
    led4.updatePortsProperties();
    QVERIFY(led4.inputSize() > 0);
    QVERIFY2(!led4.offersParam(ParamId::Colour), "4-input LED should NOT have individual color controls");
}

void TestLED::testPortNamesAfterUpdate()
{
    // Test 1-input LED port names
    Element led1{ElementType::Led};
    led1.updatePortsProperties();
    for (int i = 0; i < led1.inputSize(); ++i) {
        auto *port = led1.inputPort(i);
        QCOMPARE(port->name(), QString::number(i + 1));
    }

    // Test 2-input LED port names
    Element led2{ElementType::Led};
    led2.setInputSize(2);
    led2.updatePortsProperties();
    QCOMPARE(led2.inputSize(), 2);
    for (int i = 0; i < led2.inputSize(); ++i) {
        auto *port = led2.inputPort(i);
        QCOMPARE(port->name(), QString::number(i + 1));
    }

    // Test 3-input LED port names
    Element led3{ElementType::Led};
    led3.setInputSize(3);
    led3.updatePortsProperties();
    QCOMPARE(led3.inputSize(), 3);
    for (int i = 0; i < led3.inputSize(); ++i) {
        auto *port = led3.inputPort(i);
        QCOMPARE(port->name(), QString::number(i + 1));
    }

    // Test 4-input LED port names
    Element led4{ElementType::Led};
    led4.setInputSize(4);
    led4.updatePortsProperties();
    QCOMPARE(led4.inputSize(), 4);
    for (int i = 0; i < led4.inputSize(); ++i) {
        auto *port = led4.inputPort(i);
        QCOMPARE(port->name(), QString::number(i + 1));
    }
}

void TestLED::testHasColorsProperty()
{
    // Colors property depends on input size
    // Single input: has colors (5 colors available)
    // Multiple inputs: no colors (uses multi-input palette instead)

    // Test 1-input LED - should have colors
    Element led1{ElementType::Led};
    QCOMPARE(led1.inputSize(), 1);
    QVERIFY2(led1.offersParam(ParamId::Colour), "1-input LED should have hasColors() = true");

    // Test 2-input LED - should NOT have colors
    Element led2{ElementType::Led};
    led2.setInputSize(2);
    QCOMPARE(led2.inputSize(), 2);
    QVERIFY2(!led2.offersParam(ParamId::Colour), "2-input LED should have hasColors() = false (uses multi-input palette)");

    // Test 3-input LED - should NOT have colors
    Element led3{ElementType::Led};
    led3.setInputSize(3);
    QCOMPARE(led3.inputSize(), 3);
    QVERIFY2(!led3.offersParam(ParamId::Colour), "3-input LED should have hasColors() = false (uses multi-input palette)");

    // Test 4-input LED - should NOT have colors
    Element led4{ElementType::Led};
    led4.setInputSize(4);
    QCOMPARE(led4.inputSize(), 4);
    QVERIFY2(!led4.offersParam(ParamId::Colour), "4-input LED should have hasColors() = false (uses multi-input palette)");
}

// ============================================================================
// Serialization Tests
// ============================================================================

void TestLED::testSaveColor()
{
    Element led{ElementType::Led};

    led.setParam(ParamId::Colour, QStringLiteral("Red"));

    // Save to stream
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    led.save(stream, {.purpose = SerializationPurpose::PortableFile});

    // Verify data was written
    QVERIFY(data.size() > 0);
}

void TestLED::testLoadColorOldVersion()
{
    // v2.3.3 (old format, pre-v4.1: color stored as a bare QString -- see the
    // hasClock() branch of the LED descriptor's readBeforeMap) has four LEDs, three of them with
    // distinct non-default colors, unlike display-3bits.panda (which has no Led at all,
    // only a Display7 -- confirmed by dumping every loaded element's type).
    QString filePath = TestUtils::backwardCompatibilityDir() + "v2.3.3/box.panda";

    QVERIFY2(QFile::exists(filePath), qPrintable(QString("Backward compatibility file should exist: %1").arg(filePath)));

    WorkSpace workspace;
    try {
        workspace.load(filePath);
        Scene *scene = workspace.scene();
        QVERIFY(scene != nullptr);

        QSet<QString> loadedColors;
        for (auto *item : scene->items()) {
            if (auto *led = qgraphicsitem_cast<Element *>(item); led && led->elementType() == ElementType::Led) {
                loadedColors.insert(led->param<QString>(ParamId::Colour));
            }
        }
        QCOMPARE(loadedColors, QSet<QString>({"Red", "Green", "Blue", "White"}));
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load backward compatibility file: %1").arg(e.what())));
    }
}

void TestLED::testLoadColorNewVersion()
{
    // Create LED and save it
    auto led1 = std::make_unique<Element>(ElementType::Led);
    led1->setParam(ParamId::Colour, QStringLiteral("Blue"));

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    led1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Create new LED and load with new version (>= 4.1)
    auto led2 = std::make_unique<Element>(ElementType::Led);

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, QVersionNumber(4, 1), SerializationPurpose::PortableFile, {}};

    led2->load(loadStream, context);

    // Color should be loaded correctly
    QCOMPARE(led2->param<QString>(ParamId::Colour), QString("Blue"));
}

void TestLED::testLoadColorDefault()
{
    // Test that very old version (< 1.1) doesn't attempt to load color

    // Create LED and set color
    Element led{ElementType::Led};
    QCOMPARE(led.param<QString>(ParamId::Colour), QString("White")); // Default color

    // Change color
    led.setParam(ParamId::Colour, QStringLiteral("Red"));
    QCOMPARE(led.param<QString>(ParamId::Colour), QString("Red"));

    // For very old versions (< 1.1), the load() method returns early without loading color
    // Create minimal data (empty or just headers)
    QByteArray data;
    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    // Load with version < 1.1 - should return early, leaving color unchanged
    led.setParam(ParamId::Colour, QStringLiteral("Blue"));
    SerializationContext contextOld = {portMap, QVersionNumber(1, 0), SerializationPurpose::PortableFile, {}};
    led.load(loadStream, contextOld);

    // Color should remain "Blue" because very old version returns early without loading
    QCOMPARE(led.param<QString>(ParamId::Colour), QString("Blue"));
}

void TestLED::testAppearanceStates()
{
    // 1-input: Off/On
    Element led1{ElementType::Led};
    auto states1 = led1.appearanceStates();
    QCOMPARE(states1.size(), 2);
    QCOMPARE(states1.at(0).second, QString("Off"));
    QCOMPARE(states1.at(1).second, QString("On"));

    // 2-input: explicit per-port labels, no (off)/(on) suffix
    Element led2{ElementType::Led};
    led2.setInputSize(2);
    auto states2 = led2.appearanceStates();
    QCOMPARE(states2.size(), 4);
    QCOMPARE(states2.at(0).second, QString("Port 1=0, Port 2=0"));
    QCOMPARE(states2.at(1).second, QString("Port 1=1, Port 2=0"));
    QCOMPARE(states2.at(2).second, QString("Port 1=0, Port 2=1"));
    QCOMPARE(states2.at(3).second, QString("Port 1=1, Port 2=1"));

    // 3-input: 8 states
    Element led3{ElementType::Led};
    led3.setInputSize(3);
    auto states3 = led3.appearanceStates();
    QCOMPARE(states3.size(), 8);
    QCOMPARE(states3.at(0).second, QString("Port 1=0, Port 2=0, Port 3=0"));
    QCOMPARE(states3.at(7).second, QString("Port 1=1, Port 2=1, Port 3=1"));

    // 4-input: 16 states
    Element led4{ElementType::Led};
    led4.setInputSize(4);
    auto states4 = led4.appearanceStates();
    QCOMPARE(states4.size(), 16);
    QCOMPARE(states4.at(0).second, QString("Port 1=0, Port 2=0, Port 3=0, Port 4=0"));
    QCOMPARE(states4.at(15).second, QString("Port 1=1, Port 2=1, Port 3=1, Port 4=1"));

    // Index mapping into the appearance list must stay unchanged (serialized per-slot).
    QCOMPARE(states2.at(0).first, 18);
    QCOMPARE(states2.at(3).first, 25); // the slot two inputs both on show
    QCOMPARE(states3.at(0).first, 18);
    QCOMPARE(states4.at(0).first, 10);
}

void TestLED::testAppearancePreviewPixmap()
{
    Element led{ElementType::Led};
    led.setInputSize(2);
    const auto states = led.appearanceStates();

    const auto preview = [&led](const int index) {
        return ElementPictures::preview(led.appearance().alternativeAppearances().value(index), Theme::Light, QSize(16, 16));
    };
    for (const auto &[index, label] : states) {
        QVERIFY(!preview(index).isNull());
    }

    QVERIFY(preview(-1).isNull());
    QVERIFY(preview(9999).isNull());
}

void TestLED::testSetAppearanceDefault()
{
    // Give the LED a custom appearance, then restore the default and confirm the custom
    // external file is dropped (the default appearance references no external file).
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString imagePath = tempDir.filePath("custom_led.png");
    QImage image(64, 64, QImage::Format_RGB32);
    image.fill(Qt::red);
    QVERIFY(image.save(imagePath));

    Element led{ElementType::Led};
    led.setAppearance(false, imagePath);
    QCOMPARE(led.externalFiles(), QStringList{imagePath});

    led.setAppearance(true, {}); // restore default
    QVERIFY(led.externalFiles().isEmpty());
}

void TestLED::testSetAppearanceWithAbsolutePathNoContextDir()
{
    // A freshly picked, existing, absolute path must apply directly even when the element
    // has no scene/contextDir at all (e.g. before the project has ever been saved).
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString imagePath = tempDir.path() + "/custom_led.svg";
    QVERIFY(QFile::copy(":/Components/Output/Led/WhiteLed.svg", imagePath));

    Element led{ElementType::Led};
    bool threw = false;
    try {
        led.setAppearance(false, imagePath);
    } catch (const Pandaception &) {
        threw = true;
    }

    QVERIFY2(!threw, "setAppearance should resolve an existing absolute path directly even with no contextDir");
}

void TestLED::testSetAppearanceWithNonExistentFileShowsTheDefault()
{
    Element led{ElementType::Led};
    led.setAppearance(false, "/tmp/nonexistent_custom_led_98765.png");

    const Element fresh{ElementType::Led};
    QCOMPARE(SnapshotAccess::pixmapIdentity(&led), SnapshotAccess::pixmapIdentity(&fresh));
    QCOMPARE(led.externalFiles(), QStringList{"/tmp/nonexistent_custom_led_98765.png"});
}

void TestLED::testSetAppearanceWithoutSavedProjectLoadsPixmap()
{
    // A bare, scene-less Led mirrors a brand-new, never-saved project: setAppearance()
    // is given a path directly (as the UI does from a file picker), with no load()
    // round-trip involved, so there's no contextDir to resolve against either way.
    Element led{ElementType::Led};
    const QRectF defaultBounds = led.boundingRect();

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString imagePath = tempDir.filePath("custom_led.png");

    // Deliberately a very different size than the default LED icon, so a
    // successful pixmap load is observable via boundingRect() changing.
    QImage image(400, 400, QImage::Format_RGB32);
    image.fill(Qt::red);
    QVERIFY(image.save(imagePath));

    led.setAppearance(false, imagePath);

    QCOMPARE(led.externalFiles(), QStringList{imagePath});
    QVERIFY(led.boundingRect() != defaultBounds);
}
