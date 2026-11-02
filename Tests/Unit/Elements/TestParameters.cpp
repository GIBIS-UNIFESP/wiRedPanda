// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestParameters.h"

#include <limits>
#include <memory>

#include <QDataStream>
#include <QFile>
#include <QHash>
#include <QMetaEnum>
#include <QTemporaryDir>
#include <QTest>

#include "App/Core/Common.h"
#include "App/Element/AudioOutput.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementSerializer.h"
#include "App/Element/ElementTypeDescriptor.h"
#include "App/Element/Parameters.h"
#include "App/Element/WirelessNode.h"
#include "App/IO/SerializationContext.h"
#include "App/Versions.h"
#include "App/Wiring/Port.h"

namespace ParametersTestDetail {

/// The float nearest to 0.1, as a double.
constexpr double kNearestFloatToATenth = 0.100000001490116119384765625;
/// The nearest float to 0.35, the buzzer's default volume as every earlier version kept it.
constexpr double kNearestFloatToTheBuzzersVolume = 0.3499999940395355224609375;
/// The nearest float to 0.3.
constexpr double kNearestFloatToThreeTenths = 0.300000011920928955078125;

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
    case ParamId::Playing:   return QStringLiteral("Playing");
    case ParamId::Lit:       return QStringLiteral("Lit");
    case ParamId::Carried:   return QStringLiteral("Carried");
    case ParamId::Wired:     return QStringLiteral("Wired");
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

/// Returns the map \a element's parameters are written as for \a purpose.
QMap<QString, QVariant> savedMap(const Element &element, const SerializationPurpose purpose)
{
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    ElementSerializer::saveParameters(element, out, {.purpose = purpose});
    QDataStream in(bytes);
    QMap<QString, QVariant> map;
    in >> map;
    return map;
}

/// Reads \a map into \a element's parameters as a portable file of \a version holds it; returns how many bytes of it
/// the load read.
qint64 loadMap(Element &element, const QMap<QString, QVariant> &map, const QVersionNumber &version)
{
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    out << map;
    QDataStream in(bytes);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, version, SerializationPurpose::PortableFile, {}};
    ElementSerializer::loadParameters(element, in, context);
    return in.device()->pos();
}

/// A node counting what it is told of its parameters.
class CountingNode : public Element
{
public:
    CountingNode()
        : Element(ElementType::Node)
    {
    }

