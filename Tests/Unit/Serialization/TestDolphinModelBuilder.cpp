// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestDolphinModelBuilder.h"

#include "App/BeWavedDolphin/DolphinModelBuilder.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestDolphinModelBuilder::testCollectSortsByLabel()
{
    auto ws = std::make_unique<WorkSpace>();
    auto *scene = ws->scene();

    auto *swB = new InputSwitch();
    swB->setLabel("B");
    auto *swA = new InputSwitch();
    swA->setLabel("A");
    auto *ledB = new Led();
    ledB->setLabel("outB");
    auto *ledA = new Led();
    ledA->setLabel("outA");

    // Insertion order deliberately reversed from label order.
    scene->addItem(swB);
    scene->addItem(swA);
    scene->addItem(ledB);
    scene->addItem(ledA);

    const auto result = DolphinModelBuilder::collect(scene);

    QCOMPARE(result.inputs.size(), 2);
    QCOMPARE(result.inputs.at(0)->label(), QStringLiteral("A"));
    QCOMPARE(result.inputs.at(1)->label(), QStringLiteral("B"));

    QCOMPARE(result.outputs.size(), 2);
    QCOMPARE(result.outputs.at(0)->label(), QStringLiteral("outA"));
    QCOMPARE(result.outputs.at(1)->label(), QStringLiteral("outB"));

    QCOMPARE(result.inputLabels, QStringList({"A", "B"}));
    QCOMPARE(result.outputLabels, QStringList({"outA", "outB"}));
    QCOMPARE(result.inputPorts, 2);
}

void TestDolphinModelBuilder::testCollectBuildsIndexedLabelsForMultiPortElements()
{
    auto ws = std::make_unique<WorkSpace>();
    auto *scene = ws->scene();

    auto *rotary = new InputRotary();
    rotary->setLabel("Rot");
    auto *display = new Display7();
    display->setLabel("Disp");

    scene->addItem(rotary);
    scene->addItem(display);

    QVERIFY2(rotary->outputSize() > 1, "InputRotary should have more than one output port by default");
    QVERIFY2(display->inputSize() > 1, "Display7 should have more than one input port by default");

    const auto result = DolphinModelBuilder::collect(scene);

    QCOMPARE(result.inputLabels.size(), rotary->outputSize());
    for (int i = 0; i < rotary->outputSize(); ++i) {
        QCOMPARE(result.inputLabels.at(i), QStringLiteral("Rot[%1]").arg(i));
    }

    QCOMPARE(result.outputLabels.size(), display->inputSize());
    for (int i = 0; i < display->inputSize(); ++i) {
        QCOMPARE(result.outputLabels.at(i), QStringLiteral("Disp[%1]").arg(i));
    }
}

void TestDolphinModelBuilder::testCollectRejectsSceneWithNeitherInputsNorOutputs()
{
    auto ws = std::make_unique<WorkSpace>();
    auto *scene = ws->scene();
    scene->addItem(new And());

    QVERIFY_THROWS(std::exception, DolphinModelBuilder::collect(scene));
}

void TestDolphinModelBuilder::testCollectRejectsSceneWithNoInputs()
{
    auto ws = std::make_unique<WorkSpace>();
    auto *scene = ws->scene();
    scene->addItem(new Led());

    QVERIFY_THROWS(std::exception, DolphinModelBuilder::collect(scene));
}

void TestDolphinModelBuilder::testCollectRejectsSceneWithNoOutputs()
{
    auto ws = std::make_unique<WorkSpace>();
    auto *scene = ws->scene();
    scene->addItem(new InputSwitch());

    QVERIFY_THROWS(std::exception, DolphinModelBuilder::collect(scene));
}
