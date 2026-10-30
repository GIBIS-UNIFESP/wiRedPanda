// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestParameters.h"

#include <limits>
#include <memory>

#include <QAudioOutput>
#include <QAudioSink>
#include <QDataStream>
#include <QFile>
#include <QHash>
#include <QMetaEnum>
#include <QTemporaryDir>
#include <QTest>

#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementTypeDescriptor.h"
#include "App/Element/GraphicElements/AudioBox.h"
#include "App/Element/GraphicElements/Buzzer.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Element/GraphicElements/InputButton.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/ToneGenerator.h"
#include "App/Element/Parameters.h"
#include "App/IO/SerializationContext.h"
#include "App/Versions.h"
#include "App/Wiring/Port.h"

namespace ParametersTestDetail {

/// The float nearest to 0.1, as a double.
constexpr double kNearestFloatToATenth = 0.100000001490116119384765625;

/// Returns \a value as text: a string quoted, a bit array by its size and the bits set.
QString text(const ParamValue &value)
{
    if (const auto *flag = std::get_if<bool>(&value)) {
        return *flag ? QStringLiteral("true") : QStringLiteral("false");
    }
    if (const auto *number = std::get_if<int>(&value)) {
        return QString::number(*number);
    }
    if (const auto *real = std::get_if<double>(&value)) {
        return QString::number(*real, 'g', 17);
    }
    if (const auto *string = std::get_if<QString>(&value)) {
        return '"' + *string + '"';
    }
    const auto &bits = std::get<QBitArray>(value);
    return QStringLiteral("bits %1/%2").arg(bits.count(true)).arg(bits.size());
}

/// Returns the name of parameter \a id.
QString name(const ParamId id)
{
    switch (id) {
    case ParamId::On:        return QStringLiteral("On");
    case ParamId::Locked:    return QStringLiteral("Locked");
    case ParamId::Position:  return QStringLiteral("Position");
    case ParamId::Frequency: return QStringLiteral("Frequency");
    case ParamId::Delay:     return QStringLiteral("Delay");
    case ParamId::Volume:    return QStringLiteral("Volume");
    case ParamId::Colour:    return QStringLiteral("Colour");
    case ParamId::AudioFile: return QStringLiteral("AudioFile");
    case ParamId::Wireless:  return QStringLiteral("Wireless");
    case ParamId::Table:     return QStringLiteral("Table");
    }
    return {};
}

/// Returns \a element's parameters as "<name> <value> <key>" lines, in its schema's order; "-" for a transient one's
/// key. Each value is the element's, which a new element has from its spec's default.
QStringList written(const Element &element)
{
    QStringList lines;
    for (const ParamSpec &spec : ElementTypeDescriptor::of(element.elementType()).parameters) {
        lines << QStringLiteral("%1 %2 %3").arg(name(spec.id), text(element.parameters().value(spec.id)),
                                               spec.key ? QString::fromLatin1(spec.key) : QStringLiteral("-"));
    }
    return lines;
}

/// A node counting what it is told of its parameters.
class CountingNode : public Node
{
public:
    int changes = 0;
    ParamValue previous;

protected:
    void parameterChanged(const ParamId id, const ParamValue &before) override
    {
        Node::parameterChanged(id, before);
        ++changes;
        previous = before;
    }
};

} // namespace ParametersTestDetail

using namespace ParametersTestDetail;

// --- The schemas ---