    int changes = 0;
    ParamValue previous;

protected:
    void parameterChanged(const ParamId id, const ParamValue &before) override
    {
        Element::parameterChanged(id, before);
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
        {ElementType::Node, {"Wireless 0 wirelessMode", "Carried -1 -", "Wired true -"}},
        {ElementType::Led, {"Colour \"White\" color", "Lit 0 -"}},
        {ElementType::Display7, {"Colour \"Red\" color"}},
        {ElementType::Display14, {"Colour \"Red\" color"}},
        {ElementType::Display16, {"Colour \"Red\" color"}},
        {ElementType::Buzzer, {"Frequency 1047 frequency", "Volume 0.34999999999999998 volume", "Playing false -"}},
        {ElementType::AudioBox,
         {"AudioFile \":/Components/Output/Audio/wiredpanda.wav\" audiobox", "Volume 0.5 volume", "Playing false -"}},
        {ElementType::TruthTable, {"Table bits 0/2048 key"}},
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

void TestParameters::testEachTypeOffersThePropertiesOfItsParameters_data()
{
    QTest::addColumn<ElementType>("type");
    QTest::addColumn<QStringList>("expected");

    // What the editor offers of each type's parameters, as a new element has it: the flags the types had before
    // their schemas. A type not listed offers none.
    const QHash<ElementType, QStringList> offered{
        {ElementType::Clock, {"frequency", "delay"}},
        {ElementType::Buzzer, {"frequency", "volume"}},
        {ElementType::AudioBox, {"sound", "volume"}},
        {ElementType::Led, {"colour"}},
        {ElementType::Display7, {"colour"}},
        {ElementType::Display14, {"colour"}},
        {ElementType::Display16, {"colour"}},
        {ElementType::TruthTable, {"table"}},
        {ElementType::Node, {"wireless"}},
    };

    const QMetaEnum metaEnum = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(metaEnum.value(i));
        if ((type == ElementType::Unknown) || (type == ElementType::JKLatch)) {
            continue;
        }
        QTest::newRow(metaEnum.key(i)) << type << offered.value(type);
    }
}

void TestParameters::testEachTypeOffersThePropertiesOfItsParameters()
{
    QFETCH(ElementType, type);
    QFETCH(QStringList, expected);

    const std::unique_ptr<Element> element(ElementFactory::buildElement(type));
    QStringList offers;
    for (const auto &[name, has] : {std::pair{"frequency", element->offersParam(ParamId::Frequency)},
                                    std::pair{"delay", element->offersParam(ParamId::Delay)},
                                    std::pair{"sound", element->offersParam(ParamId::AudioFile)},
                                    std::pair{"volume", element->offersParam(ParamId::Volume)},
                                    std::pair{"colour", element->offersParam(ParamId::Colour)},
                                    std::pair{"table", element->hasTruthTable()},
                                    std::pair{"wireless", element->offersParam(ParamId::Wireless)}}) {
        if (has) {
            offers << QString::fromLatin1(name);
        }
    }
    QCOMPARE(offers, expected);
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
    Element node{ElementType::Node};

    QVERIFY(node.setParam(ParamId::Wireless, static_cast<int>(WirelessMode::Rx)));
    QCOMPARE(WirelessNode::mode(node), WirelessMode::Rx);
    QVERIFY(!node.inputPort()->isRequired());
    QVERIFY(!node.inputPort()->isVisible());

    // The value it has changes nothing; nor do a value of another kind and a parameter the type does not have, broken
    // invariants, which a debug build asserts
    QVERIFY(!node.setParam(ParamId::Wireless, static_cast<int>(WirelessMode::Rx)));
#ifdef QT_NO_DEBUG
    QVERIFY(!node.setParam(ParamId::Wireless, QStringLiteral("Tx")));
    QVERIFY(!node.setParam(ParamId::Colour, QStringLiteral("Red")));
    QVERIFY(!node.hasParam(ParamId::Colour));
#endif
    QCOMPARE(WirelessNode::mode(node), WirelessMode::Rx);

    // The spec takes a mode out of range as None, as a file's loads
    QVERIFY(node.setParam(ParamId::Wireless, 7));
    QCOMPARE(WirelessNode::mode(node), WirelessMode::None);
    QVERIFY(node.inputPort()->isRequired());
    QVERIFY(node.inputPort()->isVisible());
    QVERIFY(!node.setParam(ParamId::Wireless, -1));

    // The accessor goes through the same spec
    WirelessNode::setMode(node, WirelessMode::Tx);
    QCOMPARE(WirelessNode::mode(node), WirelessMode::Tx);
    QVERIFY(!node.outputPort()->isVisible());
    WirelessNode::setMode(node, static_cast<WirelessMode>(3));
    QCOMPARE(WirelessNode::mode(node), WirelessMode::None);

    // A type without the parameter has none, and routes nothing
    const std::unique_ptr<Element> gate(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!gate->hasParam(ParamId::Wireless));
    QVERIFY(!gate->offersParam(ParamId::Wireless));
    QCOMPARE(WirelessNode::mode(*gate), WirelessMode::None);
}

void TestParameters::testParameterChangedFollowsEachChangeOnly()
{
    CountingNode node;

    QVERIFY(node.setParam(ParamId::Wireless, static_cast<int>(WirelessMode::Tx)));
    QCOMPARE(node.changes, 1);
    QCOMPARE(node.previous, ParamValue(static_cast<int>(WirelessMode::None)));

    QVERIFY(!node.setParam(ParamId::Wireless, static_cast<int>(WirelessMode::Tx)));
#ifdef QT_NO_DEBUG
    QVERIFY(!node.setParam(ParamId::Wireless, 2.0)); // A value of another kind, which a debug build asserts
#endif
    QCOMPARE(node.changes, 1);

    QVERIFY(node.setParam(ParamId::Wireless, 9));
    QCOMPARE(node.changes, 2);
    QCOMPARE(node.previous, ParamValue(static_cast<int>(WirelessMode::Tx)));
}

// --- The inputs ---

void TestParameters::testAnInputsLevelAndLockAreItsParameters()
{
    Element input{ElementType::InputSwitch};
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
    Element input{ElementType::InputSwitch};
    Element button{ElementType::InputButton};
    Element clock{ElementType::Clock};
    QCOMPARE(input.outputPort()->status(), Status::Inactive);
    QCOMPARE(button.outputPort()->status(), Status::Inactive);
    QCOMPARE(clock.outputPort()->status(), Status::Inactive);

    Element rotary{ElementType::InputRotary};
    QCOMPARE(rotary.outputPort(0)->status(), Status::Active);
    for (int port = 1; port < rotary.outputSize(); ++port) {
        QCOMPARE(rotary.outputPort(port)->status(), Status::Inactive);
    }
}

void TestParameters::testARotarysSelectionOutOfRangeIsTheFirst()
{
    Element rotary{ElementType::InputRotary};
    rotary.setOutputSize(4);

    QVERIFY(rotary.setParam(ParamId::Position, 3));
    QCOMPARE(rotary.param<int>(ParamId::Position), 3);
    QCOMPARE(rotary.outputPort(3)->status(), Status::Active);
    QCOMPARE(rotary.outputPort(0)->status(), Status::Inactive);

    // Out of range in either direction, as a file can hold, is the first output
    QVERIFY(rotary.setParam(ParamId::Position, 4));
    QCOMPARE(rotary.param<int>(ParamId::Position), 0);
    QCOMPARE(rotary.outputPort(0)->status(), Status::Active);
    QVERIFY(rotary.setParam(ParamId::Position, 2));
    QVERIFY(rotary.setParam(ParamId::Position, -1));
    QCOMPARE(rotary.param<int>(ParamId::Position), 0);
    rotary.setOn(true, 7);
    QCOMPARE(rotary.param<int>(ParamId::Position), 0);
}

// --- The clock ---

void TestParameters::testAClocksFrequencyAndDelayAreItsRules()
{
    Element clock{ElementType::Clock};

    // A frequency is finite and non-zero with a half-period of at least a microsecond; one it does not take leaves
    // the frequency as it was
    QVERIFY(clock.setParam(ParamId::Frequency, 2.5));
    QCOMPARE(clock.param<double>(ParamId::Frequency), 2.5);
    for (const double refused : {0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(),
                                 1.0e9}) {
        QVERIFY(!clock.setParam(ParamId::Frequency, refused));
        clock.setParam(ParamId::Frequency, refused);
        QCOMPARE(clock.param<double>(ParamId::Frequency), 2.5);
    }
    QVERIFY(clock.setParam(ParamId::Frequency, 400000.0));
    clock.setParam(ParamId::Frequency, 10.0);
    QCOMPARE(clock.param<double>(ParamId::Frequency), 10.0);

    // A delay is a fraction of the period in [-1, 1]: a finite one beyond it is clamped, a non-finite one refused
    QVERIFY(clock.setParam(ParamId::Delay, 0.25));
    QCOMPARE(clock.param<double>(ParamId::Delay), 0.25);
    clock.setParam(ParamId::Delay, 3.0);
    QCOMPARE(clock.param<double>(ParamId::Delay), 1.0);
    clock.setParam(ParamId::Delay, -7.0);
    QCOMPARE(clock.param<double>(ParamId::Delay), -1.0);
    QVERIFY(!clock.setParam(ParamId::Delay, std::numeric_limits<double>::quiet_NaN()));
    QCOMPARE(clock.param<double>(ParamId::Delay), -1.0);

    // A type without them has none and offers none
    const std::unique_ptr<Element> gate(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!gate->hasParam(ParamId::Frequency));
    QVERIFY(!gate->hasParam(ParamId::Delay));
    QVERIFY(!gate->offersParam(ParamId::Frequency));
    QVERIFY(!gate->offersParam(ParamId::Delay));
}

// --- The colours ---

void TestParameters::testAColourIsAnyNameAndTheLedOffersItWithOneInput()
{
    // Any name is kept; the LED's pictures follow a listed one, and one not in the list is drawn as White
    Element led{ElementType::Led};
    QVERIFY(led.offersParam(ParamId::Colour));
    led.setParam(ParamId::Colour, QStringLiteral("Green"));
    QCOMPARE(led.param<QString>(ParamId::Colour), QStringLiteral("Green"));
    QCOMPARE(led.appearanceStates().constFirst().first, 4);
    QVERIFY(led.setParam(ParamId::Colour, QStringLiteral("Teal")));
    QCOMPARE(led.param<QString>(ParamId::Colour), QStringLiteral("Teal"));
    QCOMPARE(led.appearanceStates().constFirst().first, 0);

    // With more than one input the inputs' code picks the colour: the LED keeps it but does not offer it
    led.setParam(ParamId::Colour, QStringLiteral("Blue"));
    led.setInputSize(2);
    QVERIFY(!led.offersParam(ParamId::Colour));
    QVERIFY(led.hasParam(ParamId::Colour));
    QCOMPARE(led.param<QString>(ParamId::Colour), QStringLiteral("Blue"));
    led.setInputSize(1);
    QVERIFY(led.offersParam(ParamId::Colour));
    QCOMPARE(led.appearanceStates().constFirst().first, 6);

    Element display{ElementType::Display7};
    QVERIFY(display.offersParam(ParamId::Colour));
    display.setParam(ParamId::Colour, QStringLiteral("Purple"));
    QCOMPARE(display.param<QString>(ParamId::Colour), QStringLiteral("Purple"));

    // A type without a colour has none and offers none
    const std::unique_ptr<Element> gate(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!gate->hasParam(ParamId::Colour));
    QVERIFY(!gate->offersParam(ParamId::Colour));
}

// --- The audio outputs ---

void TestParameters::testAnAudioOutputsFrequencyFileAndVolumeAreItsRules()
{
    Element buzzer{ElementType::Buzzer};

    // A frequency is finite and positive; one it does not take leaves the frequency as it was
    QVERIFY(buzzer.setParam(ParamId::Frequency, 440.0));
    QCOMPARE(buzzer.param<double>(ParamId::Frequency), 440.0);
    for (const double refused : {0.0, -5.0, std::numeric_limits<double>::quiet_NaN(),
                                 std::numeric_limits<double>::infinity()}) {
        QVERIFY(!buzzer.setParam(ParamId::Frequency, refused));
        buzzer.setParam(ParamId::Frequency, refused);
        QCOMPARE(buzzer.param<double>(ParamId::Frequency), 440.0);
    }
    QVERIFY(buzzer.setParam(ParamId::Frequency, 1.0e9));

    // A volume is any value as it is, but one that is exactly a float, as every earlier version kept the volume, is that
    // float's shortest decimal: the nearest float to 0.1 is written out, since i386's x87 can fold
    // static_cast<double>(0.1F) at a wider precision to 0.1 itself
    QVERIFY(buzzer.setParam(ParamId::Volume, 0.3));
    QCOMPARE(buzzer.param<double>(ParamId::Volume), 0.3);
    QVERIFY(buzzer.setParam(ParamId::Volume, kNearestFloatToATenth));
    QCOMPARE(buzzer.param<double>(ParamId::Volume), 0.1);
    QVERIFY(!buzzer.setParam(ParamId::Volume, 0.1));
    QVERIFY(buzzer.setParam(ParamId::Volume, 0.5));
    QCOMPARE(buzzer.param<double>(ParamId::Volume), 0.5);
    QCOMPARE(AudioOutput::volumeTaken(0.1 + 1e-12), 0.1 + 1e-12);

    // A sound file's path is kept whether or not its file is there; an empty one changes nothing
    Element box{ElementType::AudioBox};
    box.setParam(ParamId::AudioFile, QString());
    QCOMPARE(box.param<QString>(ParamId::AudioFile), QString::fromLatin1(AudioOutput::kBuiltInSound));
    const QString gone = QStringLiteral("/nonexistent/wiredpanda/gone.wav");
    box.setParam(ParamId::AudioFile, gone);
    QCOMPARE(box.param<QString>(ParamId::AudioFile), gone);
    QCOMPARE(box.externalFiles(), QStringList{gone});
    box.setParam(ParamId::Volume, 0.75);
    QCOMPARE(box.param<double>(ParamId::Volume), 0.75);

    // A type without them has none and offers none
    const std::unique_ptr<Element> gate(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!gate->hasParam(ParamId::AudioFile));
    QVERIFY(!gate->hasParam(ParamId::Volume));
    QVERIFY(!gate->offersParam(ParamId::AudioFile));
    QVERIFY(!gate->offersParam(ParamId::Volume));
}

void TestParameters::testAnAudioBoxReportsItsSoundAsItIsWhenAFileOpens()
{
    // The sound is checked when a file reports it, whatever it was when it was set: one gone since is reported, and
    // one gone then but back since is not
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("sound.wav"));
    const auto writeSound = [&path] {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("DUMMY AUDIO DATA");
    };
    const auto report = [](const Element &box) {
        try {
            box.reportFailedFiles();
        } catch (const Pandaception &e) {
            return QString::fromUtf8(e.what());
        }
        return QString();
    };

