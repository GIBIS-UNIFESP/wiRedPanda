// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick.Dialogs

// Backs QuickDialogProvider's FileDialogProvider methods (App/QuickShell/Chrome/
// QuickDialogProvider.cpp) -- the QML equivalent of RealFileDialogProvider's QFileDialog,
// which needs a real QApplication that wiredpanda_quick (QGuiApplication only) never has.
FileDialog {
    id: root

    property bool accepted_: false

    onAccepted: root.accepted_ = true
    onRejected: root.accepted_ = false
}
