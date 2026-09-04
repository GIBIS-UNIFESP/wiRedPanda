// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Backs QuickDialogProvider::textPrompt() (App/QuickShell/Chrome/QuickDialogProvider.cpp).
// accepted_ tracks whether Ok (vs. Cancel/dismiss) closed the dialog -- read back from C++
// alongside `value`, since Dialog's own result state doesn't expose that distinction as a
// plain property the way MessageDialog's button click does.
Dialog {
    id: root

    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel

    property alias promptLabel: label.text
    property alias value: field.text
    property bool accepted_: false

    onAccepted: root.accepted_ = true
    onRejected: root.accepted_ = false
    onOpened: {
        field.selectAll()
        field.forceActiveFocus()
    }

    contentItem: ColumnLayout {
        Label {
            id: label
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }
        TextField {
            id: field
            Layout.fillWidth: true
            selectByMouse: true
            Keys.onReturnPressed: root.accept()
        }
    }
}