void TestParameters::testEachTypeHasTheParametersOfItsSchema_data()
{
    QTest::addColumn<ElementType>("type");
    QTest::addColumn<QStringList>("expected");

    // The parameters of every type, as its save() and load() have them: the default, and the key a file keeps it
    // under. A type not listed has none.
    const QHash<ElementType, QStringList> parameters{
        {ElementType::InputButton, {"On false -", "Locked false locked"}},
        {ElementType::InputSwitch, {"On false isOn", "Locked false locked"}},
        {ElementType::InputRotary, {"Position 0 currentPort", "Locked false locked"}},
        {ElementType::Clock, {"On false -", "Locked false locked", "Frequency 1 frequency", "Delay 0 delay"}},
        {ElementType::Node, {"Wireless 0 wirelessMode"}},
        {ElementType::Led, {"Colour \"White\" color"}},
        {ElementType::Display7, {"Colour \"Red\" color"}},
        {ElementType::Display14, {"Colour \"Red\" color"}},
        {ElementType::Display16, {"Colour \"Red\" color"}},
        {ElementType::Buzzer, {"Frequency 1047 frequency", "Volume 0.34999999403953552 volume"}},
        {ElementType::AudioBox, {"AudioFile \":/Components/Output/Audio/wiredpanda.wav\" audiobox", "Volume 0.5 volume"}},
    };

    const QMetaEnum metaEnum = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(metaEnum.value(i));
        if ((type == ElementType::Unknown) || (type == ElementType::JKLatch)) {
            continue;
        }
        QTest::newRow(metaEnum.key(i)) << type << parameters.value(type);
    }
}

void TestParameters::testEachTypeHasTheParametersOfItsSchema()
{
    QFETCH(ElementType, type);
    QFETCH(QStringList, expected);

    const std::unique_ptr<Element> element(ElementFactory::buildElement(type));
    QCOMPARE(written(*element), expected);
}

// --- The store ---

void TestParameters::testTheStoreHoldsItsSchemasDefaults()
{
    const QList<ParamSpec> schema{
        {.id = ParamId::On, .defaultValue = false},
        {.id = ParamId::Frequency, .defaultValue = 1.0, .key = "frequency"},
        {.id = ParamId::Colour, .defaultValue = QStringLiteral("White"), .key = "color"},
    };
    Parameters parameters(schema);

    QVERIFY(parameters.has(ParamId::Frequency));
    QVERIFY(!parameters.has(ParamId::Volume));
    QVERIFY(parameters.spec(ParamId::Volume) == nullptr);
    QCOMPARE(parameters.spec(ParamId::Colour), &schema.at(2));
    QCOMPARE(parameters.get<bool>(ParamId::On), false);
    QCOMPARE(parameters.get<double>(ParamId::Frequency), 1.0);
    QCOMPARE(parameters.get<QString>(ParamId::Colour), QStringLiteral("White"));

    // A value is set as it is, and the one it had returned
    QCOMPARE(parameters.set(ParamId::Colour, QStringLiteral("Red")), ParamValue(QStringLiteral("White")));
    QCOMPARE(parameters.get<QString>(ParamId::Colour), QStringLiteral("Red"));
    QCOMPARE(parameters.get<double>(ParamId::Frequency), 1.0);
}

void TestParameters::testTheStoreRefusesWhatItsTypeDoesNotHave()
{
    // A parameter the type does not have, or one of another kind, is a broken invariant: an assertion, and in a
    // release build the kind's empty value and nothing set
#ifndef QT_NO_DEBUG
    QSKIP("The assertion stops a debug build; CI's release builds run this");
#endif
    const QList<ParamSpec> schema{{.id = ParamId::Frequency, .defaultValue = 1.0, .key = "frequency"}};
    Parameters parameters(schema);

    QCOMPARE(parameters.get<QString>(ParamId::Colour), QString());
    QCOMPARE(parameters.get<int>(ParamId::Frequency), 0);
    QCOMPARE(parameters.set(ParamId::Colour, QStringLiteral("Red")), ParamValue(QStringLiteral("Red")));
    QCOMPARE(parameters.set(ParamId::Frequency, 2), ParamValue(2));
    QCOMPARE(parameters.get<double>(ParamId::Frequency), 1.0);
}

// --- Setting a parameter ---

