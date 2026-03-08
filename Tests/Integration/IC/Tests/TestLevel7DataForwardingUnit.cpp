// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7DataForwardingUnit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::setMultiBitInput;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

struct DataForwardingUnitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> dataAInputs;
    QVector<InputSwitch *> dataBInputs;
    QVector<InputSwitch *> dataCInputs;
    QVector<InputSwitch *> dataDInputs;
    QVector<InputSwitch *> selectInputs;
    QVector<Led *> outputLeds;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level7_data_forwarding_unit.panda");
        builder.add(ic);

        for (int i = 0; i < 8; i++) {
            auto *a = new InputSwitch(); builder.add(a); dataAInputs.append(a);
            auto *b = new InputSwitch(); builder.add(b); dataBInputs.append(b);
            auto *c = new InputSwitch(); builder.add(c); dataCInputs.append(c);
            auto *d = new InputSwitch(); builder.add(d); dataDInputs.append(d);
            auto *led = new Led(); builder.add(led); outputLeds.append(led);
        }

        for (int i = 0; i < 2; i++) {
            auto *sw = new InputSwitch(); builder.add(sw); selectInputs.append(sw);
        }

        for (int i = 0; i < 8; i++) {
            builder.connect(dataAInputs[i], 0, ic, QString("DataA[%1]").arg(i));
            builder.connect(dataBInputs[i], 0, ic, QString("DataB[%1]").arg(i));
            builder.connect(dataCInputs[i], 0, ic, QString("DataC[%1]").arg(i));
            builder.connect(dataDInputs[i], 0, ic, QString("DataD[%1]").arg(i));
            builder.connect(ic, QString("ForwardedData[%1]").arg(i), outputLeds[i], 0);
        }

        for (int i = 0; i < 2; i++) {
            builder.connect(selectInputs[i], 0, ic, QString("Select[%1]").arg(i));
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readOutput()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(outputLeds.begin(), outputLeds.end()), 0);
    }
};

static std::unique_ptr<DataForwardingUnitFixture> s_level7DataFwd;

void TestLevel7DataForwardingUnit::initTestCase()
{
    s_level7DataFwd = std::make_unique<DataForwardingUnitFixture>();
    QVERIFY(s_level7DataFwd->build());
}

void TestLevel7DataForwardingUnit::cleanupTestCase()
{
    s_level7DataFwd.reset();
}

void TestLevel7DataForwardingUnit::cleanup()
{
    if (s_level7DataFwd && s_level7DataFwd->sim) {
        s_level7DataFwd->sim->restart();
        s_level7DataFwd->sim->update();
    }
}

void TestLevel7DataForwardingUnit::testDataForwardingUnit_data()
{
    QTest::addColumn<int>("dataA");
    QTest::addColumn<int>("dataB");
    QTest::addColumn<int>("dataC");
    QTest::addColumn<int>("dataD");
    QTest::addColumn<int>("select");
    QTest::addColumn<int>("expectedOutput");

    QTest::newRow("forward DataA (select=00)") << 0x11 << 0x22 << 0x33 << 0x44 << 0 << 0x11;
    QTest::newRow("forward DataB (select=01)") << 0x11 << 0x22 << 0x33 << 0x44 << 1 << 0x22;
    QTest::newRow("forward DataC (select=10)") << 0x11 << 0x22 << 0x33 << 0x44 << 2 << 0x33;
    QTest::newRow("forward DataD (select=11)") << 0x11 << 0x22 << 0x33 << 0x44 << 3 << 0x44;
    QTest::newRow("forward 0xFF from DataA")   << 0xFF << 0x00 << 0x00 << 0x00 << 0 << 0xFF;
    QTest::newRow("forward 0xAA from DataD")   << 0x00 << 0x00 << 0x00 << 0xAA << 3 << 0xAA;
}

void TestLevel7DataForwardingUnit::testDataForwardingUnit()
{
    QFETCH(int, dataA);
    QFETCH(int, dataB);
    QFETCH(int, dataC);
    QFETCH(int, dataD);
    QFETCH(int, select);
    QFETCH(int, expectedOutput);

    auto &f = *s_level7DataFwd;

    setMultiBitInput(f.dataAInputs, dataA);
    setMultiBitInput(f.dataBInputs, dataB);
    setMultiBitInput(f.dataCInputs, dataC);
    setMultiBitInput(f.dataDInputs, dataD);
    setMultiBitInput(f.selectInputs, select);
    f.sim->update();

    QCOMPARE(f.readOutput(), expectedOutput);
}

void TestLevel7DataForwardingUnit::testDataForwardingUnitStructure()
{
    auto &f = *s_level7DataFwd;

    QVERIFY(f.ic != nullptr);

    QCOMPARE(f.ic->inputSize(), 34);
    QCOMPARE(f.ic->outputSize(), 8);
}

