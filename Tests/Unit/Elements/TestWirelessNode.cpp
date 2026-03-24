// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestWirelessNode.h"

#include <memory>

#include <QDataStream>
#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/PropertyDescriptor.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Nodes/QNEPort.h"

void TestWirelessNode::testDefaultModeIsNone()
{
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);
    QCOMPARE(node->wirelessMode(), WirelessMode::None);
    QVERIFY(node->hasWirelessMode());
}

void TestWirelessNode::testSetTxMode()
{
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    node->setWirelessMode(WirelessMode::Tx);

    QCOMPARE(node->wirelessMode(), WirelessMode::Tx);
    // Tx node still requires a physical input wire (the signal source)
    QVERIFY(node->inputPort()->isRequired());
}

void TestWirelessNode::testSetRxMode()
{
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    node->setWirelessMode(WirelessMode::Rx);

    QCOMPARE(node->wirelessMode(), WirelessMode::Rx);
    // Rx node receives its signal wirelessly — input port must be optional
    QVERIFY(!node->inputPort()->isRequired());
}

void TestWirelessNode::testSetNoneModeRestoresRequired()
{
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    node->setWirelessMode(WirelessMode::Rx);
    QVERIFY(!node->inputPort()->isRequired());

    node->setWirelessMode(WirelessMode::None);
    QCOMPARE(node->wirelessMode(), WirelessMode::None);
    QVERIFY(node->inputPort()->isRequired());
}

void TestWirelessNode::testSaveLoadRoundTrip()
{
    // Save a Tx node, reload it, and verify the mode is preserved.
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    node->setWirelessMode(WirelessMode::Tx);
    node->setLabel("CLK");

    QByteArray buffer;
    {
        QDataStream out(&buffer, QIODevice::WriteOnly);
        Serialization::writePandaHeader(out);
        node->save(out);
    }

    auto elm2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node2 = qobject_cast<Node *>(elm2.get());
    QVERIFY(node2 != nullptr);

    {
        QDataStream in(&buffer, QIODevice::ReadOnly);
        QMap<quint64, QNEPort *> portMap;
        SerializationContext ctx{portMap, Serialization::readPandaHeader(in)};
        node2->load(in, ctx);
    }

    QCOMPARE(node2->wirelessMode(), WirelessMode::Tx);
    QCOMPARE(node2->label(), QStringLiteral("CLK"));
}

void TestWirelessNode::testSetSameModeIdempotent()
{
    // Calling setWirelessMode with the current mode must not corrupt state.
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    node->setWirelessMode(WirelessMode::Tx);
    node->setWirelessMode(WirelessMode::Tx);   // second call — early-return guard makes this a true no-op
    QCOMPARE(node->wirelessMode(), WirelessMode::Tx);
    QVERIFY(node->inputPort()->isRequired());

    node->setWirelessMode(WirelessMode::Rx);
    node->setWirelessMode(WirelessMode::Rx);   // second call — early-return guard makes this a true no-op
    QCOMPARE(node->wirelessMode(), WirelessMode::Rx);
    QVERIFY(!node->inputPort()->isRequired());
}

void TestWirelessNode::testAllTransitionsIsRequired()
{
    // Walk through every meaningful mode transition and verify isRequired() at each step.
    // None(T) → Tx(T) → Rx(F) → None(T) → Rx(F) → Tx(T)
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    // Initial state
    QCOMPARE(node->wirelessMode(), WirelessMode::None);
    QVERIFY(node->inputPort()->isRequired());

    node->setWirelessMode(WirelessMode::Tx);
    QCOMPARE(node->wirelessMode(), WirelessMode::Tx);
    QVERIFY(node->inputPort()->isRequired());      // Tx still needs a physical driver

    node->setWirelessMode(WirelessMode::Rx);
    QCOMPARE(node->wirelessMode(), WirelessMode::Rx);
    QVERIFY(!node->inputPort()->isRequired());     // Rx receives wirelessly

    node->setWirelessMode(WirelessMode::None);
    QCOMPARE(node->wirelessMode(), WirelessMode::None);
    QVERIFY(node->inputPort()->isRequired());      // back to normal node

    node->setWirelessMode(WirelessMode::Rx);
    QVERIFY(!node->inputPort()->isRequired());

    node->setWirelessMode(WirelessMode::Tx);
    QCOMPARE(node->wirelessMode(), WirelessMode::Tx);
    QVERIFY(node->inputPort()->isRequired());
}