void TestParameters::testSetParamTakesAValueAsItsSpecDoes()
{
    Node node;

    QVERIFY(node.setParam(ParamId::Wireless, static_cast<int>(WirelessMode::Rx)));
    QCOMPARE(node.wirelessMode(), WirelessMode::Rx);
    QVERIFY(!node.inputPort()->isRequired());
    QVERIFY(!node.inputPort()->isVisible());

    // The value it has, a value of another kind and a parameter the type does not have change nothing
    QVERIFY(!node.setParam(ParamId::Wireless, static_cast<int>(WirelessMode::Rx)));
    QVERIFY(!node.setParam(ParamId::Wireless, QStringLiteral("Tx")));
    QVERIFY(!node.setParam(ParamId::Colour, QStringLiteral("Red")));
    QCOMPARE(node.wirelessMode(), WirelessMode::Rx);

    // The spec takes a mode out of range as None, as a file's loads
    QVERIFY(node.setParam(ParamId::Wireless, 7));
    QCOMPARE(node.wirelessMode(), WirelessMode::None);
    QVERIFY(node.inputPort()->isRequired());
    QVERIFY(node.inputPort()->isVisible());
    QVERIFY(!node.setParam(ParamId::Wireless, -1));

    // The accessor goes through the same spec
    node.setWirelessMode(WirelessMode::Tx);
    QCOMPARE(node.wirelessMode(), WirelessMode::Tx);
    QVERIFY(!node.outputPort()->isVisible());
    node.setWirelessMode(static_cast<WirelessMode>(3));
    QCOMPARE(node.wirelessMode(), WirelessMode::None);

    // A type without the parameter answers as if it were empty and ignores the write
    const std::unique_ptr<Element> gate(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!gate->hasWirelessMode());
    gate->setWirelessMode(WirelessMode::Tx);
    QCOMPARE(gate->wirelessMode(), WirelessMode::None);
}

void TestParameters::testParameterChangedFollowsEachChangeOnly()
{
    CountingNode node;

    QVERIFY(node.setParam(ParamId::Wireless, static_cast<int>(WirelessMode::Tx)));
    QCOMPARE(node.changes, 1);
    QCOMPARE(node.previous, ParamValue(static_cast<int>(WirelessMode::None)));

    QVERIFY(!node.setParam(ParamId::Wireless, static_cast<int>(WirelessMode::Tx)));
    QVERIFY(!node.setParam(ParamId::Wireless, 2.0));
    QCOMPARE(node.changes, 1);

    QVERIFY(node.setParam(ParamId::Wireless, 9));
    QCOMPARE(node.changes, 2);
    QCOMPARE(node.previous, ParamValue(static_cast<int>(WirelessMode::Tx)));
}

// --- The inputs ---

void TestParameters::testAnInputsLevelAndLockAreItsParameters()
{
    InputSwitch input;
    QVERIFY(input.setParam(ParamId::On, true));
    QVERIFY(input.isOn());
    QCOMPARE(input.outputPort()->status(), Status::Active);
    input.setOn(false);
    QCOMPARE(input.param<bool>(ParamId::On), false);
    QCOMPARE(input.outputPort()->status(), Status::Inactive);

    input.setLocked(true);
    QCOMPARE(input.param<bool>(ParamId::Locked), true);
    QVERIFY(input.setParam(ParamId::Locked, false));
    QVERIFY(!input.isLocked());
}

void TestParameters::testANewInputDrivesADefiniteLevel()
{
    // Each input shows its level also when it is the default, so a new one drives no Unknown
    InputSwitch input;
    InputButton button;
    Clock clock;
    QCOMPARE(input.outputPort()->status(), Status::Inactive);
    QCOMPARE(button.outputPort()->status(), Status::Inactive);
    QCOMPARE(clock.outputPort()->status(), Status::Inactive);

    InputRotary rotary;
    QCOMPARE(rotary.outputPort(0)->status(), Status::Active);
    for (int port = 1; port < rotary.outputSize(); ++port) {
        QCOMPARE(rotary.outputPort(port)->status(), Status::Inactive);
    }
}

