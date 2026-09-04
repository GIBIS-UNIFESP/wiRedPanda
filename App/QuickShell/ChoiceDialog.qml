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
        let flags = 0
        for (const b of requestedButtons) {
            flags |= buttonOrder[b]
        }
        return flags
    }

    // Named (not an inline onButtonClicked lambda) so QuickDialogProvider's dialogOpened()
    // testability hook can invoke it directly with a raw button value, simulating a click
    // without needing real synthetic mouse/window input. Also calls accept() explicitly -- a
    // *real* click closes the dialog via MessageDialog's own native/platform click handling
    // before this handler even runs, so accept() here is normally a harmless no-op; the
    // synthetic path (this function invoked directly, bypassing that native mechanism entirely)
    // needs it to actually unblock QuickDialogProvider::choice()'s execModal(), which waits on
    // accepted()/rejected() -- confirmed the hard way: without this, a direct
    // handleButtonClicked() call left it waiting forever. Which of accept()/reject() fires
    // doesn't matter to choice()'s own caller (it reads resultIndex, not accepted state) -- only
    // that execModal()'s loop, listening for either, gets one.
    function handleButtonClicked(button) {
        resultIndex = buttonOrder.indexOf(button)
        root.accept()
    }

    onButtonClicked: (button, role) => handleButtonClicked(button)
}
