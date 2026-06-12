// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestElementEditor.h"

#include <QComboBox>
#include <QLineEdit>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/Workspace.h"
#include "App/UI/ElementEditor.h"
#include "Tests/Common/TestUtils.h"

void TestElementEditor::testCreation()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);

    // The editor builds its UI in the constructor — the core fields must exist
    QVERIFY(editor.findChild<QLineEdit *>("lineEditElementLabel") != nullptr);
    QVERIFY(editor.findChild<QComboBox *>("comboBoxColor") != nullptr);
    QVERIFY(editor.findChild<QComboBox *>("comboBoxInputSize") != nullptr);
    QVERIFY(editor.findChild<QComboBox *>("comboBoxOutputSize") != nullptr);
}

void TestElementEditor::testSetScene()
{
    WorkSpace workspace;
    // InputSwitch is labelable (hasLabel metadata); plain gates are not
    auto *input = new InputSwitch;
    input->setLabel("mySwitch");
    workspace.scene()->addItem(input);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    auto *labelEdit = editor.findChild<QLineEdit *>("lineEditElementLabel");
    QVERIFY(labelEdit != nullptr);

    // While connected, selecting an element populates the label field
    input->setSelected(true);
    QCOMPARE(labelEdit->text(), QString("mySwitch"));

    // Set to nullptr to exercise the disconnect path: scene selection changes
    // must no longer reach the editor
    editor.setScene(nullptr);
    labelEdit->setText("untouched");
    workspace.scene()->clearSelection();
    input->setSelected(true);
    QCOMPARE(labelEdit->text(), QString("untouched"));
}

void TestElementEditor::testRetranslateUi()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    editor.retranslateUi();

    // retranslateUi rebuilds the color combo with translated names — it must
    // come back populated
    auto *colorBox = editor.findChild<QComboBox *>("comboBoxColor");
    QVERIFY(colorBox != nullptr);
    QVERIFY(colorBox->count() > 0);
}

void TestElementEditor::testSetCurrentElementsEmpty()
{
    WorkSpace workspace;
    auto *input = new InputSwitch;
    input->setLabel("mySwitch");
    workspace.scene()->addItem(input);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    auto *labelEdit = editor.findChild<QLineEdit *>("lineEditElementLabel");
    QVERIFY(labelEdit != nullptr);

    input->setSelected(true);
    QCOMPARE(labelEdit->text(), QString("mySwitch"));

    // Empty selection hides the editor and clears the label field
    workspace.scene()->clearSelection();
    QCOMPARE(labelEdit->text(), QString());
    QVERIFY(editor.isHidden());
}

void TestElementEditor::testSetCurrentElementsGate()
{
    WorkSpace workspace;
    auto *andGate = new And;
    workspace.scene()->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    // Select the gate — the input-size combo reflects the gate's port range
    workspace.scene()->clearSelection();
    andGate->setSelected(true);

    auto *inputSizeBox = editor.findChild<QComboBox *>("comboBoxInputSize");
    QVERIFY(inputSizeBox != nullptr);
    QVERIFY(inputSizeBox->isEnabled());
    QVERIFY(inputSizeBox->count() > 0);
    QCOMPARE(inputSizeBox->currentText().toInt(), andGate->inputSize());
}

void TestElementEditor::testSetCurrentElementsLed()
{
    WorkSpace workspace;
    auto *led = new Led;
    workspace.scene()->addItem(led);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    // Select LED — the color combo is active and reflects the LED's color
    workspace.scene()->clearSelection();
    led->setSelected(true);

    auto *colorBox = editor.findChild<QComboBox *>("comboBoxColor");
    QVERIFY(colorBox != nullptr);
    QVERIFY(colorBox->isEnabled());
    QVERIFY(colorBox->count() > 0);
    QCOMPARE(colorBox->currentData().toString(), led->color());
}

void TestElementEditor::testFillColorComboBox()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    auto *colorBox = editor.findChild<QComboBox *>("comboBoxColor");
    QVERIFY(colorBox != nullptr);

    // fillColorComboBox rebuilds the dropdown — repeated calls keep a stable,
    // non-empty entry list (no duplicate accumulation)
    editor.fillColorComboBox();
    const int count = colorBox->count();
    QVERIFY(count > 0);

    editor.fillColorComboBox();
    QCOMPARE(colorBox->count(), count);
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