void TestParameters::testARotarysSelectionOutOfRangeIsTheFirst()
{
    InputRotary rotary;
    rotary.setOutputSize(4);

    QVERIFY(rotary.setParam(ParamId::Position, 3));
    QCOMPARE(rotary.outputValue(), 3);
    QCOMPARE(rotary.outputPort(3)->status(), Status::Active);
    QCOMPARE(rotary.outputPort(0)->status(), Status::Inactive);

    // Out of range in either direction, as a file can hold, is the first output
    QVERIFY(rotary.setParam(ParamId::Position, 4));
    QCOMPARE(rotary.outputValue(), 0);
    QCOMPARE(rotary.outputPort(0)->status(), Status::Active);
    QVERIFY(rotary.setParam(ParamId::Position, 2));
    QVERIFY(rotary.setParam(ParamId::Position, -1));
    QCOMPARE(rotary.outputValue(), 0);
    rotary.setOn(true, 7);
    QCOMPARE(rotary.outputValue(), 0);
}

// --- The clock ---

void TestParameters::testAClocksFrequencyAndDelayAreItsRules()
{
    Clock clock;

    // A frequency is finite and non-zero with a half-period of at least a microsecond; one it does not take leaves
    // the frequency as it was, and acceptsFrequency() asks the same rule
    QVERIFY(clock.setParam(ParamId::Frequency, 2.5));
    QCOMPARE(clock.frequency(), 2.5);
    for (const double refused : {0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(),
                                 1.0e9}) {
        QVERIFY(!clock.acceptsFrequency(refused));
        QVERIFY(!clock.setParam(ParamId::Frequency, refused));
        clock.setFrequency(refused);
        QCOMPARE(clock.frequency(), 2.5);
    }
    QVERIFY(clock.acceptsFrequency(400000.0));
    clock.setFrequency(10.0);
    QCOMPARE(clock.param<double>(ParamId::Frequency), 10.0);

    // A delay is a fraction of the period in [-1, 1]: a finite one beyond it is clamped, a non-finite one refused
    QVERIFY(clock.setParam(ParamId::Delay, 0.25));
    QCOMPARE(clock.delay(), 0.25);
    clock.setDelay(3.0);
    QCOMPARE(clock.delay(), 1.0);
    clock.setDelay(-7.0);
    QCOMPARE(clock.delay(), -1.0);
    QVERIFY(!clock.setParam(ParamId::Delay, std::numeric_limits<double>::quiet_NaN()));
    QCOMPARE(clock.delay(), -1.0);

    // A type without them has none and takes none
    const std::unique_ptr<Element> gate(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!gate->acceptsFrequency(1.0));
    gate->setFrequency(1.0);
    gate->setDelay(0.5);
    QCOMPARE(gate->frequency(), 0.0);
    QCOMPARE(gate->delay(), 0.0);
}

// --- The colours ---

void TestParameters::testAColourIsAnyNameAndTheLedOffersItWithOneInput()
{
    // Any name is kept; the LED's pictures follow a listed one, and one not in the list is drawn as White
    Led led;
    QVERIFY(led.hasColors());
    led.setColor(QStringLiteral("Green"));
    QCOMPARE(led.param<QString>(ParamId::Colour), QStringLiteral("Green"));
    QCOMPARE(led.appearanceStates().constFirst().first, 4);
    QVERIFY(led.setParam(ParamId::Colour, QStringLiteral("Teal")));
    QCOMPARE(led.color(), QStringLiteral("Teal"));
    QCOMPARE(led.appearanceStates().constFirst().first, 0);

    // With more than one input the inputs' code picks the colour: the LED keeps it but does not offer it
    led.setColor(QStringLiteral("Blue"));
    led.setInputSize(2);
    QVERIFY(!led.hasColors());
    QVERIFY(led.hasParam(ParamId::Colour));
    QCOMPARE(led.color(), QStringLiteral("Blue"));
    led.setInputSize(1);
    QVERIFY(led.hasColors());
    QCOMPARE(led.appearanceStates().constFirst().first, 6);

    Display7 display;
    QVERIFY(display.hasColors());
    display.setColor(QStringLiteral("Purple"));
    QCOMPARE(display.color(), QStringLiteral("Purple"));

    // A type without a colour has none, offers none and takes none
    const std::unique_ptr<Element> gate(ElementFactory::buildElement(ElementType::And));
    gate->setColor(QStringLiteral("Red"));
    QVERIFY(!gate->hasColors());
    QCOMPARE(gate->color(), QString());
}

