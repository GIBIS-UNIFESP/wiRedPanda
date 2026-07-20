// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// Backs QuickUpdateController::downloadUpdate() -- the QProgressDialog App/UI/UpdateController.cpp
// used to show. Opened via Main.qml's Connections on AppController.updateController.downloadStarted,
// closed on downloadFinished (the success/failure notice itself is a separate, plain
// Dialogs::provider()->choice() call made directly from C++, not through this dialog).
Dialog {
    id: root
    title: qsTr("Downloading Update")
    modal: true
    closePolicy: Popup.NoAutoClose
    anchors.centerIn: parent
    width: 340

    readonly property QuickUpdateController controller: AppController.updateController

    onOpened: bar.value = 0

    footer: DialogButtonBox {
        Button {
            text: qsTr("Cancel")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }
    onRejected: root.controller.cancelDownload()

    contentItem: ColumnLayout {
        spacing: 8
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Downloading wiRedPanda %1…").arg(root.controller.latestVersion)
        }
        ProgressBar {
            id: bar
            Layout.fillWidth: true
            from: 0
            to: 100
            value: 0
        }
    }

    Connections {
        target: root.controller
        function onDownloadProgress(percent) { bar.value = percent }
        function onDownloadFinished() { root.close() }
    }
}