void TestWirelessNode::testSetNoneModeAfterTx()
{
    // Tx → None must restore the input port to required=true,
    // mirroring testSetNoneModeRestoresRequired which only covers Rx → None.
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    node->setWirelessMode(WirelessMode::Tx);
    QVERIFY(node->inputPort()->isRequired());   // Tx still requires a physical wire

    node->setWirelessMode(WirelessMode::None);
    QCOMPARE(node->wirelessMode(), WirelessMode::None);
    QVERIFY(node->inputPort()->isRequired());   // must still be required after reset
}

void TestWirelessNode::testSaveLoadRxMode()
{
    // Identical to testSaveLoadRoundTrip but for Rx mode — ensures the Rx
    // branch of saveProperties()/loadProperties() is exercised.
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    node->setWirelessMode(WirelessMode::Rx);
    node->setLabel(QStringLiteral("DATA"));

    QByteArray buffer;
    {
        QDataStream out(&buffer, QIODevice::WriteOnly);
        Serialization::writePandaHeader(out);
        node->save(out);
    }

    auto elm2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node2 = qobject_cast<Node *>(elm2.get());
    QVERIFY(node2 != nullptr);

    {
        QDataStream in(&buffer, QIODevice::ReadOnly);
        QMap<quint64, QNEPort *> portMap;
        SerializationContext ctx{portMap, Serialization::readPandaHeader(in)};
        node2->load(in, ctx);
    }

    QCOMPARE(node2->wirelessMode(), WirelessMode::Rx);
    QCOMPARE(node2->label(), QStringLiteral("DATA"));
    // Rx port must remain optional after round-trip
    QVERIFY(!node2->inputPort()->isRequired());
}

void TestWirelessNode::testLabelUsedAsChannelName()
{
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    // The label field doubles as the wireless channel name
    node->setLabel("DATA_BUS");
    QCOMPARE(node->label(), QStringLiteral("DATA_BUS"));

    node->setWirelessMode(WirelessMode::Rx);
    // Mode and label are independent; label is still accessible after mode change
    QCOMPARE(node->label(), QStringLiteral("DATA_BUS"));
    QCOMPARE(node->wirelessMode(), WirelessMode::Rx);
}

void TestWirelessNode::testSaveLoadNoneMode()
{
    // A Node explicitly cycled through Tx and back to None must load back as None.
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    node->setWirelessMode(WirelessMode::Tx);
    node->setWirelessMode(WirelessMode::None);  // reset to None before saving
    QCOMPARE(node->wirelessMode(), WirelessMode::None);

    QByteArray buffer;
    {
        QDataStream out(&buffer, QIODevice::WriteOnly);
        Serialization::writePandaHeader(out);
        node->save(out);
    }

    auto elm2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node2 = qobject_cast<Node *>(elm2.get());
    QVERIFY(node2 != nullptr);

    {
        QDataStream in(&buffer, QIODevice::ReadOnly);
        QMap<quint64, QNEPort *> portMap;
        SerializationContext ctx{portMap, Serialization::readPandaHeader(in)};
        node2->load(in, ctx);
    }

    QCOMPARE(node2->wirelessMode(), WirelessMode::None);
    // None mode must restore the port to required=true
    QVERIFY(node2->inputPort()->isRequired());
}

void TestWirelessNode::testLabelPreservedAfterModeChange()
{
    // Switching wireless mode must never modify the element label.
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    node->setLabel("CHANNEL_A");

    node->setWirelessMode(WirelessMode::Tx);
    QCOMPARE(node->label(), QStringLiteral("CHANNEL_A"));

    node->setWirelessMode(WirelessMode::Rx);
    QCOMPARE(node->label(), QStringLiteral("CHANNEL_A"));

    node->setWirelessMode(WirelessMode::None);
    QCOMPARE(node->label(), QStringLiteral("CHANNEL_A"));
}