    writeSound();
    Element box{ElementType::AudioBox};
    box.setParam(ParamId::AudioFile, path);
    QCOMPARE(report(box), QString());
    QVERIFY(QFile::remove(path));
    QVERIFY2(report(box).contains(QStringLiteral("sound.wav")), "a sound gone since it was set is not reported");

    Element other{ElementType::AudioBox};
    other.setParam(ParamId::AudioFile, path);
    writeSound();
    QCOMPARE(report(other), QString());
}

// --- The truth table ---

void TestParameters::testATruthTablesBitsAreAlwaysAWholeTable()
{
    // Every table holds 2048 bits, 256 rows for each of 8 outputs: a shorter one is padded with zeros, a longer one
    // truncated, the bits given kept
    Element table{ElementType::TruthTable};
    QBitArray bits(16);
    bits.setBit(0, true);
    bits.setBit(15, true);
    QVERIFY(table.setParam(ParamId::Table, bits));
    QCOMPARE(table.param<QBitArray>(ParamId::Table).size(), 2048);
    QCOMPARE(table.param<QBitArray>(ParamId::Table).count(true), 2);
    QVERIFY(table.param<QBitArray>(ParamId::Table).at(0) && table.param<QBitArray>(ParamId::Table).at(15));

    table.setParam(ParamId::Table, QBitArray(4096, true));
    QCOMPARE(table.param<QBitArray>(ParamId::Table), QBitArray(2048, true));

    // The same table again is no change
    QVERIFY(!table.setParam(ParamId::Table, QBitArray(2048, true)));
}

