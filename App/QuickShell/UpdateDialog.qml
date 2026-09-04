// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// Feature-gap decision 8 (post-hoc, 2026-09-01): the "update available"/download-progress
// dialog -- production's App/UI/UpdateController.cpp builds two separate QDialog/
// QProgressDialog instances instead; this is a single Dialog with three content states
// (available/downloading/finished) instead, simpler than mirroring that split 1:1. Opened via
// Main.qml's Connections on AppController.updateController.updateAvailable, mirroring
// shortcutsDialog/truthTableDialog's own open()-by-id precedent.
Dialog {
    id: root
    // Mirrors UpdateController.cpp's per-state dialog/progress-dialog titles exactly --
    // "Update Available" for the prompt, "Downloading Update" while in progress, and
    // "Download Failed"/"Download Complete" (QMessageBox::warning/information's own titles)
    // once finished.
    title: root.uiState === 1
        ? qsTr("Downloading Update")
        : (root.uiState === 2 ? (root.downloadSucceeded ? qsTr("Download Complete") : qsTr("Download Failed")) : qsTr("Update Available"))
    // Mirrors UpdateController.cpp's own lifecycle exactly: the window-modal "available" QDialog
    // is torn down before the deliberately non-modal QProgressDialog appears (its own comment:
    // "the main window stays interactive... Check for Updates can be triggered again
    // mid-download") -- but the finished state (uiState 2) is shown via QMessageBox::warning()/
    // information(), which is application-modal by default, unlike the progress dialog. Only
    // uiState 1 (downloading) is non-modal here. Untested directly: TestQuickDialogProvider.cpp
    // does load and drive real .qml dialogs (ChoiceDialog.qml/TextPromptDialog.qml/
    // QuickFileDialog.qml) in a real QQuickWindow, but those three are AppController-free --
    // this dialog is not (AppController.updateController/appVersion() below), and there is no
    // established pattern in this project for standing up the full AppController singleton
    // just to render one dialog's bindings in a test. QuickUpdateController's own signals/state
    // are covered separately in TestQuickUpdateController.cpp.
    modal: root.uiState !== 1
    anchors.centerIn: parent
    width: 420

    // --- Data from updateAvailable(), captured once and held for the dialog's lifetime ---
    property string latestVersion: ""
    property url downloadUrl: ""
    property url releaseUrl: ""
    readonly property bool hasDirectDownload: downloadUrl.toString().length > 0

    // 0 = update-available prompt, 1 = download in progress, 2 = download finished (success or
    // failure) -- reset to 0 every time the dialog is (re)opened for a fresh check.
    property int uiState: 0
    property int downloadPercent: 0
    property string finishedMessage: ""
    property bool downloadSucceeded: false

    onOpened: {
        uiState = 0
        downloadPercent = 0
        skipCheckBox.checked = false
    }

    readonly property QuickUpdateController controller: AppController.updateController

    Connections {
        target: root.controller
        function onDownloadProgress(percent) { root.downloadPercent = percent }
        function onDownloadFinished(success, message) {
            root.downloadSucceeded = success
            root.finishedMessage = message
            root.uiState = 2
        }
    }

    // The skip-checkbox is recorded regardless of which button closed the "available" prompt --
    // mirrors showUpdateDialog()'s own "recorded regardless of accept/reject" handling.
    function applySkipCheckbox() {
        if (skipCheckBox.checked && latestVersion.length > 0) {
            controller.skipVersion(latestVersion)
        }
    }

    // Cancel (downloading state) is the only button routed through the DialogButtonBox's own
    // role mapping -- every other button below closes/transitions explicitly in its own
    // onClicked instead, so "Download" can move to the progress state without the box's
    // AcceptRole auto-closing the dialog out from under it.
    onRejected: controller.cancelDownload()

    contentItem: ColumnLayout {
        spacing: 8

        Label {
            visible: root.uiState === 0
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            textFormat: Text.RichText
            text: root.hasDirectDownload
                ? qsTr("<b>wiRedPanda %1 is available.</b><br><br>You are currently running version %2.<br>Click <b>Download</b> to save the new version to your computer.").arg(root.latestVersion, AppController.appVersion())
                : qsTr("<b>wiRedPanda %1 is available.</b><br><br>You are currently running version %2.<br>Visit the release page to download the new version.").arg(root.latestVersion, AppController.appVersion())
        }
        CheckBox {
            id: skipCheckBox
            visible: root.uiState === 0
            text: qsTr("Don't notify me about this version again")
        }

        Label {
            visible: root.uiState === 1
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Downloading wiRedPanda %1…").arg(root.latestVersion)
        }
        ProgressBar {
            visible: root.uiState === 1
            Layout.fillWidth: true
            from: 0
            to: 100
            value: root.downloadPercent
        }

        Label {
            visible: root.uiState === 2
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: root.finishedMessage
            // Mirrors QMessageBox::warning()/information()'s distinct standard icon in spirit --
            // no bundled SVG maps cleanly to "success"/"error" (same reasoning the Help menu's
            // "Shortcuts and Tips"/"About Qt" items already document for a missing platform
            // icon: skip rather than invent a misleading substitute asset), so a failure gets a
            // plain warning-colored outline instead, which needs no new asset.
            Rectangle {
                visible: !root.downloadSucceeded
                anchors.fill: parent
                anchors.margins: -4
                z: -1
                radius: 4
                color: "transparent"
                border.color: "#cc3333"
                border.width: 1
            }
        }
    }

    footer: DialogButtonBox {
        Button {
            visible: root.uiState === 0
            text: qsTr("Close")
            onClicked: { root.applySkipCheckbox(); root.close() }
        }
        Button {
            visible: root.uiState === 0 && root.hasDirectDownload
            text: qsTr("Download")
            onClicked: {
                root.applySkipCheckbox()
                root.uiState = 1
                root.controller.downloadUpdate(root.latestVersion, root.downloadUrl)
            }
        }
        Button {
            visible: root.uiState === 0 && !root.hasDirectDownload
            text: qsTr("Visit Release Page")
            onClicked: {
                root.applySkipCheckbox()
                root.controller.openReleasePage(root.releaseUrl)
                root.close()
            }
        }
        Button {
            visible: root.uiState === 1
            text: qsTr("Cancel")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
        Button {
            visible: root.uiState === 2
            text: qsTr("Close")
            onClicked: root.close()
        }
    }
}
