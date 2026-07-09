// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestElementEditor.h"

#include <algorithm>

#include <QComboBox>
#include <QLabel>
#include <QToolButton>

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

    // The constructor builds the full property-editor widget tree.
    QVERIFY(editor.findChild<QComboBox *>("comboBoxColor"));
    QVERIFY(editor.findChild<QComboBox *>("comboBoxInputSize"));
    QVERIFY(editor.findChild<QWidget *>("widgetAppearanceStates"));
}

void TestElementEditor::testSetScene()
{
    WorkSpace workspace;
    auto *led = new Led;
    workspace.scene()->addItem(led);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    // Connected: selecting an element shows the editor.
    workspace.scene()->clearSelection();
    led->setSelected(true);
    QVERIFY(!editor.isHidden());

    // Disconnected: a further scene selection change is ignored — clearing the selection
    // would hide the editor if it were still connected to the scene.
    editor.setScene(nullptr);
    workspace.scene()->clearSelection();
    QVERIFY(!editor.isHidden());
}

void TestElementEditor::testRetranslateUi()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    editor.retranslateUi();

    // retranslateUi() (re)applies the static widget labels.
    auto *labelColor = editor.findChild<QLabel *>("labelColor");
    QVERIFY(labelColor);
    QCOMPARE(labelColor->text(), QStringLiteral("Color:"));
}

void TestElementEditor::testSetCurrentElementsEmpty()
{
    WorkSpace workspace;
    auto *andGate = new And;
    workspace.scene()->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    // A non-empty selection shows the editor...
    andGate->setSelected(true);
    QVERIFY(!editor.isHidden());

    // ...and clearing the selection hides it (setCurrentElements({}) calls hide()).
    workspace.scene()->clearSelection();
    QVERIFY(editor.isHidden());
}

void TestElementEditor::testSetCurrentElementsGate()
{
    WorkSpace workspace;
    auto *andGate = new And;
    workspace.scene()->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    workspace.scene()->clearSelection();
    andGate->setSelected(true);

    // Selecting a gate shows the editor and enables the input-size selector (And is 2..8 inputs).
    QVERIFY(!editor.isHidden());
    auto *inputSizeCombo = editor.findChild<QComboBox *>("comboBoxInputSize");
    QVERIFY(inputSizeCombo);
    QVERIFY(inputSizeCombo->isEnabled());
}

void TestElementEditor::testSetCurrentElementsLed()
{
    WorkSpace workspace;
    auto *led = new Led;
    workspace.scene()->addItem(led);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    // Select LED — triggers selectionChanged, populating the appearance tile grid
    workspace.scene()->clearSelection();
    led->setSelected(true);

    auto *statesWidget = editor.findChild<QWidget *>("widgetAppearanceStates");
    QVERIFY(statesWidget);
    // The editor itself is never shown in this test, so isVisible() (which also
    // depends on ancestor/window visibility) would always be false; isHidden()
    // reflects only this widget's own explicit setVisible() flag.
    QVERIFY(!statesWidget->isHidden());

    const auto tiles = statesWidget->findChildren<QToolButton *>();
    QCOMPARE(tiles.size(), 2); // 1-input LED: Off/On

    const auto checkedCount = std::count_if(tiles.begin(), tiles.end(), [](QToolButton *tile) { return tile->isChecked(); });
    QCOMPARE(checkedCount, 1);
    QVERIFY(tiles.first()->isChecked()); // first tile selected by default
}

void TestElementEditor::testAppearanceStateTileSelection()
{
    WorkSpace workspace;
    auto *led = new Led;
    led->setInputSize(2);
    workspace.scene()->addItem(led);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    workspace.scene()->clearSelection();
    led->setSelected(true);

    auto *statesWidget = editor.findChild<QWidget *>("widgetAppearanceStates");
    QVERIFY(statesWidget);

    const auto tiles = statesWidget->findChildren<QToolButton *>();
    QCOMPARE(tiles.size(), 4); // 2-input LED: 4 states

    auto *secondTile = tiles.at(1);
    secondTile->click();

    QVERIFY(secondTile->isChecked());
    QVERIFY(!tiles.first()->isChecked());
    QCOMPARE(secondTile->property("appearanceStateIndex").toInt(), led->appearanceStates().at(1).first);
}

void TestElementEditor::testFillColorComboBox()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    editor.fillColorComboBox();

    // The dropdown holds the five LED colours; item data carries the untranslated English
    // name used internally by GraphicElement::setColor().
    auto *combo = editor.findChild<QComboBox *>("comboBoxColor");
    QVERIFY(combo);
    QCOMPARE(combo->count(), 5);
    QCOMPARE(combo->itemData(0).toString(), QStringLiteral("White"));
    QCOMPARE(combo->itemData(4).toString(), QStringLiteral("Purple"));
}

void TestElementEditor::testSelectionDoesNotPushPortSizeCommandB21()
{
    // Pre-fix the input/output combo-box rebuild in setCurrentElements ran
    // without QSignalBlocker, so the synthetic setCurrentIndex inside the
    // rebuild fired inputIndexChanged → ChangePortSizeCommand each time the
    // user selected an element. The QSignalBlocker added in B21 silences
    // the rebuild signal storm; a fresh selection must therefore leave the
    // undo stack untouched.
    WorkSpace workspace;
    auto *andGate = new And();
    workspace.scene()->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    // Cycle selection a few times to give the combo-box rebuild several
    // chances to misfire.
    for (int i = 0; i < 3; ++i) {
        workspace.scene()->clearSelection();
        andGate->setSelected(true);
    }

    QCOMPARE(undoStack->count(), countBefore);
}