// --- Files ---

void TestParameters::testAPortableFileOmitsDefaultsButASnapshotWritesEveryKey()
{
    // A portable file omits a default, except the truth table's, and a sound file that is a resource; a snapshot,
    // loaded back into a live element, writes every keyed parameter and no transient one
    Element clock{ElementType::Clock};
    QCOMPARE(savedMap(clock, SerializationPurpose::PortableFile), (QMap<QString, QVariant>{}));
    QCOMPARE(savedMap(clock, SerializationPurpose::InMemorySnapshot).keys(),
             (QStringList{"delay", "frequency", "locked"}));
    clock.setParam(ParamId::Frequency, 4.0);
    QCOMPARE(savedMap(clock, SerializationPurpose::PortableFile), (QMap<QString, QVariant>{{"frequency", 4.0}}));

    Element table{ElementType::TruthTable};
    QCOMPARE(savedMap(table, SerializationPurpose::PortableFile).keys(), QStringList{"key"});

    Element box{ElementType::AudioBox};
    QCOMPARE(savedMap(box, SerializationPurpose::PortableFile), (QMap<QString, QVariant>{}));
    const QMap<QString, QVariant> snapshot = savedMap(box, SerializationPurpose::InMemorySnapshot);
    QCOMPARE(snapshot.value("audiobox").toString(), QString::fromLatin1(AudioOutput::kBuiltInSound));
    QCOMPARE(snapshot.value("volume").toDouble(), 0.5);

    // A type without keyed parameters writes nothing, not even an empty map
    const std::unique_ptr<Element> gate(ElementFactory::buildElement(ElementType::And));
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    ElementSerializer::saveParameters(*gate, out, {.purpose = SerializationPurpose::InMemorySnapshot});
    QVERIFY(bytes.isEmpty());
}

