// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestElementContextMenu.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/IC.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Workspace.h"
#include "App/UI/SelectionCapabilities.h"
#include "Tests/Common/TestUtils.h"

void TestElementContextMenu::testRotateRightAction()
{
    // ElementContextMenu::exec()'s "Rotate right"/"Rotate left" actions dispatch
    // RotateCommand(elements, +90.0, scene) / RotateCommand(elements, -90.0, scene)
    // respectively (App/UI/ElementContextMenu.cpp) — exec() itself can't be driven from a
    // headless unit test (it blocks on a real QMenu::exec() popup), so pin the sign
    // convention those two menu entries rely on directly through RotateCommand.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{andGate}, scene));
    const int initialRotation = static_cast<int>(andGate->rotation());

    scene->receiveCommand(new RotateCommand({andGate}, 90.0, scene)); // "Rotate right"
    QCOMPARE(static_cast<int>(andGate->rotation()), (initialRotation + 90) % 360);

    scene->undoStack()->undo();
    scene->receiveCommand(new RotateCommand({andGate}, -90.0, scene)); // "Rotate left"
    // ElementOrientation::setRotation normalizes via std::fmod, which is sign-preserving
    // (unlike a positive-wrapping modulo), so a negative angle stays negative here.
    QCOMPARE(static_cast<int>(andGate->rotation()), initialRotation - 90);
}

void TestElementContextMenu::testMorphMenuAction()
{
    WorkSpace workspace;
    And andGate;
    workspace.scene()->addItem(&andGate);

    SelectionCapabilities caps = computeCapabilities({&andGate});
    QVERIFY(caps.canMorph);
}

void TestElementContextMenu::testICSubcircuitAction()
{
    // The "Edit sub-circuit"/"Embed sub-circuit"/"Extract to file" menu entries are gated by
    // caps.isEmbedded/caps.isFileBacked (App/UI/ElementContextMenu.cpp), computed from
    // IC::isEmbedded() (App/UI/SelectionCapabilities.cpp) — exercise the actual IC states
    // those flags are derived from, not just the generic "no selection" case.
    QVERIFY(!computeCapabilities({}).hasElements);

    WorkSpace workspace;
    And andGate;
    workspace.scene()->addItem(&andGate);
    const SelectionCapabilities nonIcCaps = computeCapabilities({&andGate});
    QVERIFY(!nonIcCaps.isEmbedded);
    QVERIFY(!nonIcCaps.isFileBacked);

    IC fileBackedIc;
    workspace.scene()->addItem(&fileBackedIc);
    const SelectionCapabilities fileBackedCaps = computeCapabilities({&fileBackedIc});
    QVERIFY(!fileBackedCaps.isEmbedded);
    QVERIFY(fileBackedCaps.isFileBacked);

    IC embeddedIc;
    embeddedIc.setBlobName("test_blob");
    workspace.scene()->addItem(&embeddedIc);
    const SelectionCapabilities embeddedCaps = computeCapabilities({&embeddedIc});
    QVERIFY(embeddedCaps.isEmbedded);
    QVERIFY(!embeddedCaps.isFileBacked);
}
