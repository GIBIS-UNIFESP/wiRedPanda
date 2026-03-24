// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestDialogs.h"

#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QSlider>
#include <QSpinBox>
#include <QTest>
#include <QTimer>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Scene/Workspace.h"
#include "App/UI/ClockDialog.h"
#include "App/UI/ElementEditor.h"
#include "App/UI/LengthDialog.h"
#include "Tests/Common/TestUtils.h"

// ============================================================================
// ClockDialog Tests (5 tests)
// ============================================================================

void TestDialogs::testClockDialogConstructor()
{
    // Test that constructor sets initial frequency value and syncs spinbox/slider
    ClockDialog dialog(100);

    auto *spinBox = dialog.findChild<QSpinBox *>("frequencySpinBox");
    QVERIFY(spinBox != nullptr);
    QCOMPARE(spinBox->value(), 100);

    auto *slider = dialog.findChild<QSlider *>("frequencySlider");
    QVERIFY(slider != nullptr);
    QCOMPARE(slider->value(), 100);
}

void TestDialogs::testClockDialogMinFrequency()
{
    // Test that minimum frequency (2 Hz) is accepted and stored correctly
    ClockDialog dialog(2);

    auto *spinBox = dialog.findChild<QSpinBox *>("frequencySpinBox");
    QCOMPARE(spinBox->minimum(), 2);
    QCOMPARE(spinBox->value(), 2);
}

void TestDialogs::testClockDialogMaxFrequency()
{
    // Test that maximum frequency (1024 Hz) is accepted and stored correctly
    ClockDialog dialog(1024);

    auto *spinBox = dialog.findChild<QSpinBox *>("frequencySpinBox");
    QCOMPARE(spinBox->maximum(), 1024);
    QCOMPARE(spinBox->value(), 1024);
}

void TestDialogs::testClockDialogRange()
{
    // Test various valid frequencies within range (2-1024 Hz)
    QVector<int> validFrequencies = {2, 10, 100, 256, 512, 1000, 1024};

    for (int freq : validFrequencies) {
        ClockDialog dialog(freq);
        auto *spinBox = dialog.findChild<QSpinBox *>("frequencySpinBox");
        QCOMPARE(spinBox->value(), freq);
    }

    // Test out-of-range clamping: value above max clamps to 1024
    ClockDialog dialogHigh(5000);
    auto *spinBox = dialogHigh.findChild<QSpinBox *>("frequencySpinBox");
    QCOMPARE(spinBox->value(), 1024);
}

void TestDialogs::testClockDialogFrequencyReturnValue()
{
    // Test cancel and accept return values for frequency()
    // Cancel path → -1
    ClockDialog dialog1(100);
    QTimer::singleShot(0, &dialog1, &QDialog::reject);
    QCOMPARE(dialog1.frequency(), -1);

    // Accept path → returns spinbox value
    ClockDialog dialog2(256);
    QTimer::singleShot(0, &dialog2, &QDialog::accept);
    QCOMPARE(dialog2.frequency(), 256);
}

// ============================================================================
// LengthDialog Tests (4 tests)
// ============================================================================

void TestDialogs::testLengthDialogConstructor()
{
    // Test that constructor sets initial length value and syncs spinbox/slider
    LengthDialog dialog(64);

    auto *spinBox = dialog.findChild<QSpinBox *>("lengthSpinBox");
    QVERIFY(spinBox != nullptr);
    QCOMPARE(spinBox->value(), 64);

    auto *slider = dialog.findChild<QSlider *>("lengthSlider");
    QVERIFY(slider != nullptr);
    QCOMPARE(slider->value(), 64);

    // Verify constructor actually overrides: use non-default value
    LengthDialog dialog2(100);
    QCOMPARE(dialog2.findChild<QSpinBox *>("lengthSpinBox")->value(), 100);
}

void TestDialogs::testLengthDialogMinLength()
{
    // Test that minimum length (2) is accepted and stored correctly
    LengthDialog dialog(2);

    auto *spinBox = dialog.findChild<QSpinBox *>("lengthSpinBox");
    QCOMPARE(spinBox->minimum(), 2);
    QCOMPARE(spinBox->value(), 2);
}

void TestDialogs::testLengthDialogMaxLength()
{
    // Test that maximum length (2048) is accepted and stored correctly
    LengthDialog dialog(2048);

    auto *spinBox = dialog.findChild<QSpinBox *>("lengthSpinBox");
    QCOMPARE(spinBox->maximum(), 2048);
    QCOMPARE(spinBox->value(), 2048);
}

void TestDialogs::testLengthDialogRange()
{
    // Test various valid lengths within range (2-2048)
    QVector<int> validLengths = {2, 10, 32, 64, 128, 512, 1024, 2048};

    for (int length : validLengths) {
        LengthDialog dialog(length);
        auto *spinBox = dialog.findChild<QSpinBox *>("lengthSpinBox");
        QCOMPARE(spinBox->value(), length);
    }

    // Test out-of-range clamping: value above max clamps to 2048
    LengthDialog dialogHigh(9999);
    auto *spinBox = dialogHigh.findChild<QSpinBox *>("lengthSpinBox");
    QCOMPARE(spinBox->value(), 2048);
}

