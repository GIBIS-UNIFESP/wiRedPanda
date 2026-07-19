// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// Phase 4 sub-step 5: element property panel, backed by App/QuickShell/Chrome/
// QuickElementEditor (AppController.elementEditor). Every section below is shown/hidden by
// one of the editor's *Visible properties, computed from SelectionCapabilities exactly as
// ElementEditor::applyCapabilitiesToUi() does -- QML's declarative visible: bindings replace
// that method's imperative setSection() calls.
//
// Appearance (custom pixmap file dialog + per-state tile grid), AudioBox (file dialog),
// TruthTable (button opens TruthTableDialog.qml), and embedded-IC blob rename were all ported
// in a later follow-up pass (Phase 4 named-deferrals) -- see their own sections below.
Item {
    id: root
    implicitWidth: 240
    // Mirrors ElementEditor::setCurrentElements()'s hide()/show(): the whole panel disappears
    // when nothing is selected (Main.qml's leftPane ColumnLayout collapses this row back to
    // just the palette above), not a placeholder message shown in its place.
    visible: root.editor.hasSelection

    readonly property QuickElementEditor editor: AppController.elementEditor

    // Mirrors ElementEditor::renameAction()/changeTriggerAction() -- focuses + selects the
    // corresponding field. Triggered from Main.qml's element context menu's Rename/Change
    // Trigger items.
    Connections {
        target: root.editor
        function onFocusLabelRequested() { labelField.forceActiveFocus(); labelField.selectAll(); }
        function onFocusTriggerRequested() { triggerField.forceActiveFocus(); triggerField.selectAll(); }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: root.width
            spacing: 8

            Label {
                Layout.fillWidth: true
                Layout.margins: 6
                visible: root.editor.hasSelection
                text: root.editor.elementTypeTitle
                font.bold: true
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                visible: root.editor.labelVisible
                Label { text: qsTr("Label:") }
                TextField {
                    id: labelField
                    Layout.fillWidth: true
                    text: root.editor.label
                    onTextEdited: root.editor.label = text
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                visible: root.editor.colorVisible
                Label { text: qsTr("Color:") }
                ComboBox {
                    Layout.fillWidth: true
                    model: root.editor.colorOptions
                    textRole: "translatedName"
                    valueRole: "name"
                    currentIndex: indexOfValue(root.editor.color)
                    onActivated: root.editor.color = currentValue
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                visible: root.editor.audioVisible
                Label { text: qsTr("Sound:") }
                ComboBox {
                    Layout.fillWidth: true
                    model: root.editor.audioOptions
                    currentIndex: find(root.editor.audio)
                    onActivated: root.editor.audio = currentText
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                visible: root.editor.audioBoxVisible
                Button {
                    text: qsTr("Choose Audio...")
                    onClicked: root.editor.pickAudioBoxFile()
                }
                Label {
                    Layout.fillWidth: true
                    elide: Text.ElideMiddle
                    text: root.editor.audioBoxFileName
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                visible: root.editor.volumeVisible
                Label { text: qsTr("Volume:") }
                Slider {
                    Layout.fillWidth: true
                    from: 0; to: 100; stepSize: 1
                    value: root.editor.volume
                    onMoved: root.editor.volume = Math.round(value)
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                visible: root.editor.frequencyVisible
                Label { text: qsTr("Frequency:") }
                SpinBox {
                    id: freqSpin
                    Layout.fillWidth: true
                    from: Math.round(root.editor.frequencyMin * 10)
                    to: Math.round(root.editor.frequencyMax * 10)
                    stepSize: Math.max(1, Math.round(root.editor.frequencyStep * 10))
                    value: Math.round(root.editor.frequency * 10)
                    // Arrow functions cannot carry type annotations in QML -- plain
                    // function expressions can, so these use that form instead.
                    textFromValue: function (v: int): string { return (v / 10).toFixed(root.editor.frequencyDecimals) + qsTr(" Hz"); }
                    valueFromText: function (t: string): int { return Math.round(parseFloat(t) * 10); }
                    onValueModified: root.editor.frequency = value / 10
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                visible: root.editor.delayVisible
                Label { text: qsTr("Delay:") }
                // LabeledSlider port (App/UI/LabeledSlider.h): a fraction-of-clock-period label
                // under each tick, so the exact value is visible without a tooltip. delaySteps
                // is eighths of a clock period, -4..4 -- a fixed, closed set, so the value/label
                // mapping is inlined here rather than round-tripping through C++ for a table
                // this small.
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Slider {
                        Layout.fillWidth: true
                        from: -4; to: 4; stepSize: 1
                        value: root.editor.delaySteps
                        onMoved: root.editor.delaySteps = Math.round(value)
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 0
                        Repeater {
                            model: [-4, -3, -2, -1, 0, 1, 2, 3, 4]
                            delegate: Label {
                                required property int modelData
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                font.pixelSize: 9
                                text: ["-1/2", "-3/8", "-1/4", "-1/8", "0", "1/8", "1/4", "3/8", "1/2"][modelData + 4]
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                visible: root.editor.inputSizeVisible
                Label { text: qsTr("Inputs:") }
                ComboBox {
                    Layout.fillWidth: true
                    model: root.editor.inputSizeOptions
                    textRole: "label"
                    valueRole: "value"
                    currentIndex: indexOfValue(root.editor.inputSize)
                    onActivated: root.editor.inputSize = currentValue
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                visible: root.editor.outputSizeVisible
                Label { text: qsTr("Outputs:") }
                ComboBox {
                    Layout.fillWidth: true
                    model: root.editor.outputSizeOptions
                    textRole: "label"
                    valueRole: "value"
                    currentIndex: indexOfValue(root.editor.outputSize)
                    onActivated: root.editor.outputSize = currentValue
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                visible: root.editor.outputValueVisible
                Label { text: qsTr("Value:") }
                ComboBox {
                    Layout.fillWidth: true
                    model: root.editor.outputValueOptions
                    textRole: "label"
                    valueRole: "value"
                    currentIndex: indexOfValue(root.editor.outputValue)
                    onActivated: root.editor.outputValue = currentValue
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                visible: root.editor.lockedVisible
                Label { text: qsTr("Locked:") }
                CheckBox {
                    tristate: true
                    checkState: root.editor.lockedState
                    onToggled: root.editor.lockedState = checkState
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                visible: root.editor.triggerVisible
                Label { text: qsTr("Trigger:") }
                TextField {
                    id: triggerField
                    Layout.fillWidth: true
                    text: root.editor.trigger
                    maximumLength: 1
                    onTextEdited: root.editor.trigger = text
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                Layout.bottomMargin: 6
                visible: root.editor.wirelessModeVisible
                Label { text: qsTr("Wireless:") }
                ComboBox {
                    Layout.fillWidth: true
                    model: root.editor.wirelessModeOptions
                    currentIndex: root.editor.wirelessMode
                    onActivated: root.editor.wirelessMode = currentIndex
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                visible: root.editor.truthTableVisible
                Button {
                    text: qsTr("Edit Truth Table...")
                    onClicked: root.editor.openTruthTable()
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                spacing: 4
                visible: root.editor.appearanceVisible

                RowLayout {
                    Layout.fillWidth: true
                    Button {
                        text: qsTr("Change Appearance...")
                        onClicked: root.editor.changeAppearance()
                    }
                    Button {
                        text: qsTr("Reset")
                        onClicked: root.editor.resetAppearance()
                    }
                }

                // Multi-state tile grid (Led/Display7/14/16/Clock/InputButton/InputSwitch) --
                // empty for single-state elements, per QuickElementEditor::refresh()'s own gate.
                Flow {
                    Layout.fillWidth: true
                    visible: root.editor.appearanceStates.length > 1
                    spacing: 2

                    Repeater {
                        model: root.editor.appearanceStates
                        delegate: ToolButton {
                            required property appearanceStateOption modelData
                            checkable: true
                            // AppController (not root.editor) avoids needing "pragma
                            // ComponentBehavior: Bound" for this delegate -- same precedent as
                            // Main.qml's colorOptions/morphCandidates Repeaters, which reference
                            // the global singleton directly rather than a local alias property.
                            checked: AppController.elementEditor.appearanceStateIndex === modelData.index
                            icon.source: modelData.previewImageUrl
                            ToolTip.text: modelData.label
                            ToolTip.visible: hovered
                            onClicked: AppController.elementEditor.appearanceStateIndex = modelData.index
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                Layout.bottomMargin: 6
                visible: root.editor.blobNameVisible
                Label { text: qsTr("IC name:") }
                TextField {
                    Layout.fillWidth: true
                    text: root.editor.blobName
                    onEditingFinished: root.editor.commitBlobRename(text)
                }
            }
        }
    }
}
