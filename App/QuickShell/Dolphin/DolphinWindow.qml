// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// Quick-side port of BewavedDolphin's QMainWindow shell. Mirrors BewavedDolphinUi::setupUi()'s
// menu/toolbar structure -- Edit's undo/clipboard/cell-edit actions and File's Load/Save/Export
// family are all real, backed by QuickDolphinController methods that own their own file dialogs
// (FileDialogs::provider() already works unmodified here, same precedent QuickExportController's
// export actions established). Help reuses Main.qml's own aboutDialog/aboutQtDialog content via
// a duplicated shell -- see the Help menu and F1/Ctrl+Shift+H shortcuts below. Launched
// for real via Main.qml's static instance + AppController.openWaveform()/waveformOpenRequested --
// QuickAppController implements DolphinHost, so the "link this file to the current circuit" save
// prompt and window-title file-name decoration are real too (onClosing below).
ApplicationWindow {
    id: dolphinWindow

    required property QuickDolphinController controller

    // Mirrors BewavedDolphin::BewavedDolphin()'s `setWindowModality(Qt::WindowModal)` -- "the
    // user cannot interact with the main circuit while the waveform is open". Window.modality
    // targets this window's transientParent, which Qt Quick sets automatically here since
    // dolphinWindow is declared as a child Window of Main.qml's top-level `window`.
    modality: Qt.WindowModal

    width: 800
    height: 500
    visible: true
    title: qsTr("beWavedDolphin Simulator") + (dolphinWindow.controller.currentFileName.length > 0 ? " [" + dolphinWindow.controller.currentFileName + "]" : "")

    Component.onCompleted: {
        const geom = AppController.restoreDolphinWindowGeometry();
        if (geom.width > 0 && geom.height > 0) {
            dolphinWindow.x = geom.x;
            dolphinWindow.y = geom.y;
            dolphinWindow.width = geom.width;
            dolphinWindow.height = geom.height;
        }
    }

    onClosing: (close) => {
        if (!dolphinWindow.controller.checkSave()) {
            close.accepted = false;
            return;
        }
        AppController.saveDolphinWindowGeometry(dolphinWindow.x, dolphinWindow.y, dolphinWindow.width, dolphinWindow.height);
        AppController.notifyWaveformClosed();
    }

    // Mirrors BewavedDolphinUI.cpp's retranslateUi() shortcut assignments exactly. QML's Shortcut
    // item is this project's established mechanism for window-scoped shortcuts (see
    // TourOverlay.qml's Escape shortcut) -- each one below calls the same controller/menu-item
    // method the corresponding MenuItem/ToolButton above already calls.
    Shortcut { sequence: "Ctrl+L"; onActivated: dolphinWindow.controller.load() }
    Shortcut { sequence: "Ctrl+W"; onActivated: dolphinWindow.close() }
    Shortcut { sequence: "Alt+C"; onActivated: dolphinWindow.controller.combinational() }
    Shortcut { sequence: "Ctrl+P"; onActivated: dolphinWindow.controller.exportToPdf() }
    Shortcut { sequence: "Ctrl+S"; onActivated: dolphinWindow.controller.save() }
    Shortcut { sequence: "Ctrl+Shift+S"; onActivated: dolphinWindow.controller.saveAs() }
    Shortcut { sequence: StandardKey.Undo; onActivated: dolphinWindow.controller.undo() }
    Shortcut { sequence: StandardKey.Redo; onActivated: dolphinWindow.controller.redo() }
    Shortcut { sequence: "Ctrl+C"; onActivated: dolphinWindow.controller.copy() }
    Shortcut { sequence: "Ctrl+V"; onActivated: dolphinWindow.controller.paste() }
    Shortcut { sequence: "0"; onActivated: dolphinWindow.controller.setSelectedTo0() }
    Shortcut { sequence: "1"; onActivated: dolphinWindow.controller.setSelectedTo1() }
    Shortcut { sequence: "Alt+W"; onActivated: clockDialog.openForSelection() }
    Shortcut { sequence: "Space"; onActivated: dolphinWindow.controller.invertSelected() }
    Shortcut { sequence: "Ctrl+Shift+P"; onActivated: dolphinWindow.controller.exportToPng() }
    Shortcut { sequence: "Alt+L"; onActivated: lengthDialog.openForCurrentLength() }
    Shortcut { sequence: "Ctrl+Shift+H"; onActivated: aboutQtDialog.open() }
    Shortcut { sequence: "Ctrl+=" ; onActivated: dolphinWindow.controller.zoomIn() }
    Shortcut { sequence: "Ctrl+-"; onActivated: dolphinWindow.controller.zoomOut() }
    Shortcut { sequence: "Ctrl+Shift+F"; onActivated: dolphinWindow.fitScreenToViewport() }
    Shortcut { sequence: "Ctrl+Home"; onActivated: dolphinWindow.controller.resetZoom() }
    Shortcut { sequence: "Alt+X"; onActivated: dolphinWindow.controller.clearInputs() }
    Shortcut { sequence: "Ctrl+X"; onActivated: dolphinWindow.controller.cut() }
    Shortcut { sequence: "Alt+A"; onActivated: dolphinWindow.controller.autoCrop() }
    Shortcut { sequence: "F1"; onActivated: aboutDialog.open() }

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

        // Mirrors BewavedDolphinUI.cpp's menuAbout ("Help" menu, oddly named after its first
        // action) -- reuses AppController.aboutHtml()/aboutQtHtml() via the same two-Dialog shell
        // Main.qml's own aboutDialog/aboutQtDialog already establish, rather than duplicating
        // their HTML content here.
        Menu {
            title: qsTr("&Help")
            MenuItem { text: qsTr("About"); onTriggered: aboutDialog.open() }
            MenuItem { text: qsTr("About Qt"); onTriggered: aboutQtDialog.open() }
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
        basePointSize: dolphinWindow.font.pointSize
    }

    // Mirrors BewavedDolphin::on_actionSetClockWave_triggered(): m_clockPeriod (the dialog's
    // remembered initial value across invocations) lives here as a plain property rather than
    // on the controller -- it's pure dialog UI state the original also never persisted past the
    // window's own lifetime.
    property int lastClockPeriod: 2

    RangeDialog {
        id: clockDialog
        title: qsTr("Clock Period Selection")
        dialogLabel: qsTr("Clock Period")
        from: 2
        to: 1024
        step: 2
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
        title: qsTr("Simulation Length Selection")
        dialogLabel: qsTr("Simulation Length")
        from: 2
        to: 2048
        function openForCurrentLength() {
            lengthDialog.value = dolphinWindow.controller.length;
            lengthDialog.open();
        }
        onAccepted: dolphinWindow.controller.setLength(lengthDialog.value)
    }

    // Mirrors Main.qml's aboutDialog/aboutQtDialog exactly (same shared AppController content
    // methods, duplicated shell only) -- see the Help menu above and the F1/Ctrl+Shift+H shortcuts.
    Dialog {
        id: aboutDialog
        title: qsTr("wiRedPanda")
        modal: true
        standardButtons: Dialog.Ok
        anchors.centerIn: parent
        width: 480
        contentItem: Label {
            text: AppController.aboutHtml()
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            width: aboutDialog.availableWidth
            onLinkActivated: (link) => Qt.openUrlExternally(link)
        }
    }

    Dialog {
        id: aboutQtDialog
        title: qsTr("About Qt")
        modal: true
        standardButtons: Dialog.Ok
        anchors.centerIn: parent
        width: 480
        contentItem: Label {
            text: AppController.aboutQtHtml()
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            width: aboutQtDialog.availableWidth
            onLinkActivated: (link) => Qt.openUrlExternally(link)
        }
    }
}
