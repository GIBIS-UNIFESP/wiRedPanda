// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/Runners/QuickRunnerUtils.h"

#include "Tests/QuickShell/TestCanvasCommands.h"
#include "Tests/QuickShell/TestCanvasItemSmoke.h"
#include "Tests/QuickShell/TestQuickDolphinController.h"
#include "Tests/QuickShell/TestQuickElementHandler.h"
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
    });
}