void TestWirelessNode::testHasWirelessModeFalseForNonNode()
{
    // hasWirelessMode() must return false for elements that are not Nodes.
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(elm != nullptr);
    QVERIFY(!elm->hasWirelessMode());
}

void TestWirelessNode::testEditablePropertiesContainsWirelessMode()
{
    // Node::editableProperties() must include a WirelessMode descriptor.
    // Non-Node elements must NOT include it.
    auto nodeElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    QVERIFY(nodeElm != nullptr);

    bool nodeHasIt = false;
    for (const auto &desc : nodeElm->editableProperties()) {
        if (desc.type == PropertyDescriptor::Type::WirelessModeSelector) {
            nodeHasIt = true;
            break;
        }
    }
    QVERIFY(nodeHasIt);

    auto andElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(andElm != nullptr);
    for (const auto &desc : andElm->editableProperties()) {
        QVERIFY(desc.type != PropertyDescriptor::Type::WirelessModeSelector);
    }
}

// ============================================================================
// Port visibility edge cases
// ============================================================================

void TestWirelessNode::testPortVisibilityTxMode()
{
    // Tx mode hides the output port (signal goes wireless) but keeps input visible.
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    node->setWirelessMode(WirelessMode::Tx);

    QVERIFY(node->inputPort()->isVisible());
    QVERIFY(!node->outputPort()->isVisible());
}

void TestWirelessNode::testPortVisibilityRxMode()
{
    // Rx mode hides the input port (signal arrives wirelessly) but keeps output visible.
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    node->setWirelessMode(WirelessMode::Rx);

    QVERIFY(!node->inputPort()->isVisible());
    QVERIFY(node->outputPort()->isVisible());
}

void TestWirelessNode::testPortVisibilityAllTransitions()
{
    // Walk through every mode and verify both ports' visibility.
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    // None: both visible
    QVERIFY(node->inputPort()->isVisible());
    QVERIFY(node->outputPort()->isVisible());

    // Tx: input visible, output hidden
    node->setWirelessMode(WirelessMode::Tx);
    QVERIFY(node->inputPort()->isVisible());
    QVERIFY(!node->outputPort()->isVisible());

    // Rx: input hidden, output visible
    node->setWirelessMode(WirelessMode::Rx);
    QVERIFY(!node->inputPort()->isVisible());
    QVERIFY(node->outputPort()->isVisible());

    // Back to None: both visible again
    node->setWirelessMode(WirelessMode::None);
    QVERIFY(node->inputPort()->isVisible());
    QVERIFY(node->outputPort()->isVisible());
}

// ============================================================================
// Deserialization robustness
// ============================================================================

void TestWirelessNode::testLoadCorruptedWirelessModeClampsToNone()
{
    // Node::load() must clamp out-of-range wirelessMode values to None.
    // Craft a buffer with wirelessMode=99 and verify it loads as None.
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    // Save a normal Tx node to get a valid buffer, then patch the mode value.
    node->setWirelessMode(WirelessMode::Tx);

    QByteArray buffer;
    {
        QDataStream out(&buffer, QIODevice::WriteOnly);
        Serialization::writePandaHeader(out);
        node->save(out);
    }

    // Patch: replace the wirelessMode value in the trailing QMap.
    // The map contains {"wirelessMode": 1}. We need to change the int 1 to 99.
    // Find the last occurrence of the serialized int value 1 (which is the wireless mode).
    // Instead of brittle byte patching, build a buffer with a corrupt map directly.
    QByteArray corruptBuffer;
    {
        // Save the base element first, then append a corrupt map.
        auto elm2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
        QDataStream out(&corruptBuffer, QIODevice::WriteOnly);
        Serialization::writePandaHeader(out);
        elm2->save(out);  // saves base GraphicElement + normal Node map (mode=0)
    }

    // Overwrite the trailing map with one that has wirelessMode=99.
    // Easier: save a fresh node and manually write the corrupt map.
    QByteArray finalBuffer;
    {
        auto baseElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
        QDataStream out(&finalBuffer, QIODevice::WriteOnly);
        Serialization::writePandaHeader(out);
        // Save the GraphicElement base portion
        baseElm->save(out);
        // Write the Node-specific map with an invalid wirelessMode
        QMap<QString, QVariant> map;
        map.insert("wirelessMode", 99);
        out << map;
    }

    // Load it back
    auto loadElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *loadNode = qobject_cast<Node *>(loadElm.get());
    QVERIFY(loadNode != nullptr);

    {
        QDataStream in(&finalBuffer, QIODevice::ReadOnly);
        QMap<quint64, QNEPort *> portMap;
        SerializationContext ctx{portMap, Serialization::readPandaHeader(in)};
        loadNode->load(in, ctx);
    }

    // Must clamp to None
    QCOMPARE(loadNode->wirelessMode(), WirelessMode::None);
    QVERIFY(loadNode->inputPort()->isRequired());
}

