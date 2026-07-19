// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/Runners/QuickRunnerUtils.h"

#include "Tests/QuickShell/TestCanvasItemSmoke.h"

int main(int argc, char **argv)
{
    return runQuickTestSuite(argc, argv, {
        {"TestCanvasItemSmoke", []() -> QObject * { return new TestCanvasItemSmoke; }},
    });
}
