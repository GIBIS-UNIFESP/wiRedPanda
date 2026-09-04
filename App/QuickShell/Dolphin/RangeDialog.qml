// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Generic slider+spinbox integer prompt, backing both ClockDialog and LengthDialog's ports
// (App/UI/ClockDialogUI.cpp / LengthDialogUI.cpp): both are the exact same UI shape --
// a title label, a Slider and SpinBox bound to the same value, min/max end labels, and OK/
// Cancel -- differing only in title/label text and range, which DolphinWindow.qml supplies as
// two separate instances of this one component (same consolidation TextPromptDialog.qml/
// ChoiceDialog.qml already established for other single-value prompts in this rewrite).
Dialog {
    id: root

    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel

    property alias dialogLabel: label.text
    property alias from: slider.from
    property alias to: slider.to
    property alias value: slider.value
    // Mirrors ClockDialogUI.cpp's setSingleStep(2) (so the clock period's 50% duty cycle divides
    // cleanly) vs. LengthDialogUI.cpp's setSingleStep(1) -- the two Widgets dialogs this component
    // ports differ here, so it can't be a shared hardcoded constant.
    property alias step: slider.stepSize

    onOpened: slider.forceActiveFocus()

    contentItem: ColumnLayout {
        Label {
            id: label
            Layout.fillWidth: true
        }
        RowLayout {
            Layout.fillWidth: true
            Label { text: root.from }
            Slider {
                id: slider
                Layout.fillWidth: true
                from: 2
                to: 2048
                stepSize: 1 // overridden per-instance via the `step` alias above
            }
            Label { text: root.to }
        }
        SpinBox {
            id: spinBox
            Layout.alignment: Qt.AlignHCenter
            from: slider.from
            to: slider.to
            stepSize: slider.stepSize
            value: slider.value
            onValueModified: slider.value = value
        }
    }
}