// ============================================================================
// ElementEditor — Wireless Tx duplicate-label rejection (5 tests)
// ============================================================================

// Build a Tx Node, add it to the scene, and return it.
static Node *makeTxNode(Scene *scene, const QString &label)
{
    auto *node = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    node->setWirelessMode(WirelessMode::Tx);
    node->setLabel(label);
    scene->addItem(node);
    return node;
}

struct EditorFixture {
    WorkSpace workspace;
    ElementEditor editor;
    QLineEdit *labelEdit = nullptr;
    QComboBox *modeCombo = nullptr;

    explicit EditorFixture()
    {
        editor.setScene(workspace.scene());
        labelEdit = editor.findChild<QLineEdit *>("lineEditElementLabel");
        modeCombo = editor.findChild<QComboBox *>("comboBoxWirelessMode");
        QVERIFY(labelEdit);
        QVERIFY(modeCombo);
    }

    // Select an element: mark it selected and process events so the scene's
    // selectionChanged signal reaches the ElementEditor.
    void select(GraphicElement *elm)
    {
        elm->setSelected(true);
        QCoreApplication::processEvents();
    }

    // Set the label field without triggering apply(), then emit textChanged
    // to fire apply() exactly once.  Place dismissNextDialog() before this
    // call when a rejection dialog is expected.
    void changeLabel(const QString &text)
    {
        { QSignalBlocker b(labelEdit); labelEdit->setText(text); }
        emit labelEdit->textChanged(text);
    }

    // Change the wireless-mode combo.  currentIndexChanged is connected to
    // apply(), so this fires apply() exactly once.
    void changeMode(WirelessMode mode)
    {
        modeCombo->setCurrentIndex(static_cast<int>(mode));
    }

    // Schedule auto-close of the next modal dialog so tests don't block.
    static void dismissNextDialog()
    {
        QTimer::singleShot(0, [] {
            if (auto *w = QApplication::activeModalWidget()) {
                w->close();
            }
        });
    }
};

void TestDialogs::testElementEditorRejectsDuplicateTxLabel()
{
    // Renaming a Tx node to a label already used by another Tx node must be rejected.
    EditorFixture f;
    auto *nodeA = makeTxNode(f.workspace.scene(), "CLOCK");
    auto *nodeB = makeTxNode(f.workspace.scene(), "DATA");
    Q_UNUSED(nodeA)

    f.select(nodeB);
    EditorFixture::dismissNextDialog();
    f.changeLabel("CLOCK"); // collision — dialog shown and dismissed

    QCOMPARE(nodeB->label(), QStringLiteral("DATA")); // must be unchanged
}

void TestDialogs::testElementEditorAllowsUniqueTxLabel()
{
    // Renaming a Tx node to a label not used by any other Tx node must succeed.
    EditorFixture f;
    auto *nodeA = makeTxNode(f.workspace.scene(), "CLOCK");
    makeTxNode(f.workspace.scene(), "DATA");

    f.select(nodeA);
    f.changeLabel("BUS"); // unique — no collision

    QCOMPARE(nodeA->label(), QStringLiteral("BUS"));
}

void TestDialogs::testElementEditorRejectsModeChangeToDuplicateTx()
{
    // Switching a None node to Tx must be rejected when its label already
    // matches an existing Tx node's label.
    EditorFixture f;
    makeTxNode(f.workspace.scene(), "ADDR");

    auto *nodeB = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    nodeB->setWirelessMode(WirelessMode::None);
    nodeB->setLabel("ADDR");
    f.workspace.scene()->addItem(nodeB);

    f.select(nodeB);
    EditorFixture::dismissNextDialog();
    f.changeMode(WirelessMode::Tx); // would collide — rejected

    QCOMPARE(nodeB->wirelessMode(), WirelessMode::None);
}

void TestDialogs::testElementEditorAllowsSelfRelabelTx()
{
    // A Tx node must not collide with itself — re-applying the same label is allowed.
    EditorFixture f;
    auto *node = makeTxNode(f.workspace.scene(), "CLK");

    f.select(node);
    f.changeLabel("CLK"); // same label — must not show dialog

    QCOMPARE(node->label(), QStringLiteral("CLK"));
    QCOMPARE(node->wirelessMode(), WirelessMode::Tx);
}

void TestDialogs::testElementEditorAllowsRxWithDuplicateLabel()
{
    // An Rx node sharing a label with a Tx node is valid — the duplicate
    // check must only block Tx→Tx collisions.
    EditorFixture f;
    makeTxNode(f.workspace.scene(), "SDA");

    auto *rxNode = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    rxNode->setWirelessMode(WirelessMode::Rx);
    rxNode->setLabel("SDA_OLD");
    f.workspace.scene()->addItem(rxNode);

    f.select(rxNode);
    f.changeLabel("SDA"); // same label as a Tx, but this node is Rx — allowed

    QCOMPARE(rxNode->label(), QStringLiteral("SDA"));
}

