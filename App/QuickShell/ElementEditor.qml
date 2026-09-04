// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// Element property panel, backed by App/QuickShell/Chrome/QuickElementEditor
// (AppController.elementEditor). Every section below is shown/hidden by one of the editor's
// *Visible properties, computed from SelectionCapabilities exactly as
// ElementEditor::applyCapabilitiesToUi() does -- QML's declarative visible: bindings replace
// that method's imperative setSection() calls.
//
// Appearance (custom pixmap file dialog + per-state tile grid), AudioBox (file dialog),
// TruthTable (button opens TruthTableDialog.qml), and embedded-IC blob rename are also
// implemented here -- see their own sections below.
//
// The six GroupBox sections below (Identity/Ports/Timing/Sound/Interaction/Appearance) mirror
// ElementEditorUI.cpp's own addSection()-built QGroupBoxes exactly -- same titles, same field
// grouping, same "hide the whole section when none of its fields apply" behavior
// (ElementEditor::applyCapabilitiesToUi()'s groupBoxXxx->setVisible(...) calls) -- just a plain
// ColumnLayout inside each GroupBox instead of a literal QGridLayout port, matching this file's
// existing per-field-RowLayout style everywhere else.
Item {
    id: root
    implicitWidth: 240
    // Mirrors ElementEditor::setCurrentElements()'s hide()/show(): the whole panel disappears
    // when nothing is selected. A later redesign might instead keep it present with an
    // empty-state message, to avoid the left column's height jumping on every click into empty
    // canvas -- but for now this matches production's actual current behavior.
    visible: root.editor.hasSelection
    // Widgets' elementEditor has a Fixed vertical QSizePolicy: MainWindowUI.cpp's QGridLayout
    // just gives it its natural sizeHint below tabElements, rather than stretching it. A plain
    // Item has no implicit height of its own, so without this it would silently collapse to
    // zero height in Main.qml's left ColumnLayout (below ElementPalette) -- capped so one
    // element's property panel can never swallow the whole left column; taller content scrolls
    // within editorScrollView instead, which Widgets' own uncapped/unscrolled panel doesn't
    // need to.
    implicitHeight: Math.min(editorColumn.implicitHeight + 16, 360)

    readonly property QuickElementEditor editor: AppController.elementEditor

    // Tab/Shift+Tab element-cycling: mirrors ElementTabNavigator's event filter, installed (in
    // Widgets) on every editable control in this panel. Each control below wires its own
    // Keys.onTabPressed/onBacktabPressed to this one shared helper, passing itself (so it can
    // refocus after the cycle) and its own fieldKey (see QuickElementEditor::cycleSelection()'s
    // own doc comment for the fieldKey vocabulary) -- refocusing happens unconditionally,
    // matching ElementTabNavigator::eventFilter()'s own widget->setFocus() at the very end
    // regardless of whether cycling actually found a different element.
    function cycleField(control, forward, fieldKey) {
        root.editor.cycleSelection(forward, fieldKey);
        control.forceActiveFocus();
    }

    // Mirrors ElementEditor::renameAction()/changeTriggerAction() -- focuses + selects the
    // corresponding field. Triggered from Main.qml's element context menu's Rename/Change
    // Trigger items.
    Connections {
        target: root.editor
        function onFocusLabelRequested() { labelField.forceActiveFocus(); labelField.selectAll() }
        function onFocusTriggerRequested() { triggerField.forceActiveFocus(); triggerField.selectAll() }
        // Mirrors ElementEditor::contextMenu()'s onFrequencyFocus lambda exactly -- no
        // selectAll() there either (m_ui->doubleSpinBoxFrequency->setFocus() alone).
        function onFocusFrequencyRequested() { freqSpin.forceActiveFocus() }
    }

    ScrollView {
        id: editorScrollView
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            id: editorColumn
            width: root.width
            spacing: 8

            Label {
                Layout.fillWidth: true
                Layout.margins: 6
                text: root.editor.elementTypeTitle
                font.bold: true
            }

            // --- Identity: Label, Color ---
            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                title: qsTr("Identity")
                visible: root.editor.labelVisible || root.editor.colorVisible

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.editor.labelVisible
                        Label { text: qsTr("Label:") }
                        TextField {
                            id: labelField
                            Layout.fillWidth: true
                            text: root.editor.label
                            onTextEdited: root.editor.label = text
                            Keys.onTabPressed: (event) => { root.cycleField(labelField, true, "label"); event.accepted = true }
                            Keys.onBacktabPressed: (event) => { root.cycleField(labelField, false, "label"); event.accepted = true }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.editor.colorVisible
                        Label { text: qsTr("Color:") }
                        ComboBox {
                            id: colorField
                            Layout.fillWidth: true
                            model: root.editor.colorOptions
                            textRole: "translatedName"
                            valueRole: "name"
                            currentIndex: indexOfValue(root.editor.color)
                            onActivated: root.editor.color = currentValue
                            Keys.onTabPressed: (event) => { root.cycleField(colorField, true, "color"); event.accepted = true }
                            Keys.onBacktabPressed: (event) => { root.cycleField(colorField, false, "color"); event.accepted = true }
                        }
                    }
                }
            }

            // --- Ports: Inputs, Outputs, Value, Locked ---
            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                title: qsTr("Ports")
                visible: root.editor.inputSizeVisible || root.editor.outputSizeVisible
                         || root.editor.outputValueVisible || root.editor.lockedVisible

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.editor.inputSizeVisible
                        Label { text: qsTr("Inputs:") }
                        ComboBox {
                            id: inputSizeField
                            Layout.fillWidth: true
                            model: root.editor.inputSizeOptions
                            textRole: "label"
                            valueRole: "value"
                            currentIndex: indexOfValue(root.editor.inputSize)
                            onActivated: root.editor.inputSize = currentValue
                            Keys.onTabPressed: (event) => { root.cycleField(inputSizeField, true, "inputSize"); event.accepted = true }
                            Keys.onBacktabPressed: (event) => { root.cycleField(inputSizeField, false, "inputSize"); event.accepted = true }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.editor.outputSizeVisible
                        Label { text: qsTr("Outputs:") }
                        ComboBox {
                            id: outputSizeField
                            Layout.fillWidth: true
                            model: root.editor.outputSizeOptions
                            textRole: "label"
                            valueRole: "value"
                            currentIndex: indexOfValue(root.editor.outputSize)
                            onActivated: root.editor.outputSize = currentValue
                            Keys.onTabPressed: (event) => { root.cycleField(outputSizeField, true, "outputSize"); event.accepted = true }
                            Keys.onBacktabPressed: (event) => { root.cycleField(outputSizeField, false, "outputSize"); event.accepted = true }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.editor.outputValueVisible
                        Label {
                            text: qsTr("Value:")
                            // Mirrors ElementEditorUI.cpp's valueTip, set on both the label and
                            // the control it describes.
                            HoverHandler { id: valueLabelHover }
                            ToolTip.text: qsTr("The value this input element outputs.")
                            ToolTip.visible: valueLabelHover.hovered
                        }
                        ComboBox {
                            id: outputValueField
                            Layout.fillWidth: true
                            model: root.editor.outputValueOptions
                            textRole: "label"
                            valueRole: "value"
                            currentIndex: indexOfValue(root.editor.outputValue)
                            onActivated: root.editor.outputValue = currentValue
                            // Mirrors ElementEditorUI.cpp's valueTip.
                            hoverEnabled: true
                            ToolTip.text: qsTr("The value this input element outputs.")
                            ToolTip.visible: hovered
                            Keys.onTabPressed: (event) => { root.cycleField(outputValueField, true, "outputValue"); event.accepted = true }
                            Keys.onBacktabPressed: (event) => { root.cycleField(outputValueField, false, "outputValue"); event.accepted = true }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.editor.lockedVisible
                        Label {
                            text: qsTr("Locked:")
                            // Mirrors ElementEditorUI.cpp's lockedTip, set on both the label and
                            // the control it describes.
                            HoverHandler { id: lockedLabelHover }
                            ToolTip.text: qsTr("Prevent this element from being toggled by clicking it during simulation.")
                            ToolTip.visible: lockedLabelHover.hovered
                        }
                        CheckBox {
                            id: lockedField
                            tristate: true
                            checkState: root.editor.lockedState
                            onToggled: root.editor.lockedState = checkState
                            // Mirrors ElementEditorUI.cpp's lockedTip.
                            hoverEnabled: true
                            ToolTip.text: qsTr("Prevent this element from being toggled by clicking it during simulation.")
                            ToolTip.visible: hovered
                            Keys.onTabPressed: (event) => { root.cycleField(lockedField, true, "locked"); event.accepted = true }
                            Keys.onBacktabPressed: (event) => { root.cycleField(lockedField, false, "locked"); event.accepted = true }
                        }
                    }
                }
            }

            // --- Timing: Frequency, Delay ---
            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                title: qsTr("Timing")
                visible: root.editor.frequencyVisible || root.editor.delayVisible

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    RowLayout {
                        Layout.fillWidth: true
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
                            Keys.onTabPressed: (event) => { root.cycleField(freqSpin, true, "frequency"); event.accepted = true }
                            Keys.onBacktabPressed: (event) => { root.cycleField(freqSpin, false, "frequency"); event.accepted = true }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.editor.delayVisible
                        Label {
                            text: qsTr("Delay:")
                            // Mirrors ElementEditorUI.cpp's delayTip, set on both the label and
                            // the control it describes.
                            HoverHandler { id: delayLabelHover }
                            ToolTip.text: qsTr("Phase offset from the driving clock, as a fraction of its period.")
                            ToolTip.visible: delayLabelHover.hovered
                        }
                        // LabeledSlider port (App/UI/LabeledSlider.h): a fraction-of-clock-period
                        // label under each tick, so the exact value is visible without a
                        // tooltip. delaySteps is eighths of a clock period, -4..4 -- a fixed,
                        // closed set, so the value/label mapping is inlined here rather than
                        // round-tripping through C++ for a table this small.
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Slider {
                                id: delayField
                                Layout.fillWidth: true
                                from: -4; to: 4; stepSize: 1
                                value: root.editor.delaySteps
                                onMoved: root.editor.delaySteps = Math.round(value)
                                // Mirrors ElementEditorUI.cpp's delayTip.
                                hoverEnabled: true
                                ToolTip.text: qsTr("Phase offset from the driving clock, as a fraction of its period.")
                                ToolTip.visible: hovered
                                Keys.onTabPressed: (event) => { root.cycleField(delayField, true, "delay"); event.accepted = true }
                                Keys.onBacktabPressed: (event) => { root.cycleField(delayField, false, "delay"); event.accepted = true }
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
                }
            }

            // --- Sound: Tone (Buzzer), Sound file (AudioBox), Volume ---
            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                title: qsTr("Sound")
                visible: root.editor.audioVisible || root.editor.audioBoxVisible || root.editor.volumeVisible

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.editor.audioVisible
                        Label { text: qsTr("Sound:") }
                        ComboBox {
                            id: audioField
                            Layout.fillWidth: true
                            model: root.editor.audioOptions
                            currentIndex: find(root.editor.audio)
                            onActivated: root.editor.audio = currentText
                            Keys.onTabPressed: (event) => { root.cycleField(audioField, true, "audio"); event.accepted = true }
                            Keys.onBacktabPressed: (event) => { root.cycleField(audioField, false, "audio"); event.accepted = true }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
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
                        visible: root.editor.volumeVisible
                        Label { text: qsTr("Volume:") }
                        Slider {
                            id: volumeField
                            Layout.fillWidth: true
                            from: 0; to: 100
                            // Mirrors ElementEditorUI.cpp's sliderVolume->setSingleStep(5)
                            // (tick-mark rendering itself is left as a stock QtQuick.Controls
                            // Slider -- there's no built-in tick-mark visual to enable the way
                            // QSlider::setTickPosition() is, only a custom-drawn background, not
                            // attempted here for one cosmetic detail).
                            stepSize: 5
                            value: root.editor.volume
                            onMoved: root.editor.volume = Math.round(value)
                            Keys.onTabPressed: (event) => { root.cycleField(volumeField, true, "volume"); event.accepted = true }
                            Keys.onBacktabPressed: (event) => { root.cycleField(volumeField, false, "volume"); event.accepted = true }
                        }
                    }
                }
            }

            // --- Interaction: Trigger, (shared row) Truth Table button / Wireless mode ---
            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                title: qsTr("Interaction")
                visible: root.editor.triggerVisible || root.editor.truthTableVisible || root.editor.wirelessModeVisible

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.editor.triggerVisible
                        Label {
                            text: qsTr("Trigger:")
                            // Mirrors ElementEditorUI.cpp's triggerTip, set on both the label and
                            // the control it describes.
                            HoverHandler { id: triggerLabelHover }
                            ToolTip.text: qsTr("Keyboard key that toggles this element during simulation.")
                            ToolTip.visible: triggerLabelHover.hovered
                        }
                        TextField {
                            id: triggerField
                            Layout.fillWidth: true
                            text: root.editor.trigger
                            maximumLength: 1
                            onTextEdited: root.editor.trigger = text
                            // Mirrors ElementEditorUI.cpp's triggerTip.
                            hoverEnabled: true
                            ToolTip.text: qsTr("Keyboard key that toggles this element during simulation.")
                            ToolTip.visible: hovered
                            Keys.onTabPressed: (event) => { root.cycleField(triggerField, true, "trigger"); event.accepted = true }
                            Keys.onBacktabPressed: (event) => { root.cycleField(triggerField, false, "trigger"); event.accepted = true }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.editor.wirelessModeVisible
                        Label {
                            text: qsTr("Wireless:")
                            // Mirrors ElementEditorUI.cpp's wirelessTip, set on both the label and
                            // the control it describes.
                            HoverHandler { id: wirelessLabelHover }
                            ToolTip.text: qsTr("Nodes with the same label connect wirelessly — Transmit sends its input, Receive outputs it.")
                            ToolTip.visible: wirelessLabelHover.hovered
                        }
                        ComboBox {
                            id: wirelessModeField
                            Layout.fillWidth: true
                            model: root.editor.wirelessModeOptions
                            currentIndex: root.editor.wirelessMode
                            onActivated: root.editor.wirelessMode = currentIndex
                            // Mirrors ElementEditorUI.cpp's wirelessTip.
                            hoverEnabled: true
                            ToolTip.text: qsTr("Nodes with the same label connect wirelessly — Transmit sends its input, Receive outputs it.")
                            ToolTip.visible: hovered
                            Keys.onTabPressed: (event) => { root.cycleField(wirelessModeField, true, "wirelessMode"); event.accepted = true }
                            Keys.onBacktabPressed: (event) => { root.cycleField(wirelessModeField, false, "wirelessMode"); event.accepted = true }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.editor.truthTableVisible
                        Button {
                            text: qsTr("Edit Truth Table...")
                            onClicked: root.editor.openTruthTable()
                        }
                    }
                }
            }

            // --- Appearance: state tile grid, (shared row) Change/Default buttons / IC Name ---
            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                title: qsTr("Appearance")
                visible: root.editor.appearanceVisible || root.editor.blobNameVisible

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    ColumnLayout {
                        Layout.fillWidth: true
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

                        // Mirrors ElementEditorUI.cpp's labelAppearanceState ("Appearance
                        // for:"), toggled visible alongside the tile grid it captions.
                        Label {
                            text: qsTr("Appearance for:")
                            visible: root.editor.appearanceStates.length > 1
                        }

                        // Multi-state tile grid (Led/Display7/14/16/Clock/InputButton/
                        // InputSwitch) -- empty for single-state elements, per
                        // QuickElementEditor::refresh()'s own gate.
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
                                    // ComponentBehavior: Bound" for this delegate -- same
                                    // precedent as Main.qml's colorOptions/morphCandidates
                                    // Repeaters, which reference the global singleton directly
                                    // rather than a local alias property.
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

            // Embed/extract for a selected IC. Mirrors ElementEditor's "Embed"/"Extract to
            // file..." buttons; dispatches straight
            // to AppController since QuickICController has no per-editor-instance state to own.
            // No Widgets section to nest under -- these buttons don't exist in
            // ElementEditorUI.cpp at all (Widgets only reaches them via the context menu).
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 6
                Layout.rightMargin: 6
                Layout.bottomMargin: 6
                visible: root.editor.embedIcVisible || root.editor.extractIcVisible

                Button {
                    text: qsTr("Embed")
                    visible: root.editor.embedIcVisible
                    onClicked: AppController.embedSelectedIC()
                }
                Button {
                    text: qsTr("Extract to file...")
                    visible: root.editor.extractIcVisible
                    onClicked: AppController.extractSelectedIC()
                }
            }
        }
    }
}