// --- The audio outputs ---

void TestParameters::testAnAudioOutputsFrequencyFileAndVolumeAreItsRules()
{
    Buzzer buzzer;

    // A frequency is finite and positive; one it does not take leaves the frequency as it was
    QVERIFY(buzzer.setParam(ParamId::Frequency, 440.0));
    QCOMPARE(buzzer.frequency(), 440.0);
    for (const double refused : {0.0, -5.0, std::numeric_limits<double>::quiet_NaN(),
                                 std::numeric_limits<double>::infinity()}) {
        QVERIFY(!buzzer.acceptsFrequency(refused));
        buzzer.setFrequency(refused);
        QCOMPARE(buzzer.frequency(), 440.0);
    }
    QVERIFY(buzzer.acceptsFrequency(1.0e9));

    // A volume is any value, at a float's precision: the nearest float to 0.1, written out, since i386's x87 can fold
    // static_cast<double>(0.1F) at a wider precision to 0.1 itself
    QVERIFY(buzzer.setParam(ParamId::Volume, 0.1));
    QCOMPARE(buzzer.param<double>(ParamId::Volume), kNearestFloatToATenth);
    QCOMPARE(buzzer.volume(), 0.1F);

    // A sound file's path is kept whether or not its file is there; an empty one changes nothing
    AudioBox box;
    box.setAudio(QString());
    QCOMPARE(box.audio(), QString::fromLatin1(AudioBox::kDefaultAudioPath));
    const QString gone = QStringLiteral("/nonexistent/wiredpanda/gone.wav");
    box.setAudio(gone);
    QCOMPARE(box.param<QString>(ParamId::AudioFile), gone);
    QCOMPARE(box.externalFiles(), QStringList{gone});
    box.setVolume(0.75F);
    QCOMPARE(box.volume(), 0.75F);

    // With an output device, a new frequency and volume reach it
    if (auto *tone = buzzer.findChild<ToneGenerator *>()) {
        buzzer.setFrequency(880.0);
        QCOMPARE(tone->frequency(), 880.0);
        buzzer.setVolume(0.2F);
        QCOMPARE(buzzer.findChild<QAudioSink *>()->volume(), static_cast<qreal>(buzzer.volume()));
    }
    if (auto *output = box.findChild<QAudioOutput *>()) {
        box.setVolume(0.3F);
        QCOMPARE(output->volume(), 0.3F);
    }

    // A type without them has none and takes none
    const std::unique_ptr<Element> gate(ElementFactory::buildElement(ElementType::And));
    gate->setAudio(gone);
    gate->setVolume(0.5F);
    QCOMPARE(gate->audio(), QString());
    QCOMPARE(gate->volume(), 0.0F);
}

void TestParameters::testAnAudioBoxReadsItsFileAgainOnEveryLoad()
{
    // A snapshot restored with the path the box has reads its file again: a sound whose file has gone since is
    // reported, not taken as loaded
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("sound.wav"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("DUMMY AUDIO DATA");
    file.close();

    AudioBox box;
    box.setAudio(path);
    box.reportFailedFiles();

    QByteArray snapshot;
    QDataStream out(&snapshot, QIODevice::WriteOnly);
    box.save(out, {.purpose = SerializationPurpose::InMemorySnapshot});
    QVERIFY(QFile::remove(path));

    QDataStream in(snapshot);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::InMemorySnapshot, {}};
    box.load(in, context);
    QCOMPARE(box.audio(), path);

    QString message;
    try {
        box.reportFailedFiles();
    } catch (const Pandaception &e) {
        message = QString::fromUtf8(e.what());
    }
    QVERIFY2(message.contains(QStringLiteral("sound.wav")), qPrintable(message));
}