void TestParameters::testALoadReadsEachValueFromTheVersionItMeansSomething()
{
    // The clock's delay before 4.3 had another meaning and is dropped
    Element clock{ElementType::Clock};
    loadMap(clock, {{"frequency", 2.0}, {"delay", 0.5}}, QVersionNumber(4, 2));
    QCOMPARE(clock.param<double>(ParamId::Frequency), 2.0);
    QCOMPARE(clock.param<double>(ParamId::Delay), 0.0);
    loadMap(clock, {{"delay", 0.5}}, QVersionNumber(4, 3));
    QCOMPARE(clock.param<double>(ParamId::Delay), 0.5);

    // A buzzer's map without a frequency can hold the old note name
    Element buzzer{ElementType::Buzzer};
    loadMap(buzzer, {{"note", "A4"}}, QVersionNumber(4, 1));
    QCOMPARE(buzzer.param<double>(ParamId::Frequency), static_cast<double>(AudioOutput::noteToFrequency("A4")));
    loadMap(buzzer, {{"frequency", 300.0}, {"note", "C6"}}, QVersionNumber(4, 1));
    QCOMPARE(buzzer.param<double>(ParamId::Frequency), 300.0);

    // A node's map came in 4.4: an older file holds none, and nothing is read
    Element node{ElementType::Node};
    QCOMPARE(loadMap(node, {{"wirelessMode", 1}}, QVersionNumber(4, 3)), qint64{0});
    QCOMPARE(WirelessNode::mode(node), WirelessMode::None);
    loadMap(node, {{"wirelessMode", 1}}, QVersionNumber(4, 4));
    QCOMPARE(WirelessNode::mode(node), WirelessMode::Tx);

    // A value that does not read as its kind is dropped
    Element rotary{ElementType::InputRotary};
    rotary.setOn(true, 1);
    loadMap(rotary, {{"currentPort", "first"}}, QVersionNumber(4, 1));
    QCOMPARE(rotary.param<int>(ParamId::Position), 1);
}

