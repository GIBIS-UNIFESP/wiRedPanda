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
        const geom = AppController.restoreWindowGeometry();
        if (geom.width > 0 && geom.height > 0) {
            window.x = geom.x;
            window.y = geom.y;
            window.width = geom.width;
            window.height = geom.height;
        }
    }

    onClosing: (close) => {
        if (!AppController.confirmClose()) {
            close.accepted = false;
            return;
        }
        AppController.saveWindowGeometry(window.x, window.y, window.width, window.height);
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
                        required property exampleEntry modelData
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
                checked: AppController.muted
                onTriggered: AppController.muted = checked
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
            MenuSeparator {}
            MenuItem { text: qsTr("PDF..."); onTriggered: AppController.exportPdf() }
            MenuItem { text: qsTr("Image..."); onTriggered: AppController.exportImage() }
        }

        // Mirrors MainWindowUi's View menu, scoped to the Zoom, Minimap, and Theme items --
        // View's remaining items (wire/gate visibility, fast-mode) still depend on chrome that
        // doesn't exist yet in this Quick shell, so they stay out of this menu rather than
        // being built as non-functional entries. Zoom keyboard shortcuts
        // (Ctrl+=/Ctrl+-/Ctrl+0/Ctrl+Shift+F) are handled directly by CanvasItem::keyPressEvent()
        // -- no chrome QAction/Shortcut layer exists yet, same as rotate/flip -- so these
        // MenuItems are plain, unshortcut-ed triggers, matching the Theme submenu below.
        Menu {
            title: qsTr("&View")
            Menu {
                title: qsTr("Zoom")
                MenuItem { text: qsTr("Zoom In"); onTriggered: AppController.zoomIn() }
                MenuItem { text: qsTr("Zoom Out"); onTriggered: AppController.zoomOut() }
                MenuItem { text: qsTr("Reset Zoom"); onTriggered: AppController.resetZoom() }
                MenuItem { text: qsTr("Zoom to Fit"); onTriggered: AppController.zoomToFit() }
            }
            MenuItem {
                text: qsTr("Show Minimap")
                checkable: true
                checked: AppController.minimap.visible
                onTriggered: AppController.minimap.visible = checked
            }
            Menu {
                title: qsTr("Theme")
                MenuItem { text: qsTr("Light"); checkable: true; checked: AppController.theme === 0; onTriggered: AppController.theme = 0 }
                MenuItem { text: qsTr("Dark"); checkable: true; checked: AppController.theme === 1; onTriggered: AppController.theme = 1 }
                MenuItem { text: qsTr("System"); checkable: true; checked: AppController.theme === 2; onTriggered: AppController.theme = 2 }
            }
        }

        Menu {
            title: qsTr("&Language")
            Repeater {
                model: AppController.languages
                MenuItem {
                    required property languageEntry modelData
                    text: modelData.displayName
                    icon.source: modelData.flagIcon
                    checkable: true
                    checked: AppController.currentLanguage === modelData.code
                    onTriggered: AppController.switchLanguage(modelData.code)
                }
            }
        }

        Menu {
            title: qsTr("&Help")
            MenuItem { text: qsTr("Shortcuts and Tips"); onTriggered: shortcutsDialog.open() }
        }
    }

    header: ColumnLayout {
        spacing: 0

        // Mirrors MainWindowUi's mainToolBar exactly (MainWindowUI.cpp's addAction() order and
        // separators): New/Open/Save | Rotate/Cut/Copy/Paste/Delete | Zoom In/Out/Reset/Fit |
        // Play/Restart. Waveform is deliberately excluded -- it opens BeWavedDolphin, not
        // ported to Quick yet (Phase 6); matches this rewrite's "don't build dead UI" precedent
        // (the Learn menu's own deferral).
        ToolBar {
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                spacing: 0

                ToolButton {
                    icon.source: "qrc:/Interface/Toolbar/new.svg"
                    ToolTip.text: qsTr("New")
                    ToolTip.visible: hovered
                    onClicked: AppController.newTab()
                }
                ToolButton {
                    icon.source: "qrc:/Interface/Toolbar/folder.svg"
                    ToolTip.text: qsTr("Open...")
                    ToolTip.visible: hovered
                    onClicked: AppController.openFile()
                }
                ToolButton {
                    icon.source: "qrc:/Interface/Toolbar/save.svg"
                    ToolTip.text: qsTr("Save")
                    ToolTip.visible: hovered
                    onClicked: AppController.saveFile()
                }

                ToolSeparator {}

                ToolButton {
                    icon.source: "qrc:/Interface/Toolbar/rotateL.svg"
                    ToolTip.text: qsTr("Rotate left")
                    ToolTip.visible: hovered
                    onClicked: AppController.rotateLeft()
                }
                ToolButton {
                    icon.source: "qrc:/Interface/Toolbar/rotateR.svg"
                    ToolTip.text: qsTr("Rotate right")
                    ToolTip.visible: hovered
                    onClicked: AppController.rotateRight()
                }
                ToolButton {
                    icon.source: "qrc:/Interface/Toolbar/cut.svg"
                    ToolTip.text: qsTr("Cut")
                    ToolTip.visible: hovered
                    onClicked: AppController.cut()
                }
                ToolButton {
                    icon.source: "qrc:/Interface/Toolbar/copy.svg"
                    ToolTip.text: qsTr("Copy")
                    ToolTip.visible: hovered
                    onClicked: AppController.copy()
                }
                ToolButton {
                    icon.source: "qrc:/Interface/Toolbar/paste.svg"
                    ToolTip.text: qsTr("Paste")
                    ToolTip.visible: hovered
                    onClicked: AppController.paste()
                }
                ToolButton {
                    icon.source: "qrc:/Interface/Toolbar/delete.svg"
                    ToolTip.text: qsTr("Delete")
                    ToolTip.visible: hovered
                    onClicked: AppController.deleteSelection()
                }

                ToolSeparator {}

                ToolButton {
                    icon.source: "qrc:/Interface/Toolbar/zoomIn.svg"
                    ToolTip.text: qsTr("Zoom in")
                    ToolTip.visible: hovered
                    onClicked: AppController.zoomIn()
                }
                ToolButton {
                    icon.source: "qrc:/Interface/Toolbar/zoomOut.svg"
                    ToolTip.text: qsTr("Zoom out")
                    ToolTip.visible: hovered
                    onClicked: AppController.zoomOut()
                }
                ToolButton {
                    icon.source: "qrc:/Interface/Toolbar/zoomReset.svg"
                    ToolTip.text: qsTr("Reset zoom")
                    ToolTip.visible: hovered
                    onClicked: AppController.resetZoom()
                }
                ToolButton {
                    icon.source: "qrc:/Interface/Dolphin/zoomRange.svg"
                    ToolTip.text: qsTr("Zoom to fit")
                    ToolTip.visible: hovered
                    onClicked: AppController.zoomToFit()
                }

                ToolSeparator {}

                ToolButton {
                    checkable: true
                    checked: AppController.simulationRunning
                    icon.source: checked ? "qrc:/Interface/Toolbar/pause.svg" : "qrc:/Interface/Toolbar/play.svg"
                    ToolTip.text: qsTr("Play/Pause")
                    ToolTip.visible: hovered
                    onToggled: AppController.simulationRunning = checked
                }
                ToolButton {
                    icon.source: "qrc:/Interface/Toolbar/reset.svg"
                    ToolTip.text: qsTr("Restart")
                    ToolTip.visible: hovered
                    onClicked: AppController.restartSimulation()
                }

                Item { Layout.fillWidth: true } // pushes the toolbar's own content left
            }
        }

        TabBar {
            id: tabBar
            Layout.fillWidth: true
            visible: AppController.tabCount > 0
            currentIndex: AppController.currentIndex
            onCurrentIndexChanged: AppController.currentIndex = currentIndex

            Repeater {
                model: AppController.tabCount
                delegate: TabButton {
                    required property int index
                    // A real property read (QuickWorkSpace.title), not the invokable
                    // tabTitle()/tabAt() pair -- stays live across Save/Save As renaming an
                    // Untitled tab, since title has a NOTIFY signal behind it.
                    text: AppController.tabAt(index).title
                }
            }
        }
    }

    // Mirrors MainWindow::showStatusMessage()'s real (and only) status-bar contract: a
    // transient message that auto-clears after its timeout (AppController.statusMessage/
    // showStatusMessage() -- see QuickAppController.h). No permanent zoom/selection widgets
    // exist in production's status bar to port alongside it (confirmed by grep, not assumed).
    footer: Label {
        visible: AppController.statusMessage.length > 0
        text: AppController.statusMessage
        leftPadding: 6
        topPadding: 2
        bottomPadding: 2
        background: Rectangle { color: window.palette.window }
    }

    // Mirrors MainWindowUI.cpp's real structure: one QSplitter with the palette+editor stacked
    // in a single left pane (a QGridLayout there, a ColumnLayout here) and the canvas tabs as
    // the other pane -- not two independent side-by-side columns the way an earlier pass of
    // this Quick chrome had it. SplitView (QtQuick.Controls, already imported) is the direct
    // equivalent of QSplitter -- no reason to hand-roll drag logic the way Minimap.qml's
    // floating-overlay resize needed to, since this is exactly the two-pane case SplitView is
    // built for.
    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        ColumnLayout {
            id: leftPane
            // 280-600px, matching leftPannel's own setMinimumSize(280,0)/setMaximumSize(600,...).
            SplitView.preferredWidth: 280
            SplitView.minimumWidth: 280
            SplitView.maximumWidth: 600
            spacing: 0

            Component.onCompleted: {
                const saved = AppController.restoreSplitterWidth();
                if (saved > 0) {
                    SplitView.preferredWidth = saved;
                }
            }
            // Debounced (not every drag frame) persistence, mirroring Minimap.qml's own
            // "commit on release, not every frame" discipline -- SplitView has no direct
            // per-pane "drag finished" signal to hook, so a short idle timer stands in for one.
            onWidthChanged: splitterPersistTimer.restart()
            Timer {
                id: splitterPersistTimer
                interval: 300
                onTriggered: AppController.saveSplitterWidth(leftPane.width)
            }

            ElementPalette {
                Layout.fillWidth: true
                Layout.fillHeight: true
                canvasWidth: canvasHost.width
                canvasHeight: canvasHost.height
            }

            // Mirrors ElementEditor::setCurrentElements()'s hide()/show() exactly: the whole
            // panel disappears when nothing is selected (no placeholder message), collapsing
            // this column's remaining space back to the palette above -- see
            // ElementEditor.qml's own visible: binding. Layout.fillHeight (capped by
            // maximumHeight so it doesn't crowd out the palette above) is required, not
            // optional: an invisible item is excluded from ColumnLayout's size calculation
            // entirely, but once visible, a plain Item with no explicit height renders at zero
            // height without this -- a real, previously-unnoticed regression from this file's
            // own SplitView restructure (the prior RowLayout-based placement had this same
            // Layout.fillHeight; it was dropped in the move and never re-verified with a real
            // selection, only the empty/collapsed case).
            ElementEditor {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.maximumHeight: 360
            }
        }

        // Hosts the current tab's CanvasItem; reparented here on every tab switch (see
        // Connections below) since each open QuickWorkSpace owns its own CanvasItem instance
        // rather than one shared canvas whose content gets swapped.
        Item {
            id: canvasHost
            SplitView.fillWidth: true

            function showCurrentTab() {
                const tab = AppController.currentTab;
                if (!tab) {
                    return;
                }
                const canvas = tab.canvas;
                canvas.parent = canvasHost;
                canvas.anchors.fill = canvasHost;
                canvas.forceActiveFocus();
            }

            Component.onCompleted: showCurrentTab()

            Connections {
                target: AppController
                function onCurrentTabChanged() { canvasHost.showCurrentTab(); }
            }

            // Drop target for ElementPalette.qml's delegates -- drop.source is the dragged
            // delegate itself (Drag.source defaults to the Item the Drag attached property is
            // set on), so its modelData carries everything addElementToCurrentTab() needs.
            // Cast to PaletteItemDelegate (DragEvent.source's declared type is plain QObject,
            // since a drag source can be any Item) so modelData resolves to a real property
            // instead of an unqualified access. drop.x/drop.y are canvasHost-local (DragEvent
            // coordinates are relative to the DropArea) -- i.e. CanvasItem's own screen space --
            // which addElementFromPalette() converts to world coordinates internally via
            // screenToWorld(), so drops land correctly under whatever pan/zoom is active.
            DropArea {
                anchors.fill: parent
                onDropped: (drop) => {
                    const item = drop.source as PaletteItemDelegate;
                    AppController.addElementToCurrentTab(
                        item.modelData.type, item.modelData.icFileName,
                        item.modelData.isEmbedded, drop.x, drop.y);
                }
            }

            // Right-click / inline-label-edit gestures deferred from Phase 3 (sub-step 6).
            // target re-evaluates whenever currentTab changes since the expression reads
            // AppController.currentTab (a property) before reading the plain canvas property.
            Connections {
                target: AppController.currentTab ? AppController.currentTab.canvas : null

                // `element` stays untyped in these three handlers: GraphicElement is
                // deliberately not QML-registered at all (Layer-1 domain code, see
                // CanvasItem.h), so it has no QML type name a type annotation could name.
                function onElementContextMenuRequested(element, pos: point) {
                    AppController.elementEditor.prepareContextMenu(element);
                    elementContextMenu.popup(pos.x, pos.y);
                }

                function onEmptyContextMenuRequested(pos: point) {
                    // canPaste() reads the live OS clipboard directly (no NOTIFY-backed
                    // property behind it), so it can't be a live `enabled:` binding --
                    // recompute it fresh here, mirroring Scene::contextMenu()'s own
                    // "recompute when the menu is built" behavior.
                    pasteMenuItem.enabled = AppController.canPaste();
                    emptyContextMenu.popup(pos.x, pos.y);
                }

                function onInlineEditRequested(element, currentLabel: string, targetRect: rect) {
                    inlineLabelEditor.targetElement = element;
                    inlineLabelEditor.text = currentLabel;
                    inlineLabelEditor.x = targetRect.x;
                    inlineLabelEditor.y = targetRect.y;
                    inlineLabelEditor.width = Math.max(80, targetRect.width);
                    inlineLabelEditor.visible = true;
                    inlineLabelEditor.forceActiveFocus();
                    inlineLabelEditor.selectAll();
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
                            required property colorOption modelData
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
                            required property morphCandidate modelData
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
                    id: pasteMenuItem
                    text: qsTr("Paste")
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
                        AppController.currentTab.canvas.commitInlineLabelEdit(targetElement, text);
                        targetElement = null;
                    }
                    visible = false;
                }

                function cancelEdit() {
                    targetElement = null;
                    visible = false;
                }

                onEditingFinished: commit()
                Keys.onEscapePressed: cancelEdit()
            }

            // IC hover preview (Phase 4 sub-step 6's last deferred gesture). Reparents itself to
            // the window's content area (see its own parent: binding) since its screenPos is a
            // global coordinate, not canvasHost-local -- declared here only for proximity to the
            // other canvasHost-adjacent overlays above.
            ICPreviewPopup {}

            // Circuit-overview minimap (the pan/zoom follow-up's own follow-up -- see
            // Minimap.qml's own doc comment). Genuinely canvasHost-local (unlike
            // ICPreviewPopup), so no reparenting is needed here.
            Minimap {}
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

    // Opened via AppController.elementEditor.openTruthTable() (ElementEditor.qml's "Edit Truth
    // Table..." button), mirroring shortcutsDialog's own open()-by-id precedent.
    TruthTableDialog {
        id: truthTableDialog
    }
    Connections {
        target: AppController.elementEditor
        function onTruthTableRequested() { truthTableDialog.open() }
    }
}
