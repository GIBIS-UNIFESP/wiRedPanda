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

    RowLayout {
        anchors.fill: parent
        spacing: 0

        ElementPalette {
            Layout.fillHeight: true
            Layout.preferredWidth: 220
        }

        // Hosts the current tab's CanvasItem; reparented here on every tab switch (see
        // Connections below) since each open QuickWorkSpace owns its own CanvasItem instance
        // rather than one shared canvas whose content gets swapped.
        Item {
            id: canvasHost
            Layout.fillWidth: true
            Layout.fillHeight: true

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

            // Drop target for ElementPalette.qml's delegates -- drop.source is the dragged
            // delegate itself (Drag.source defaults to the Item the Drag attached property is
            // set on), so its modelData carries everything addElementToCurrentTab() needs.
            // drop.x/drop.y are already canvasHost-local (DragEvent coordinates are relative
            // to the DropArea), which is also canvas-local since CanvasItem has no pan/zoom
            // transform yet.
            DropArea {
                anchors.fill: parent
                onDropped: (drop) => {
                    appController.addElementToCurrentTab(
                        drop.source.modelData.type, drop.source.modelData.icFileName,
                        drop.source.modelData.isEmbedded, drop.x, drop.y)
                }
            }

            // Right-click / inline-label-edit gestures deferred from Phase 3 (sub-step 6).
            // target re-evaluates whenever currentTab changes since the expression reads
            // appController.currentTab (a property) before calling the plain .canvas() method.
            Connections {
                target: appController.currentTab ? appController.currentTab.canvas() : null

                function onElementContextMenuRequested(element, pos) {
                    appController.elementEditor.prepareContextMenu(element)
                    elementContextMenu.popup(pos.x, pos.y)
                }

                function onEmptyContextMenuRequested(pos) {
                    emptyContextMenu.popup(pos.x, pos.y)
                }

                function onInlineEditRequested(element, currentLabel, targetRect) {
                    inlineLabelEditor.targetElement = element
                    inlineLabelEditor.text = currentLabel
                    inlineLabelEditor.x = targetRect.x
                    inlineLabelEditor.y = targetRect.y
                    inlineLabelEditor.width = Math.max(80, targetRect.width)
                    inlineLabelEditor.visible = true
                    inlineLabelEditor.forceActiveFocus()
                    inlineLabelEditor.selectAll()
                }
            }

            Menu {
                id: elementContextMenu

                MenuItem {
                    text: qsTr("Rename")
                    visible: appController.elementEditor.labelVisible
                    onTriggered: appController.elementEditor.requestRenameFocus()
                }
                MenuItem {
                    text: qsTr("Change trigger")
                    visible: appController.elementEditor.triggerVisible
                    onTriggered: appController.elementEditor.requestTriggerFocus()
                }
                MenuItem { text: qsTr("Rotate left"); onTriggered: appController.rotateLeft() }
                MenuItem { text: qsTr("Rotate right"); onTriggered: appController.rotateRight() }
                MenuItem { text: qsTr("Flip horizontally"); onTriggered: appController.flipHorizontal() }
                MenuItem { text: qsTr("Flip vertically"); onTriggered: appController.flipVertical() }
                Menu {
                    title: qsTr("Change color to...")
                    enabled: appController.elementEditor.colorVisible
                    Repeater {
                        model: appController.elementEditor.colorOptions
                        MenuItem {
                            required property var modelData
                            text: modelData.translatedName
                            onTriggered: appController.elementEditor.color = modelData.name
                        }
                    }
                }
                Menu {
                    title: qsTr("Morph to...")
                    enabled: appController.elementEditor.canMorph && appController.elementEditor.morphCandidates.length > 0
                    Repeater {
                        model: appController.elementEditor.morphCandidates
                        MenuItem {
                            required property var modelData
                            text: modelData.name
                            onTriggered: appController.elementEditor.morphSelectionTo(modelData.type)
                        }
                    }
                }
                MenuSeparator {}
                MenuItem { text: qsTr("Copy"); onTriggered: appController.copy() }
                MenuItem { text: qsTr("Cut"); onTriggered: appController.cut() }
                MenuItem { text: qsTr("Delete"); onTriggered: appController.deleteSelection() }
            }

            // Right-click on empty canvas: mirrors Scene::contextMenu()'s "no item" branch.
            Menu {
                id: emptyContextMenu

                MenuItem {
                    text: qsTr("Paste")
                    enabled: appController.canPaste()
                    onTriggered: appController.paste()
                }
                MenuItem {
                    text: qsTr("Select all")
                    onTriggered: appController.selectAll()
                }
            }

            // Inline label editing (double-click an element with a label). Positioned/sized by
            // onInlineEditRequested above; canvas-local coordinates match canvasHost directly.
            TextField {
                id: inlineLabelEditor
                visible: false
                z: 1000

                property var targetElement: null

                function commit() {
                    if (targetElement) {
                        appController.currentTab.canvas().commitInlineLabelEdit(targetElement, text)
                        targetElement = null
                    }
                    visible = false
                }

                function cancelEdit() {
                    targetElement = null
                    visible = false
                }

                onEditingFinished: commit()
                Keys.onEscapePressed: cancelEdit()
            }
        }

        ElementEditor {
            Layout.fillHeight: true
            Layout.preferredWidth: 240
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
