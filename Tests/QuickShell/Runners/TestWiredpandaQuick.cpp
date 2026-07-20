// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/Runners/QuickRunnerUtils.h"

#include "Tests/QuickShell/TestCanvasCommands.h"
#include "Tests/QuickShell/TestCanvasEmbeddedIC.h"
#include "Tests/QuickShell/TestCanvasInlineIC.h"
#include "Tests/QuickShell/TestCanvasItemInteraction.h"
#include "Tests/QuickShell/TestCanvasPortHover.h"
#include "Tests/QuickShell/TestCanvasItemSmoke.h"
#include "Tests/QuickShell/IC/TestLevel1DFlipFlop.h"
#include "Tests/QuickShell/TestFeedback.h"
#include "Tests/QuickShell/TestSimulation.h"
#include "Tests/QuickShell/TestQuickWorkSpace.h"
#include "Tests/QuickShell/TestConnection.h"
#include "Tests/QuickShell/TestConnectionValidity.h"
#include "Tests/QuickShell/TestPort.h"
#include "Tests/QuickShell/TestSceneConnections.h"
#include "Tests/QuickShell/TestSelectionCapabilities.h"
#include "Tests/QuickShell/TestQuickAppController.h"
#include "Tests/QuickShell/TestQuickCanvasZoom.h"
#include "Tests/QuickShell/TestQuickDolphinController.h"
#include "Tests/QuickShell/TestQuickElementEditor.h"
#include "Tests/QuickShell/TestQuickElementHandler.h"
#include "Tests/QuickShell/TestQuickElementPalette.h"
#include "Tests/QuickShell/TestQuickMinimap.h"
#include "Tests/QuickShell/TestQuickFileHandlerSecurity.h"
#include "Tests/QuickShell/TestQuickICHandlerSecurity.h"

int main(int argc, char **argv)
{
    return runQuickTestSuite(argc, argv, {
        {"TestCanvasItemSmoke", []() -> QObject * { return new TestCanvasItemSmoke; }},
        {"TestCanvasCommands", []() -> QObject * { return new TestCanvasCommands; }},
        {"TestQuickElementHandler", []() -> QObject * { return new TestQuickElementHandler; }},
        {"TestQuickFileHandlerSecurity", []() -> QObject * { return new TestQuickFileHandlerSecurity; }},
        {"TestQuickICHandlerSecurity", []() -> QObject * { return new TestQuickICHandlerSecurity; }},
        {"TestQuickDolphinController", []() -> QObject * { return new TestQuickDolphinController; }},
        {"TestQuickAppController", []() -> QObject * { return new TestQuickAppController; }},
        {"TestCanvasEmbeddedIC", []() -> QObject * { return new TestCanvasEmbeddedIC; }},
        {"TestCanvasInlineIC", []() -> QObject * { return new TestCanvasInlineIC; }},
        {"TestCanvasItemInteraction", []() -> QObject * { return new TestCanvasItemInteraction; }},
        {"TestCanvasPortHover", []() -> QObject * { return new TestCanvasPortHover; }},
        {"TestQuickElementEditor", []() -> QObject * { return new TestQuickElementEditor; }},
        {"TestQuickElementPalette", []() -> QObject * { return new TestQuickElementPalette; }},
        {"TestQuickCanvasZoom", []() -> QObject * { return new TestQuickCanvasZoom; }},
        {"TestQuickMinimap", []() -> QObject * { return new TestQuickMinimap; }},
        {"TestConnection", []() -> QObject * { return new TestConnection; }},
        {"TestConnectionValidity", []() -> QObject * { return new TestConnectionValidity; }},
        {"TestPort", []() -> QObject * { return new TestPort; }},
        {"TestSelectionCapabilities", []() -> QObject * { return new TestSelectionCapabilities; }},
        {"TestSceneConnections", []() -> QObject * { return new TestSceneConnections; }},
        {"TestFeedback", []() -> QObject * { return new TestFeedback; }},
        {"TestSimulation", []() -> QObject * { return new TestSimulation; }},
        {"TestQuickWorkSpace", []() -> QObject * { return new TestQuickWorkSpace; }},
        {"TestLevel1DFlipFlop", []() -> QObject * { return new TestLevel1DFlipFlop; }},
    });
}
