// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// Quick-side port of BewavedDolphin's QMainWindow shell (Phase 6b-6e of the Qt Quick rewrite).
// Mirrors BewavedDolphinUi::setupUi()'s menu/toolbar structure -- Edit's undo/clipboard/cell-edit
// actions (6c) and File's Load/Save/Export family (6d) are all real, backed by
// QuickDolphinController methods that own their own file dialogs (FileDialogs::provider()
// already works unmodified here, same precedent QuickExportController's export actions
// established). Help is omitted here, matching the main Quick app's own current gap (no
// About/AboutQt there either yet) rather than inventing dialogs Dolphin-only. Launched for real
// via Main.qml's static instance + AppController.openWaveform()/waveformOpenRequested (Phase 6e,
// see Main.qml's own comment on that instance) -- QuickAppController now implements DolphinHost,
// so the "link this file to the current circuit" save prompt and window-title file-name
// decoration are real too (onClosing below).
ApplicationWindow {
    id: dolphinWindow

    required property QuickDolphinController controller

    width: 800
    height: 500
    visible: true
    title: qsTr("beWavedDolphin Simulator") + (dolphinWindow.controller.currentFileName.length > 0 ? " [" + dolphinWindow.controller.currentFileName + "]" : "")

    onClosing: (close) => {
        if (!dolphinWindow.controller.checkSave()) {
            close.accepted = false;
            return;
        }
        AppController.notifyWaveformClosed();
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem { text: qsTr("Load"); onTriggered: dolphinWindow.controller.load() }
            MenuSeparator {}
            MenuItem { text: qsTr("Save"); onTriggered: dolphinWindow.controller.save() }
            MenuItem { text: qsTr("Save As..."); onTriggered: dolphinWindow.controller.saveAs() }
            MenuItem { text: qsTr("Export to PDF"); onTriggered: dolphinWindow.controller.exportToPdf() }
            MenuItem { text: qsTr("Export to PNG"); onTriggered: dolphinWindow.controller.exportToPng() }
            MenuSeparator {}
            MenuItem { text: qsTr("Exit"); onTriggered: dolphinWindow.close() }
        }

        Menu {
            title: qsTr("&Edit")
            MenuItem {
                text: qsTr("Undo") + (dolphinWindow.controller.undoText ? " (" + dolphinWindow.controller.undoText + ")" : "")
                enabled: dolphinWindow.controller.canUndo
                onTriggered: dolphinWindow.controller.undo()
            }
            MenuItem {
                text: qsTr("Redo") + (dolphinWindow.controller.redoText ? " (" + dolphinWindow.controller.redoText + ")" : "")
                enabled: dolphinWindow.controller.canRedo
                onTriggered: dolphinWindow.controller.redo()
            }
            MenuSeparator {}
            MenuItem { text: qsTr("Cut"); onTriggered: dolphinWindow.controller.cut() }
            MenuItem { text: qsTr("Copy"); onTriggered: dolphinWindow.controller.copy() }
            MenuItem { text: qsTr("Paste"); onTriggered: dolphinWindow.controller.paste() }
            MenuSeparator {}
            MenuItem { text: qsTr("Clear"); onTriggered: dolphinWindow.controller.clearInputs() }
            MenuItem { text: qsTr("Combinational"); onTriggered: dolphinWindow.controller.combinational() }
            MenuItem { text: qsTr("Set to 0"); onTriggered: dolphinWindow.controller.setSelectedTo0() }
            MenuItem { text: qsTr("Set to 1"); onTriggered: dolphinWindow.controller.setSelectedTo1() }
            MenuItem { text: qsTr("Invert"); onTriggered: dolphinWindow.controller.invertSelected() }
            MenuItem {
                text: qsTr("Set clock period")
                enabled: dolphinWindow.controller.hasSelection
                onTriggered: clockDialog.openForSelection()
            }
            MenuSeparator {}
            MenuItem { text: qsTr("AutoCrop"); onTriggered: dolphinWindow.controller.autoCrop() }
        }

        Menu {
            title: qsTr("&View")
            MenuItem { text: qsTr("Zoom In"); enabled: dolphinWindow.controller.canZoomIn; onTriggered: dolphinWindow.controller.zoomIn() }
            MenuItem { text: qsTr("Zoom Out"); enabled: dolphinWindow.controller.canZoomOut; onTriggered: dolphinWindow.controller.zoomOut() }
            MenuItem { text: qsTr("Reset Zoom"); onTriggered: dolphinWindow.controller.resetZoom() }
            MenuItem { text: qsTr("Fit to screen"); onTriggered: dolphinWindow.fitScreenToViewport() }
            MenuItem { text: qsTr("Set Length"); onTriggered: lengthDialog.openForCurrentLength() }
            MenuSeparator {}
            MenuItem {
                text: qsTr("Show Numbers")
                checkable: true
                checked: dolphinWindow.controller.numberMode
                onTriggered: dolphinWindow.controller.showNumbers()
            }
            MenuItem {
                text: qsTr("Show Waveforms")
                checkable: true
                checked: !dolphinWindow.controller.numberMode
                onTriggered: dolphinWindow.controller.showWaveforms()
            }
        }
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            spacing: 0

            ToolButton {
                icon.source: "qrc:/Interface/Dolphin/folder.svg"
                ToolTip.text: qsTr("Load")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.load()
            }
            ToolButton {
                icon.source: "qrc:/Interface/Dolphin/save.svg"
                ToolTip.text: qsTr("Save")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.save()
            }
            ToolButton {
                icon.source: "qrc:/Interface/Dolphin/pdf.svg"
                ToolTip.text: qsTr("Export to PDF")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.exportToPdf()
            }
            ToolButton {
                icon.source: "qrc:/Interface/Dolphin/png.svg"
                ToolTip.text: qsTr("Export to PNG")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.exportToPng()
            }

            ToolSeparator {}

            ToolButton {
                icon.source: "qrc:/Interface/Toolbar/undo.svg"
                enabled: dolphinWindow.controller.canUndo
                ToolTip.text: qsTr("Undo")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.undo()
            }
            ToolButton {
                icon.source: "qrc:/Interface/Toolbar/redo.svg"
                enabled: dolphinWindow.controller.canRedo
                ToolTip.text: qsTr("Redo")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.redo()
            }

            ToolSeparator {}

            ToolButton {
                icon.source: "qrc:/Interface/Dolphin/cut.svg"
                ToolTip.text: qsTr("Cut")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.cut()
            }
            ToolButton {
                icon.source: "qrc:/Interface/Toolbar/copy.svg"
                ToolTip.text: qsTr("Copy")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.copy()
            }
            ToolButton {
                icon.source: "qrc:/Interface/Dolphin/paste.svg"
                ToolTip.text: qsTr("Paste")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.paste()
            }

            ToolSeparator {}

            ToolButton {
                icon.source: "qrc:/Interface/Dolphin/reloadFile.svg"
                ToolTip.text: qsTr("Clear")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.clearInputs()
            }
            ToolButton {
                icon.source: "qrc:/Interface/Toolbar/wavyIcon.svg"
                ToolTip.text: qsTr("Combinational")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.combinational()
            }

            ToolSeparator {}

            ToolButton {
                icon.source: "qrc:/Components/Input/0.svg"
                ToolTip.text: qsTr("Set to 0")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.setSelectedTo0()
            }
            ToolButton {
                icon.source: "qrc:/Components/Input/1.svg"
                ToolTip.text: qsTr("Set to 1")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.setSelectedTo1()
            }
            ToolButton {
                icon.source: "qrc:/Components/Logic/not.svg"
                ToolTip.text: qsTr("Invert")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.invertSelected()
            }
            ToolButton {
                icon.source: "qrc:/Components/Input/clock1.svg"
                enabled: dolphinWindow.controller.hasSelection
                ToolTip.text: qsTr("Set clock period")
                ToolTip.visible: hovered
                onClicked: clockDialog.openForSelection()
            }
            ToolButton {
                icon.source: "qrc:/Interface/Dolphin/autoCrop.svg"
                ToolTip.text: qsTr("AutoCrop")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.autoCrop()
            }

            ToolSeparator {}

            ToolButton {
                icon.source: "qrc:/Interface/Dolphin/zoomIn.svg"
                enabled: dolphinWindow.controller.canZoomIn
                ToolTip.text: qsTr("Zoom In")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.zoomIn()
            }
            ToolButton {
                icon.source: "qrc:/Interface/Dolphin/zoomOut.svg"
                enabled: dolphinWindow.controller.canZoomOut
                ToolTip.text: qsTr("Zoom Out")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.zoomOut()
            }
            ToolButton {
                icon.source: "qrc:/Interface/Dolphin/zoomReset.svg"
                ToolTip.text: qsTr("Reset Zoom")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.controller.resetZoom()
            }
            ToolButton {
                icon.source: "qrc:/Interface/Dolphin/zoomRange.svg"
                ToolTip.text: qsTr("Fit to screen")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.fitScreenToViewport()
            }
            ToolButton {
                icon.source: "qrc:/Interface/Dolphin/range.svg"
                ToolTip.text: qsTr("Set Length")
                ToolTip.visible: hovered
                onClicked: lengthDialog.openForCurrentLength()
            }

            ToolSeparator {}

            Item { Layout.fillWidth: true }

            ToolButton {
                icon.source: "qrc:/Interface/Dolphin/exit.svg"
                ToolTip.text: qsTr("Exit")
                ToolTip.visible: hovered
                onClicked: dolphinWindow.close()
            }
        }
    }

    // Mirrors BewavedDolphin's own m_ui->statusbar->showMessage() calls after save/load --
    // same auto-clearing transient-message pattern as Main.qml's own footer (AppController.
    // statusMessage), just backed by QuickDolphinController.statusMessage instead.
    footer: Label {
        visible: dolphinWindow.controller.statusMessage.length > 0
        text: dolphinWindow.controller.statusMessage
        leftPadding: 6
        topPadding: 2
        bottomPadding: 2
        background: Rectangle { color: dolphinWindow.palette.window }
    }

    // Mirrors DolphinZoom::fitScreen()'s own real viewport-size query (m_view->viewport()->
    // size(), m_view->horizontalHeader()->height(), m_view->verticalHeader()->width()) --
    // signalTable's own header items are the Quick equivalent.
    function fitScreenToViewport() {
        dolphinWindow.controller.fitScreen(
            signalTable.width, signalTable.height,
            signalTable.verticalHeaderWidth, signalTable.horizontalHeaderHeight);
    }

    SignalTable {
        id: signalTable
        anchors.fill: parent
        controller: dolphinWindow.controller
    }

    // Mirrors BewavedDolphin::on_actionSetClockWave_triggered(): m_clockPeriod (the dialog's
    // remembered initial value across invocations) lives here as a plain property rather than
    // on the controller -- it's pure dialog UI state the original also never persisted past the
    // window's own lifetime.
    property int lastClockPeriod: 2

    RangeDialog {
        id: clockDialog
        title: qsTr("Clock")
        dialogLabel: qsTr("Clock Period")
        from: 2
        to: 1024
        function openForSelection() {
            clockDialog.value = dolphinWindow.lastClockPeriod;
            clockDialog.open();
        }
        onAccepted: {
            dolphinWindow.lastClockPeriod = clockDialog.value;
            dolphinWindow.controller.applyClockWave(clockDialog.value);
        }
    }

    RangeDialog {
        id: lengthDialog
        title: qsTr("Simulation Length")
        dialogLabel: qsTr("Simulation Length")
        from: 2
        to: 2048
        function openForCurrentLength() {
            lengthDialog.value = dolphinWindow.controller.length;
            lengthDialog.open();
        }
        onAccepted: dolphinWindow.controller.setLength(lengthDialog.value)
    }
}
