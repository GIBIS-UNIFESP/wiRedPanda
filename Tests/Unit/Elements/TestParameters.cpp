// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestParameters.h"

#include <memory>

#include <QHash>
#include <QMetaEnum>
#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementTypeDescriptor.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/Parameters.h"
#include "App/Wiring/Port.h"

namespace ParametersTestDetail {

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
        {ElementType::Node, {"Wireless 0 wirelessMode"}},
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
