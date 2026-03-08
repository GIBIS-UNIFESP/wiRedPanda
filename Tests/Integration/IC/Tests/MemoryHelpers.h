// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Scene/Workspace.h"

class Simulation;

// ============================================================================
// Output fixtures — declare these as local variables in your test function.
//
// Ownership: WorkSpace is declared FIRST so it is destroyed LAST (C++ destroys
// struct members in reverse declaration order). Circuit elements declared after
// workspace are destroyed before it: each element's ~QGraphicsItem() calls
// scene->removeItem(), so the scene is empty by the time workspace (and its
// scene) is finally destroyed. This prevents double-free and resource leaks.
// ============================================================================

// Plain IC-based 4x4 Register File fixture
struct RegisterFile4x4Output {
    WorkSpace workspace;         // destroyed last: owns the scene
    InputSwitch writeAddr[2];    // destroyed before workspace; removes self from scene
    InputSwitch writeData[4];
    InputSwitch writeEnable;
    InputSwitch clock;
    InputSwitch readAddr[2];
    Led readData[4];
    Simulation *sim = nullptr;  // owned by scene, valid while workspace is alive
};

// Plain IC-based 8x8 Register File fixture
struct RegisterFile8x8Output {
    WorkSpace workspace;         // destroyed last: owns the scene
    InputSwitch writeAddr[3];
    InputSwitch writeData[8];
    InputSwitch writeEnable;
    InputSwitch clock;
    InputSwitch readAddr[3];
    Led readData[8];
    Simulation *sim = nullptr;
};

// Helper function declarations
void buildRegisterFile4x4(RegisterFile4x4Output &out);
void buildRegisterFile8x8(RegisterFile8x8Output &out);

