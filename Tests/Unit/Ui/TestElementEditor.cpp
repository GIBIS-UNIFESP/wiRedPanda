// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestElementEditor.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/Workspace.h"
#include "App/UI/ElementEditor.h"
#include "Tests/Common/TestUtils.h"

void TestElementEditor::testCreation()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);
    QVERIFY(true);
}

void TestElementEditor::testSetScene()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    // Set to nullptr to exercise disconnect path
    editor.setScene(nullptr);
    QVERIFY(true);
}

void TestElementEditor::testRetranslateUi()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    editor.retranslateUi();
    QVERIFY(true);
}

void TestElementEditor::testSetCurrentElementsEmpty()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    // Empty selection — triggers selectionChanged with no elements
    workspace.scene()->clearSelection();
    QVERIFY(true);
}

void TestElementEditor::testSetCurrentElementsGate()
{
    WorkSpace workspace;
    auto *andGate = new And;
    workspace.scene()->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    // Select the gate — triggers selectionChanged with input-count UI
    workspace.scene()->clearSelection();
    andGate->setSelected(true);
    QVERIFY(true);
}

void TestElementEditor::testSetCurrentElementsLed()
{
    WorkSpace workspace;
    auto *led = new Led;
    workspace.scene()->addItem(led);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    // Select LED — triggers selectionChanged with color combo UI
    workspace.scene()->clearSelection();
    led->setSelected(true);
    QVERIFY(true);
}

void TestElementEditor::testFillColorComboBox()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    // fillColorComboBox rebuilds the color dropdown
    editor.fillColorComboBox();
    QVERIFY(true);
}

