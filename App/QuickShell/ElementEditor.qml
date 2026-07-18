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
// Not built in this pass (real, separately-scoped follow-ups, not oversights): Appearance
// (custom pixmap file dialog + per-state tile grid), AudioBox (file dialog), TruthTable (the
// editor grid dialog), and embedded-IC blob rename -- each needs its own QML dialog surface.
Item {
    id: root
    implicitWidth: 240

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
                visible: !root.editor.hasSelection
                text: qsTr("No element selected")
                wrapMode: Text.WordWrap
                opacity: 0.6
            }

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
                    textFromValue: (v) => (v / 10).toFixed(root.editor.frequencyDecimals) + qsTr(" Hz")
                    valueFromText: (t) => Math.round(parseFloat(t) * 10)
                    onValueModified: root.editor.frequency = value / 10
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                visible: root.editor.delayVisible
                Label { text: qsTr("Delay:") }
                Slider {
                    Layout.fillWidth: true
                    from: -4; to: 4; stepSize: 1
                    value: root.editor.delaySteps
                    onMoved: root.editor.delaySteps = Math.round(value)
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
        }
    }
}
