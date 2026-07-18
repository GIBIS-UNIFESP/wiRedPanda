// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// Phase 4 sub-step 3: real menu/toolbar/action shell, replacing Phase 1-3's bare single-canvas
// stub. appController (App/QuickShell/Chrome/QuickAppController) is set as a context property
// in Main.cpp before this file loads -- it owns the tab list (QuickWorkspaceManager) and
// exposes every action below as a plain, directly-callable method, so this file is close to
// pure layout/wiring: no business logic lives here.
ApplicationWindow {
    id: window

    width: 1280
    height: 800
    visible: true
    title: appController.windowTitle

    Component.onCompleted: {
        const geom = appController.restoreWindowGeometry()
        if (geom.width > 0 && geom.height > 0) {
            window.x = geom.x
            window.y = geom.y
            window.width = geom.width
            window.height = geom.height
        }
    }

    onClosing: (close) => {
        if (!appController.confirmClose()) {
            close.accepted = false
            return
        }
        appController.saveWindowGeometry(window.x, window.y, window.width, window.height)
    }

    onActiveChanged: appController.handleWindowActiveChanged(active)

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem { text: qsTr("New"); onTriggered: appController.newTab() }
            MenuItem { text: qsTr("Open..."); onTriggered: appController.openFile() }
            MenuItem { text: qsTr("Save"); onTriggered: appController.saveFile() }
            MenuItem { text: qsTr("Save As..."); onTriggered: appController.saveFileAs() }
            MenuItem { text: qsTr("Reload"); onTriggered: appController.reloadFile() }
            Menu {
                title: qsTr("Recent Files")
                enabled: appController.recentFiles.length > 0
                Repeater {
                    model: appController.recentFiles
                    MenuItem {
                        required property string modelData
                        text: modelData
                        onTriggered: appController.openRecentFile(modelData)
                    }
                }
            }
            Menu {
                title: qsTr("Examples")
                Repeater {
                    model: appController.examplesList()
                    MenuItem {
                        required property var modelData
                        text: modelData.title
                        onTriggered: appController.openRecentFile(modelData.path)
                    }
                }
            }
            MenuSeparator {}
            MenuItem { text: qsTr("Quit"); onTriggered: Qt.quit() }
        }

        Menu {
            title: qsTr("&Edit")
            MenuItem { text: qsTr("Undo") + (appController.undoText ? " (" + appController.undoText + ")" : ""); enabled: appController.canUndo; onTriggered: appController.undo() }
            MenuItem { text: qsTr("Redo") + (appController.redoText ? " (" + appController.redoText + ")" : ""); enabled: appController.canRedo; onTriggered: appController.redo() }
            MenuSeparator {}
            MenuItem { text: qsTr("Cut"); onTriggered: appController.cut() }
            MenuItem { text: qsTr("Copy"); onTriggered: appController.copy() }
            MenuItem { text: qsTr("Paste"); onTriggered: appController.paste() }
            MenuItem { text: qsTr("Duplicate"); onTriggered: appController.duplicateSelection() }
            MenuItem { text: qsTr("Delete"); onTriggered: appController.deleteSelection() }
            MenuSeparator {}
            MenuItem { text: qsTr("Select All"); onTriggered: appController.selectAll() }
        }

        Menu {
            title: qsTr("&Transform")
            MenuItem { text: qsTr("Rotate Right"); onTriggered: appController.rotateRight() }
            MenuItem { text: qsTr("Rotate Left"); onTriggered: appController.rotateLeft() }
            MenuItem { text: qsTr("Flip Horizontally"); onTriggered: appController.flipHorizontal() }
            MenuItem { text: qsTr("Flip Vertically"); onTriggered: appController.flipVertical() }
            MenuSeparator {}
            MenuItem { text: qsTr("Align Left"); onTriggered: appController.alignLeft() }
            MenuItem { text: qsTr("Align Right"); onTriggered: appController.alignRight() }
            MenuItem { text: qsTr("Align Top"); onTriggered: appController.alignTop() }
            MenuItem { text: qsTr("Align Bottom"); onTriggered: appController.alignBottom() }
            MenuItem { text: qsTr("Align Horizontal Center"); onTriggered: appController.alignHorizontalCenter() }
            MenuItem { text: qsTr("Align Vertical Center"); onTriggered: appController.alignVerticalCenter() }
            MenuItem { text: qsTr("Distribute Horizontally"); onTriggered: appController.distributeHorizontally() }
            MenuItem { text: qsTr("Distribute Vertically"); onTriggered: appController.distributeVertically() }
        }

        Menu {
            title: qsTr("&Simulation")
            MenuItem {
                text: qsTr("Play")
                checkable: true
                checked: appController.simulationRunning
                onTriggered: appController.simulationRunning = checked
            }
            MenuItem {
                text: qsTr("Mute")
                checkable: true
                onTriggered: appController.mute(checked)
            }
            MenuItem { text: qsTr("Restart"); onTriggered: appController.restartSimulation() }
            MenuSeparator {}
            MenuItem {
                text: qsTr("Background Simulation")
                checkable: true
                checked: appController.backgroundSimulationEnabled
                onTriggered: appController.backgroundSimulationEnabled = checked
            }
        }

        Menu {
            title: qsTr("E&xport")
            MenuItem { text: qsTr("Arduino..."); onTriggered: appController.exportArduino() }
            MenuItem { text: qsTr("SystemVerilog..."); onTriggered: appController.exportSystemVerilog() }
        }

        Menu {
            title: qsTr("&Help")
            MenuItem { text: qsTr("Shortcuts and Tips"); onTriggered: shortcutsDialog.open() }
        }
    }

    header: TabBar {
        id: tabBar
        visible: appController.tabCount > 0
        currentIndex: appController.currentIndex
        onCurrentIndexChanged: appController.currentIndex = currentIndex

        Repeater {
            model: appController.tabCount
            delegate: TabButton {
                required property int index
                text: appController.tabTitle(appController.tabAt(index))
            }
        }
    }

    // Hosts the current tab's CanvasItem; reparented here on every tab switch (see
    // Connections below) since each open QuickWorkSpace owns its own CanvasItem instance
    // rather than one shared canvas whose content gets swapped.
    Item {
        id: canvasHost
        anchors.fill: parent

        function showCurrentTab() {
            const tab = appController.currentTab
            if (!tab) {
                return
            }
            const canvas = tab.canvas()
            canvas.parent = canvasHost
            canvas.anchors.fill = canvasHost
            canvas.forceActiveFocus()
        }

        Component.onCompleted: showCurrentTab()

        Connections {
            target: appController
            function onCurrentTabChanged() { canvasHost.showCurrentTab() }
        }
    }

    Dialog {
        id: shortcutsDialog
        title: qsTr("Shortcuts and Tips")
        modal: true
        standardButtons: Dialog.Ok
        anchors.centerIn: parent
        // A fixed width breaks the implicitWidth<->contentItem.implicitWidth binding loop a
        // bare, unconstrained Label as contentItem would otherwise create (Dialog sizes to
        // its content, content wraps to the Dialog's width -- circular without an anchor).
        width: 480
        contentItem: Label {
            text: appController.shortcutsHelpHtml()
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            width: parent ? parent.width : 480
        }
    }
}