void TestParameters::testAnOlderFilesVolumeIsTheValueItsFloatNamed()
{
    // Every earlier version kept the volume as a float, which its files hold widened: each such value reads as the
    // float's shortest decimal, the volume the user set, the default included; a volume that is no float, as this
    // version writes 0.1, reads as written
    Element buzzer{ElementType::Buzzer};
    loadMap(buzzer, {{"volume", kNearestFloatToATenth}}, QVersionNumber(4, 1));
    QCOMPARE(buzzer.param<double>(ParamId::Volume), 0.1);
    loadMap(buzzer, {{"volume", kNearestFloatToTheBuzzersVolume}}, FormatRev::current);
    QCOMPARE(buzzer.param<double>(ParamId::Volume), AudioOutput::kBuzzerVolume);
    loadMap(buzzer, {{"volume", 0.3}}, FormatRev::current);
    QCOMPARE(buzzer.param<double>(ParamId::Volume), 0.3);

    // A volume that is not finite is refused: a file's loads as the default, and is saved as none
    for (const double refused : {std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(),
                                 std::numeric_limits<double>::quiet_NaN()}) {
        Element fresh{ElementType::Buzzer};
        loadMap(fresh, {{"volume", refused}}, FormatRev::current);
        QCOMPARE(fresh.param<double>(ParamId::Volume), AudioOutput::kBuzzerVolume);
        QVERIFY(!savedMap(fresh, SerializationPurpose::PortableFile).contains("volume"));
        QVERIFY(!fresh.setParam(ParamId::Volume, refused));
    }
}

