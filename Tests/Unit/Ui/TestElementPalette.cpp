// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestElementPalette.h"

#include "Tests/Common/TestUtils.h"

void TestElementPalette::testPaletteSearch()
{
    QSKIP("ElementPalette requires MainWindowUi; palette search is covered by "
          "TestMainWindowGui (lineEditSearch focus/filter tests)");
}

void TestElementPalette::testPaletteRebuild()
{
    QSKIP("ElementPalette requires MainWindowUi; palette population is covered by "
          "TestMainWindowGui::testDragElementFromPalette");
}
