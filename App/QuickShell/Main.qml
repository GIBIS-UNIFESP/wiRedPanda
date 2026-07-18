// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// Phase 4 sub-step 3: real menu/toolbar/action shell, replacing Phase 1-3's bare single-canvas
// stub. AppController (App/QuickShell/Chrome/QuickAppController, exposed as the AppController
// QML singleton -- see AppControllerForeign in QuickAppController.h) is constructed in Main.cpp
// before this file loads -- it owns the tab list (QuickWorkspaceManager) and exposes every
// action below as a plain, directly-callable method, so this file is close to pure
// layout/wiring: no business logic lives here.
ApplicationWindow {
    id: window

    width: 1280
    height: 800
    visible: true
    title: AppController.windowTitle

    Component.onCompleted: {
        const geom = AppController.restoreWindowGeometry()
        if (geom.width > 0 && geom.height > 0) {
            window.x = geom.x
            window.y = geom.y
            window.width = geom.width
            window.height = geom.height
        }
    }

    onClosing: (close) => {
        if (!AppController.confirmClose()) {
            close.accepted = false
            return
        }
        AppController.saveWindowGeometry(window.x, window.y, window.width, window.height)
    }

    onActiveChanged: AppController.handleWindowActiveChanged(active)

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem { text: qsTr("New"); onTriggered: AppController.newTab() }
            MenuItem { text: qsTr("Open..."); onTriggered: AppController.openFile() }
            MenuItem { text: qsTr("Save"); onTriggered: AppController.saveFile() }
            MenuItem { text: qsTr("Save As..."); onTriggered: AppController.saveFileAs() }
            MenuItem { text: qsTr("Reload"); onTriggered: AppController.reloadFile() }
            Menu {
                title: qsTr("Recent Files")
                enabled: AppController.recentFiles.length > 0
                Repeater {
                    model: AppController.recentFiles
                    MenuItem {
                        required property string modelData
                        text: modelData
                        onTriggered: AppController.openRecentFile(modelData)
                    }
                }
            }
            Menu {
                title: qsTr("Examples")
                Repeater {
                    model: AppController.examplesList()
                    MenuItem {
                        required property var modelData
                        text: modelData.title
                        onTriggered: AppController.openRecentFile(modelData.path)
                    }
                }
            }
            MenuSeparator {}
            MenuItem { text: qsTr("Quit"); onTriggered: Qt.quit() }
        }

        Menu {
            title: qsTr("&Edit")
            MenuItem { text: qsTr("Undo") + (AppController.undoText ? " (" + AppController.undoText + ")" : ""); enabled: AppController.canUndo; onTriggered: AppController.undo() }
            MenuItem { text: qsTr("Redo") + (AppController.redoText ? " (" + AppController.redoText + ")" : ""); enabled: AppController.canRedo; onTriggered: AppController.redo() }
            MenuSeparator {}
            MenuItem { text: qsTr("Cut"); onTriggered: AppController.cut() }
            MenuItem { text: qsTr("Copy"); onTriggered: AppController.copy() }
            MenuItem { text: qsTr("Paste"); onTriggered: AppController.paste() }
            MenuItem { text: qsTr("Duplicate"); onTriggered: AppController.duplicateSelection() }
            MenuItem { text: qsTr("Delete"); onTriggered: AppController.deleteSelection() }
            MenuSeparator {}
            MenuItem { text: qsTr("Select All"); onTriggered: AppController.selectAll() }
        }

        Menu {
            title: qsTr("&Transform")
            MenuItem { text: qsTr("Rotate Right"); onTriggered: AppController.rotateRight() }
            MenuItem { text: qsTr("Rotate Left"); onTriggered: AppController.rotateLeft() }
            MenuItem { text: qsTr("Flip Horizontally"); onTriggered: AppController.flipHorizontal() }
            MenuItem { text: qsTr("Flip Vertically"); onTriggered: AppController.flipVertical() }
            MenuSeparator {}
            MenuItem { text: qsTr("Align Left"); onTriggered: AppController.alignLeft() }
            MenuItem { text: qsTr("Align Right"); onTriggered: AppController.alignRight() }
            MenuItem { text: qsTr("Align Top"); onTriggered: AppController.alignTop() }
            MenuItem { text: qsTr("Align Bottom"); onTriggered: AppController.alignBottom() }
            MenuItem { text: qsTr("Align Horizontal Center"); onTriggered: AppController.alignHorizontalCenter() }
            MenuItem { text: qsTr("Align Vertical Center"); onTriggered: AppController.alignVerticalCenter() }
            MenuItem { text: qsTr("Distribute Horizontally"); onTriggered: AppController.distributeHorizontally() }
            MenuItem { text: qsTr("Distribute Vertically"); onTriggered: AppController.distributeVertically() }
        }

        Menu {
            title: qsTr("&Simulation")
            MenuItem {
                text: qsTr("Play")
                checkable: true
                checked: AppController.simulationRunning
                onTriggered: AppController.simulationRunning = checked
            }
            MenuItem {
                text: qsTr("Mute")
                checkable: true
                onTriggered: AppController.mute(checked)
            }
            MenuItem { text: qsTr("Restart"); onTriggered: AppController.restartSimulation() }
            MenuSeparator {}
            MenuItem {
                text: qsTr("Background Simulation")
                checkable: true
                checked: AppController.backgroundSimulationEnabled
                onTriggered: AppController.backgroundSimulationEnabled = checked
            }
        }

        Menu {
            title: qsTr("E&xport")
            MenuItem { text: qsTr("Arduino..."); onTriggered: AppController.exportArduino() }
            MenuItem { text: qsTr("SystemVerilog..."); onTriggered: AppController.exportSystemVerilog() }
        }

        Menu {
            title: qsTr("&Help")
            MenuItem { text: qsTr("Shortcuts and Tips"); onTriggered: shortcutsDialog.open() }
        }
    }

    header: TabBar {
        id: tabBar
        visible: AppController.tabCount > 0
        currentIndex: AppController.currentIndex
        onCurrentIndexChanged: AppController.currentIndex = currentIndex

        Repeater {
            model: AppController.tabCount
            delegate: TabButton {
                required property int index
                text: AppController.tabTitle(AppController.tabAt(index))
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        ElementPalette {
            Layout.fillHeight: true
            Layout.preferredWidth: 220
            canvasWidth: canvasHost.width
            canvasHeight: canvasHost.height
        }

        // Hosts the current tab's CanvasItem; reparented here on every tab switch (see
        // Connections below) since each open QuickWorkSpace owns its own CanvasItem instance
        // rather than one shared canvas whose content gets swapped.
        Item {
            id: canvasHost
            Layout.fillWidth: true
            Layout.fillHeight: true

            function showCurrentTab() {
                const tab = AppController.currentTab
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
                target: AppController
                function onCurrentTabChanged() { canvasHost.showCurrentTab() }
            }

            // Drop target for ElementPalette.qml's delegates -- drop.source is the dragged
            // delegate itself (Drag.source defaults to the Item the Drag attached property is
            // set on), so its modelData carries everything addElementToCurrentTab() needs.
            // Cast to PaletteItemDelegate (DragEvent.source's declared type is plain QObject,
            // since a drag source can be any Item) so modelData resolves to a real property
            // instead of an unqualified access. drop.x/drop.y are already canvasHost-local
            // (DragEvent coordinates are relative to the DropArea), which is also canvas-local
            // since CanvasItem has no pan/zoom transform yet.
            DropArea {
                anchors.fill: parent
                onDropped: (drop) => {
                    const item = drop.source as PaletteItemDelegate
                    AppController.addElementToCurrentTab(
                        item.modelData.type, item.modelData.icFileName,
                        item.modelData.isEmbedded, drop.x, drop.y)
                }
            }

            // Right-click / inline-label-edit gestures deferred from Phase 3 (sub-step 6).
            // target re-evaluates whenever currentTab changes since the expression reads
            // AppController.currentTab (a property) before calling the plain .canvas() method.
            Connections {
                target: AppController.currentTab ? AppController.currentTab.canvas() : null

                function onElementContextMenuRequested(element, pos) {
                    AppController.elementEditor.prepareContextMenu(element)
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
                    visible: AppController.elementEditor.labelVisible
                    onTriggered: AppController.elementEditor.requestRenameFocus()
                }
                MenuItem {
                    text: qsTr("Change trigger")
                    visible: AppController.elementEditor.triggerVisible
                    onTriggered: AppController.elementEditor.requestTriggerFocus()
                }
                MenuItem { text: qsTr("Rotate left"); onTriggered: AppController.rotateLeft() }
                MenuItem { text: qsTr("Rotate right"); onTriggered: AppController.rotateRight() }
                MenuItem { text: qsTr("Flip horizontally"); onTriggered: AppController.flipHorizontal() }
                MenuItem { text: qsTr("Flip vertically"); onTriggered: AppController.flipVertical() }
                Menu {
                    title: qsTr("Change color to...")
                    enabled: AppController.elementEditor.colorVisible
                    Repeater {
                        model: AppController.elementEditor.colorOptions
                        MenuItem {
                            required property var modelData
                            text: modelData.translatedName
                            onTriggered: AppController.elementEditor.color = modelData.name
                        }
                    }
                }
                Menu {
                    title: qsTr("Morph to...")
                    enabled: AppController.elementEditor.canMorph && AppController.elementEditor.morphCandidates.length > 0
                    Repeater {
                        model: AppController.elementEditor.morphCandidates
                        MenuItem {
                            required property var modelData
                            text: modelData.name
                            onTriggered: AppController.elementEditor.morphSelectionTo(modelData.type)
                        }
                    }
                }
                MenuSeparator {}
                MenuItem { text: qsTr("Copy"); onTriggered: AppController.copy() }
                MenuItem { text: qsTr("Cut"); onTriggered: AppController.cut() }
                MenuItem { text: qsTr("Delete"); onTriggered: AppController.deleteSelection() }
            }

            // Right-click on empty canvas: mirrors Scene::contextMenu()'s "no item" branch.
            Menu {
                id: emptyContextMenu

                MenuItem {
                    text: qsTr("Paste")
                    enabled: AppController.canPaste()
                    onTriggered: AppController.paste()
                }
                MenuItem {
                    text: qsTr("Select all")
                    onTriggered: AppController.selectAll()
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
                        AppController.currentTab.canvas().commitInlineLabelEdit(targetElement, text)
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
            text: AppController.shortcutsHelpHtml()
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            width: parent ? parent.width : 480
        }
    }
}