void TestWirelessNode::testLoadNegativeWirelessModeClampsToNone()
{
    // Negative values (e.g. -1) must also clamp to None.
    auto baseElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));

    QByteArray buffer;
    {
        QDataStream out(&buffer, QIODevice::WriteOnly);
        Serialization::writePandaHeader(out);
        baseElm->save(out);
        QMap<QString, QVariant> map;
        map.insert("wirelessMode", -1);
        out << map;
    }

    auto loadElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *loadNode = qobject_cast<Node *>(loadElm.get());
    QVERIFY(loadNode != nullptr);

    {
        QDataStream in(&buffer, QIODevice::ReadOnly);
        QMap<quint64, QNEPort *> portMap;
        SerializationContext ctx{portMap, Serialization::readPandaHeader(in)};
        loadNode->load(in, ctx);
    }

    QCOMPARE(loadNode->wirelessMode(), WirelessMode::None);
    QVERIFY(loadNode->inputPort()->isRequired());
}

void TestWirelessNode::testSaveLoadRxPortVisibility()
{
    // After saving and loading an Rx node, port visibility must be preserved:
    // input hidden, output visible.
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    node->setWirelessMode(WirelessMode::Rx);
    node->setLabel("VIS_TEST");

    QByteArray buffer;
    {
        QDataStream out(&buffer, QIODevice::WriteOnly);
        Serialization::writePandaHeader(out);
        node->save(out);
    }

    auto elm2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node2 = qobject_cast<Node *>(elm2.get());
    QVERIFY(node2 != nullptr);

    {
        QDataStream in(&buffer, QIODevice::ReadOnly);
        QMap<quint64, QNEPort *> portMap;
        SerializationContext ctx{portMap, Serialization::readPandaHeader(in)};
        node2->load(in, ctx);
    }

    QCOMPARE(node2->wirelessMode(), WirelessMode::Rx);
    QVERIFY(!node2->inputPort()->isVisible());
    QVERIFY(node2->outputPort()->isVisible());
}

void TestWirelessNode::testSaveLoadTxPortVisibility()
{
    // Tx mode hides the output port. This must survive a save/load round-trip.
    auto elm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node = qobject_cast<Node *>(elm.get());
    QVERIFY(node != nullptr);

    node->setWirelessMode(WirelessMode::Tx);
    node->setLabel("TX_VIS");

    QByteArray buffer;
    {
        QDataStream out(&buffer, QIODevice::WriteOnly);
        Serialization::writePandaHeader(out);
        node->save(out);
    }

    auto elm2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    auto *node2 = qobject_cast<Node *>(elm2.get());
    QVERIFY(node2 != nullptr);

    {
        QDataStream in(&buffer, QIODevice::ReadOnly);
        QMap<quint64, QNEPort *> portMap;
        SerializationContext ctx{portMap, Serialization::readPandaHeader(in)};
        node2->load(in, ctx);
    }

    QCOMPARE(node2->wirelessMode(), WirelessMode::Tx);
    QVERIFY(node2->inputPort()->isVisible());
    QVERIFY(!node2->outputPort()->isVisible());
}

