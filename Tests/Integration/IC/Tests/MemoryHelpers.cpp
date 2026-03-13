// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/MemoryHelpers.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Element/GraphicElements/Not.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::getInputStatus;
using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;

void buildRegisterFile4x4(RegisterFile4x4Output &out)
{
    CircuitBuilder builder(out.workspace.scene());

    IC *regFileIC = CPUTestUtils::loadBuildingBlockIC("level5_register_file_4x4.panda");

    // Add all elements to scene
    builder.add(&out.writeAddr[0], &out.writeAddr[1],
                &out.writeData[0], &out.writeData[1],
                &out.writeData[2], &out.writeData[3],
                &out.writeEnable, &out.clock,
                &out.readAddr[0], &out.readAddr[1],
                regFileIC);

    for (int i = 0; i < 4; ++i) {
        builder.add(&out.readData[i]);
    }

    // Connect inputs to IC using semantic port labels
    for (int i = 0; i < 2; i++) {
        builder.connect(&out.writeAddr[i], 0, regFileIC, QString("Write_Addr[%1]").arg(i));
        builder.connect(&out.readAddr[i], 0, regFileIC, QString("Read_Addr1[%1]").arg(i));
    }
    for (int i = 0; i < 4; i++) {
        builder.connect(&out.writeData[i], 0, regFileIC, QString("Data_In[%1]").arg(i));
    }
    builder.connect(&out.writeEnable, 0, regFileIC, "Write_Enable");
    builder.connect(&out.clock, 0, regFileIC, "Clock");

    // Connect IC outputs to LEDs
    for (int i = 0; i < 4; i++) {
        builder.connect(regFileIC, QString("Read_Data1[%1]").arg(i), &out.readData[i], 0);
    }

    out.sim = builder.initSimulation();
}

void buildRegisterFile8x8(RegisterFile8x8Output &out)
{
    CircuitBuilder builder(out.workspace.scene());

    IC *regFileIC = CPUTestUtils::loadBuildingBlockIC("level5_register_file_8x8.panda");

    // Add all elements to scene
    builder.add(&out.writeAddr[0], &out.writeAddr[1], &out.writeAddr[2],
                &out.writeData[0], &out.writeData[1], &out.writeData[2],
                &out.writeData[3], &out.writeData[4], &out.writeData[5],
                &out.writeData[6], &out.writeData[7],
                &out.writeEnable, &out.clock,
                &out.readAddr[0], &out.readAddr[1], &out.readAddr[2],
                regFileIC);

    for (int i = 0; i < 8; ++i) {
        builder.add(&out.readData[i]);
    }

    // Connect inputs to IC using semantic port labels
    for (int i = 0; i < 3; i++) {
        builder.connect(&out.writeAddr[i], 0, regFileIC, QString("Write_Addr[%1]").arg(i));
        builder.connect(&out.readAddr[i], 0, regFileIC, QString("Read_Addr1[%1]").arg(i));
    }
    for (int i = 0; i < 8; i++) {
        builder.connect(&out.writeData[i], 0, regFileIC, QString("Data_In[%1]").arg(i));
    }
    builder.connect(&out.writeEnable, 0, regFileIC, "Write_Enable");
    builder.connect(&out.clock, 0, regFileIC, "Clock");

    // Connect IC outputs to LEDs
    for (int i = 0; i < 8; i++) {
        builder.connect(regFileIC, QString("Read_Data1[%1]").arg(i), &out.readData[i], 0);
    }

    out.sim = builder.initSimulation();
}

