// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick.Dialogs

// Backs QuickDialogProvider::choice() (App/QuickShell/Chrome/QuickDialogProvider.cpp).
// requestedButtons is a list of DialogButton (App/QuickShell/Chrome/DialogProvider.h) integer
// values; buttonOrder's array position must match DialogButton's declaration order exactly.
// resultIndex is set to the DialogButton value of whichever button was clicked, or left at -1
// if the dialog was dismissed without a button click.
MessageDialog {
    id: root

    property var requestedButtons: []
    property int resultIndex: -1

    readonly property var buttonOrder: [
        MessageDialog.Ok, MessageDialog.Yes, MessageDialog.No,
        MessageDialog.Cancel, MessageDialog.YesToAll, MessageDialog.NoToAll
    ]

    buttons: {
        let flags = 0;
        for (const b of requestedButtons) {
            flags |= buttonOrder[b];
        }
        return flags;
    }

    // Named (not an inline onButtonClicked lambda) so QuickDialogProvider's dialogOpened()
    // testability hook can invoke it directly with a raw button value, simulating a click
    // without needing real synthetic mouse/window input.
    function handleButtonClicked(button: int) {
        resultIndex = buttonOrder.indexOf(button);
    }

    onButtonClicked: (button, role) => handleButtonClicked(button)
}