void TestParameters::testTheVolumesRuleTakesEachFloatOnce()
{
    // Every volume the slider gave as a float, k / 100, becomes the decimal k / 100 the float names, and stays so;
    // a double the slider gives now is that decimal already. The float is assigned first, as i386's x87 would keep the
    // quotient wider than a float.
    for (int percent = 0; percent <= 100; ++percent) {
        const float single = static_cast<float>(percent) / 100.0f;
        const double once = AudioOutput::volumeTaken(static_cast<double>(single));
        QCOMPARE(once, percent / 100.0);
        QCOMPARE(AudioOutput::volumeTaken(once), once);
        QCOMPARE(AudioOutput::volumeTaken(percent / 100.0), percent / 100.0);
    }
    for (const double volume : {AudioOutput::kBuzzerVolume, AudioOutput::kBoxVolume}) {
        QCOMPARE(AudioOutput::volumeTaken(volume), volume);
    }
    QCOMPARE(AudioOutput::volumeTaken(kNearestFloatToTheBuzzersVolume), AudioOutput::kBuzzerVolume);

    // Kept: a double that is no float (0.8), and one exact both as a float and as its decimal (0.5, 0.375). Moved: only
    // a float whose decimal is no float, which a writer meaning that float's digits gets as the decimal
    for (const double kept : {0.8, 0.5, 0.375}) {
        QCOMPARE(AudioOutput::volumeTaken(kept), kept);
    }
    QCOMPARE(AudioOutput::volumeTaken(kNearestFloatToThreeTenths), 0.3);
}

void TestParameters::testAnOlderFilesVolumeSavesAsItsDecimal()
{
    // An older file's volume, a float widened, reads as its decimal and is saved so; that file read and saved again,
    // as a newer file's volume is, is byte for byte what it saved
    Element buzzer{ElementType::Buzzer};
    loadMap(buzzer, {{"volume", kNearestFloatToThreeTenths}}, QVersionNumber(4, 1));
    const QMap<QString, QVariant> saved = savedMap(buzzer, SerializationPurpose::PortableFile);
    QCOMPARE(saved.value("volume").toDouble(), 0.3);

    Element again{ElementType::Buzzer};
    loadMap(again, saved, FormatRev::current);
    QCOMPARE(savedMap(again, SerializationPurpose::PortableFile), saved);
}

void TestParameters::testEveryLoadDrivesASwitchsLevelAndARotarysSelection()
{
    // A load that leaves the level or the selection as it was still drives it, on the outputs the load added too
    Element toggle{ElementType::InputSwitch};
    toggle.outputPort(0)->setStatus(Status::Unknown);
    loadMap(toggle, {}, QVersionNumber(4, 1));
    QCOMPARE(toggle.outputPort(0)->status(), Status::Inactive);

    Element rotary{ElementType::InputRotary};
    rotary.setOutputSize(4);
    for (int port = 0; port < rotary.outputSize(); ++port) {
        rotary.outputPort(port)->setStatus(Status::Unknown);
    }
    loadMap(rotary, {}, QVersionNumber(4, 1));
    QCOMPARE(rotary.outputPort(0)->status(), Status::Active);
    for (int port = 1; port < rotary.outputSize(); ++port) {
        QCOMPARE(rotary.outputPort(port)->status(), Status::Inactive);
    }
}
