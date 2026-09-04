// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// Lets paletteItemComponent's PaletteItemDelegate {} reference this file's root id (see
// canvasWidth/canvasHeight below) in a way qmllint can verify statically -- the pragma binds
// inner Components to their defining context; modelData already flows in via a required
// property, not a context property, so this loses nothing this file relies on.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// Element palette panel, backed by App/QuickShell/Chrome/QuickElementPalette
// (AppController.elementPalette). Every category grid and the search grid share one delegate
// (paletteItemComponent) so drag-to-canvas and double-click-to-add behave identically
// everywhere an element can be picked from -- mirrors ElementPalette::populateMenu()'s "every
// entry gets the same ElementLabel behavior" invariant.
//
// The IC tab holds two sections (file-based ICs, embedded ICs) in one scrollable column,
// mirroring MainWindowUi's single "ic" tab widget (scrollAreaWidgetContents_IC and
// scrollAreaWidgetContents_EmbeddedIC are both children of the same tab, not separate tabs).
// Each section is wrapped in a DropArea (ICDropZone port) so dragging the opposite type onto it
// embeds/extracts; a trash drop target below both (TrashButton port) removes whichever is
// dropped on it -- all three wired to QuickICController via AppController. Each section's own
// "Add IC..." ToolButton (pushButtonAddIC/pushButtonAddEmbeddedIC in production) and the "Make
// Self-Contained" Button below the trash target (pushButtonMakeSelfContained/
// actionMakeSelfContained in production) live here; Embed/Extract for a *selected* IC live in
// ElementEditor.qml instead, next to its blob-rename field, since they act on the current
// selection rather than the palette. There's no click-triggered popup on the trash target since
// its "Drag here to remove" label below is already always visible (see QuickICController's own
// doc comment).
Item {
    id: root
    implicitWidth: 220

    readonly property QuickElementPalette controller: AppController.elementPalette
    // Exposed for Main.qml's tour target-resolution/click-dispatch functions -- neither is
    // otherwise reachable from outside this file.
    property alias categoryTabBar: categoryTabBar
    property alias searchField: searchField
    // Canvas-area size, for centering a newly-added element (double-click/search-Enter
    // below) -- passed down explicitly by Main.qml rather than reached for via an implicit
    // ancestor-id lookup (this component doesn't otherwise know or need to know that a
    // sibling called canvasHost exists).
    property real canvasWidth: 0
    property real canvasHeight: 0

    Component {
        id: paletteItemComponent

        PaletteItemDelegate {
            canvasWidth: root.canvasWidth
            canvasHeight: root.canvasHeight
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 4

        // MainWindowUI.cpp's search row is a separate "Search:" QLabel + a plain,
        // placeholder-less lineEditSearch (label + lineEditSearch, both direct leftPannel
        // children) -- not one merged placeholder-text field.
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 4

            Label {
                text: qsTr("Search:")
            }

            TextField {
                id: searchField
                Layout.fillWidth: true
                onTextChanged: root.controller.searchText = text
                // Mirrors ElementPalette.cpp's setAccessibleName()/setWhatsThis() pair for
                // lineEditSearch.
                Accessible.name: qsTr("Search elements")
                Accessible.description: qsTr("Type to filter the palette by element name. Press Enter to add the first match to the circuit.")

                Keys.onReturnPressed: {
                    const result = root.controller.firstSearchResult()
                    if (result.type !== undefined) {
                        AppController.addElementToCurrentTab(
                            result.type, result.icFileName, result.isEmbedded,
                            root.canvasWidth / 2, root.canvasHeight / 2)
                        searchField.text = ""
                    }
                }
            }
        }

        // tabElements (MainWindowUI.cpp) is a real QTabWidget, whose Fusion style draws a
        // border around the tab strip + content pane as one unit -- this Frame reproduces
        // that bordered "pane" look; the search field above deliberately stays outside it,
        // matching Widgets' separate lineEditSearch row.
        Frame {
            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: 0

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Icon-only tabs with tooltips, matching MainWindowUI.cpp's tabElements
                // exactly (tabElements->addTab(..., QIcon(...), QString()) -- empty text,
                // real name as a tooltip). icon.width/height double Fusion TabButton.qml's own
                // default (16x16) deliberately, not a parity gap.
                TabBar {
                    id: categoryTabBar
                    Layout.fillWidth: true
                    visible: searchField.text.length === 0
                    // Mirrors ElementPalette.cpp's setAccessibleName()/setWhatsThis() pair for
                    // tabElements.
                    Accessible.name: qsTr("Element palette")
                    Accessible.description: qsTr("Elements grouped by category. Drag one onto the canvas, or double-click to add it to the active circuit.")

                    TabButton { icon.source: "qrc:/Components/Input/buttonOff.svg"; icon.width: 32; icon.height: 32; ToolTip.text: qsTr("Inputs/Outputs"); ToolTip.visible: hovered }
                    TabButton { icon.source: "qrc:/Components/Logic/xor.svg"; icon.width: 32; icon.height: 32; ToolTip.text: qsTr("Gates"); ToolTip.visible: hovered }
                    TabButton { icon.source: "qrc:/Components/Logic/truthtable-rotated.svg"; icon.width: 32; icon.height: 32; ToolTip.text: qsTr("Combinational"); ToolTip.visible: hovered }
                    TabButton { icon.source: "qrc:/Components/Memory/Light/D-flipflop.svg"; icon.width: 32; icon.height: 32; ToolTip.text: qsTr("Memory"); ToolTip.visible: hovered }
                    TabButton { icon.source: "qrc:/Components/Logic/ic-panda.svg"; icon.width: 32; icon.height: 32; ToolTip.text: qsTr("Integrated Circuits"); ToolTip.visible: hovered }
                    TabButton { icon.source: "qrc:/Components/Misc/text.png"; icon.width: 32; icon.height: 32; ToolTip.text: qsTr("Miscellaneous"); ToolTip.visible: hovered }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: searchField.text.length > 0 ? 6 : categoryTabBar.currentIndex

                    // Every scrollAreaXxx in MainWindowUI.cpp (scrollAreaInOut, scrollAreaGates,
                    // scrollArea_Combinational, scrollAreaMemory, scrollArea_Misc, scrollArea_
                    // Search, scrollArea_Box, scrollAreaEmbeddedIC) is a real QScrollArea, whose
                    // Fusion-styled scrollbar is always visible once content overflows --
                    // ScrollBar.vertical here (and on every other ListView below) reproduces
                    // that, in place of Quick's default invisible-until-flicked overlay.
                    // ThickScrollBar (not the bare QtQuick.Controls ScrollBar) for width -- see
                    // its own doc comment.
                    ListView { clip: true; model: root.controller.ioElements; delegate: paletteItemComponent; ScrollBar.vertical: ThickScrollBar { policy: ScrollBar.AlwaysOn } }
                    ListView { clip: true; model: root.controller.gatesElements; delegate: paletteItemComponent; ScrollBar.vertical: ThickScrollBar { policy: ScrollBar.AlwaysOn } }
                    ListView { clip: true; model: root.controller.combinationalElements; delegate: paletteItemComponent; ScrollBar.vertical: ThickScrollBar { policy: ScrollBar.AlwaysOn } }
                    ListView { clip: true; model: root.controller.memoryElements; delegate: paletteItemComponent; ScrollBar.vertical: ThickScrollBar { policy: ScrollBar.AlwaysOn } }

                    ColumnLayout {
                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: qsTr("Files"); leftPadding: 4; Layout.fillWidth: true }
                            // ICController::addICFromFile() -- import a .panda file into the project.
                            // Mirrors MainWindow.cpp's setICButtonsVisible(!isInlineIC())/
                            // refreshICButtonsEnabled() exactly: an inline IC tab has no project
                            // file/directory for this to copy into, and an unsaved project has none
                            // yet either.
                            ToolButton {
                                text: qsTr("Add IC files")
                                visible: AppController.icButtonsVisible
                                enabled: AppController.icButtonsEnabled
                                onClicked: AppController.addICFromFile()
                            }
                        }
                        // ICDropZone port (Section::FileBased): dropping an embedded IC label here
                        // extracts it to a file. drag.source is only meaningful while containsDrag is
                        // true (no drag in progress otherwise), hence the "as PaletteItemDelegate" cast
                        // + null check rather than trusting it unconditionally.
                        Item {
                            id: fileBasedZone
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.max(78, filesGrid.contentHeight)
                            // Mirrors ICDropZone.cpp's setAccessibleName()/setWhatsThis() pair for
                            // Section::FileBased.
                            Accessible.name: qsTr("File-based IC drop zone")
                            Accessible.description: qsTr("Drop an embedded IC here to extract it to its own file.")

                            ListView {
                                id: filesGrid
                                anchors.fill: parent
                                clip: true
                                model: root.controller.icElements
                                delegate: paletteItemComponent
                            }

                            DropArea {
                                id: fileBasedDropArea
                                anchors.fill: parent
                                readonly property PaletteItemDelegate dragged: drag.source as PaletteItemDelegate
                                onDropped: (drop) => {
                                    const item = drop.source as PaletteItemDelegate
                                    if (item && item.modelData.isEmbedded) {
                                        AppController.extractICByBlobName(item.modelData.icFileName)
                                    }
                                }
                            }

                            Rectangle {
                                anchors.fill: parent
                                visible: fileBasedDropArea.containsDrag && fileBasedDropArea.dragged !== null
                                         && fileBasedDropArea.dragged.modelData.isEmbedded
                                radius: 4
                                color: "#3381cc"
                                border.color: "white"
                                border.width: 2

                                Label {
                                    anchors.centerIn: parent
                                    width: parent.width - 16
                                    wrapMode: Text.WordWrap
                                    horizontalAlignment: Text.AlignHCenter
                                    color: "white"
                                    font.bold: true
                                    text: qsTr("Drop here to extract this IC to a file")
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: qsTr("Embedded"); leftPadding: 4; Layout.fillWidth: true }
                            // ICController::addEmbeddedICFromFile() -- register a .panda file as an
                            // embedded blob directly, without a scene instance.
                            ToolButton {
                                text: qsTr("Embed IC")
                                onClicked: AppController.addEmbeddedICFromFile()
                            }
                        }
                        // ICDropZone port (Section::Embedded): dropping a file-based IC label here
                        // embeds it.
                        Item {
                            id: embeddedZone
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            // Mirrors ICDropZone.cpp's setAccessibleName()/setWhatsThis() pair for
                            // Section::Embedded.
                            Accessible.name: qsTr("Embedded IC drop zone")
                            Accessible.description: qsTr("Drop a file-based IC here to embed it directly in the circuit.")

                            ListView {
                                id: embeddedGrid
                                anchors.fill: parent
                                clip: true
                                model: root.controller.embeddedICElements
                                delegate: paletteItemComponent
                                ScrollBar.vertical: ThickScrollBar { policy: ScrollBar.AlwaysOn }
                            }

                            DropArea {
                                id: embeddedDropArea
                                anchors.fill: parent
                                readonly property PaletteItemDelegate dragged: drag.source as PaletteItemDelegate
                                onDropped: (drop) => {
                                    const item = drop.source as PaletteItemDelegate
                                    if (item && !item.modelData.isEmbedded) {
                                        AppController.embedICByFile(item.modelData.icFileName)
                                    }
                                }
                            }

                            Rectangle {
                                anchors.fill: parent
                                visible: embeddedDropArea.containsDrag && embeddedDropArea.dragged !== null
                                         && !embeddedDropArea.dragged.modelData.isEmbedded
                                radius: 4
                                color: "#3381cc"
                                border.color: "white"
                                border.width: 2

                                Label {
                                    anchors.centerIn: parent
                                    width: parent.width - 16
                                    wrapMode: Text.WordWrap
                                    horizontalAlignment: Text.AlignHCenter
                                    color: "white"
                                    font.bold: true
                                    text: qsTr("Drop here to embed this IC in the circuit")
                                }
                            }
                        }

                        // TrashButton port: drag either an embedded or file-based IC label here to
                        // remove it (after a confirmation -- handled in C++, see QuickICController's
                        // own doc comment on why the confirm lives there instead of here).
                        Item {
                            id: trashZone
                            Layout.fillWidth: true
                            Layout.preferredHeight: 32
                            // Mirrors MainWindow::setICButtonsVisible() hiding pushButtonAddIC,
                            // pushButtonRemoveIC, and pushButtonMakeSelfContained together on an
                            // inline-IC tab -- this drop zone is the Quick equivalent of
                            // pushButtonRemoveIC and needs the same gate the other two buttons
                            // already have.
                            visible: AppController.icButtonsVisible
                            Accessible.name: qsTr("Remove IC")
                            Accessible.description: qsTr("Drag an IC here to remove it.")

                            Rectangle {
                                anchors.fill: parent
                                radius: 4
                                color: trashDropArea.containsDrag ? "#cc3333" : "transparent"
                                border.color: trashZone.palette.mid
                                border.width: 1
                            }

                            RowLayout {
                                anchors.centerIn: parent
                                spacing: 4
                                // Mirrors TrashButton's real trash-can icon (setIcon(QIcon(":/
                                // Interface/Toolbar/trashcan.svg"))); Quick's target is a
                                // passive drop area rather than a real clickable button (see this
                                // file's header comment on why there's no click-triggered popup),
                                // but the icon itself is a one-line visual parity fix.
                                Image {
                                    source: "qrc:/Interface/Toolbar/trashcan.svg"
                                    sourceSize.width: 16
                                    sourceSize.height: 16
                                }
                                Label {
                                    text: qsTr("Drag here to remove")
                                }
                            }

                            DropArea {
                                id: trashDropArea
                                anchors.fill: parent
                                onDropped: (drop) => {
                                    const item = drop.source as PaletteItemDelegate
                                    if (!item) {
                                        return
                                    }
                                    if (item.modelData.isEmbedded) {
                                        AppController.removeEmbeddedIC(item.modelData.icFileName)
                                    } else {
                                        AppController.removeICFile(item.modelData.icFileName)
                                    }
                                }
                            }
                        }

                        // ICController::makeSelfContained() -- embed every file-based IC in the circuit
                        // at once (mirrors pushButtonMakeSelfContained/actionMakeSelfContained).
                        Button {
                            Layout.fillWidth: true
                            text: qsTr("Make Self-Contained")
                            visible: AppController.icButtonsVisible
                            onClicked: AppController.makeSelfContained()
                        }
                    }

                    ListView { clip: true; model: root.controller.miscElements; delegate: paletteItemComponent; ScrollBar.vertical: ThickScrollBar { policy: ScrollBar.AlwaysOn } }
                    ListView { clip: true; model: root.controller.searchResults; delegate: paletteItemComponent; ScrollBar.vertical: ThickScrollBar { policy: ScrollBar.AlwaysOn } }
                }
            }
        }
    }
}
