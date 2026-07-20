// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// Backs QuickUpdateController::checkForUpdates() (App/QuickShell/Chrome/QuickUpdateController.cpp)
// -- the QDialog + QCheckBox + custom "Download"/"Close" QDialogButtonBox App/UI/UpdateController.cpp
// used to show. Opened via Main.qml's Connections on AppController.updateController.updateAvailable.
Dialog {
    id: root
    title: qsTr("Update Available")
    modal: true
    anchors.centerIn: parent
    width: 420

    readonly property QuickUpdateController controller: AppController.updateController

    onOpened: skipCheckBox.checked = false

    contentItem: ColumnLayout {
        spacing: 12
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            textFormat: Text.RichText
            text: root.controller.hasDirectDownload
                ? qsTr("<b>wiRedPanda %1 is available.</b><br><br>" +
                       "You are currently running version %2.<br>" +
                       "Click <b>Download</b> to save the new version to your computer.")
                      .arg(root.controller.latestVersion, root.controller.currentVersion)
                : qsTr("<b>wiRedPanda %1 is available.</b><br><br>" +
                       "You are currently running version %2.<br>" +
                       "Visit the release page to download the new version.")
                      .arg(root.controller.latestVersion, root.controller.currentVersion)
        }
        CheckBox {
            id: skipCheckBox
            text: qsTr("Don't notify me about this version again")
        }
    }

    footer: DialogButtonBox {
        Button {
            text: root.controller.hasDirectDownload ? qsTr("Download") : qsTr("Visit Release Page")
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
        }
        Button {
            text: qsTr("Close")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }

    onAccepted: root.controller.respondToUpdate(true, skipCheckBox.checked)
    onRejected: root.controller.respondToUpdate(false, skipCheckBox.checked)
}
