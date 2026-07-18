// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// Lets paletteItemComponent's PaletteItemDelegate {} reference this file's root id (see
// canvasWidth/canvasHeight below) in a way qmllint can verify statically -- the pragma
// binds inner Components to their defining context; modelData already flows in via a
// required property, not a context property, so this loses nothing this file relies on.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// Phase 4 sub-step 4: element palette panel, backed by App/QuickShell/Chrome/
// QuickElementPalette (AppController.elementPalette). Every category grid and the search
// grid share one delegate (paletteItemComponent) so drag-to-canvas and double-click-to-add
// behave identically everywhere an element can be picked from -- mirrors ElementPalette::
// populateMenu()'s "every entry gets the same ElementLabel behavior" invariant.
//
// The IC tab holds two sections (file-based ICs, embedded ICs) in one scrollable column,
// mirroring MainWindowUi's single "ic" tab widget (scrollAreaWidgetContents_IC and
// scrollAreaWidgetContents_EmbeddedIC are both children of the same tab, not separate tabs --
// confirmed by reading MainWindowUI.cpp's layout construction). Each section is wrapped in a
// DropArea (ICDropZone port) so dragging the opposite type onto it embeds/extracts; a trash
// drop target below both (TrashButton port) removes whichever is dropped on it -- all three
// wired to QuickICController via AppController (Phase 4 sub-step 7). Add/remove-embedded-IC
// *buttons* (pushButtonAddEmbeddedIC/pushButtonRemoveEmbeddedIC in production -- a different,
// toolbar-driven way to trigger the same operations) are still not here: they need
// ICController methods (addEmbeddedICFromFile()/embedSelectedIC()/etc.) this pass
// deliberately didn't port, since they need their own toolbar-button UI trigger that doesn't
// exist yet -- see QuickICController's own doc comment for the full list.
Item {
    id: root
    implicitWidth: 220

    readonly property QuickElementPalette controller: AppController.elementPalette
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

        TextField {
            id: searchField
            Layout.fillWidth: true
            Layout.margins: 4
            placeholderText: qsTr("Search elements")
            onTextChanged: root.controller.searchText = text

            Keys.onReturnPressed: {
                const result = root.controller.firstSearchResult();
                if (result.type !== undefined) {
                    AppController.addElementToCurrentTab(
                        result.type, result.icFileName, result.isEmbedded,
                        root.canvasWidth / 2, root.canvasHeight / 2);
                    searchField.text = "";
                }
            }
        }

        // Icon-only tabs with tooltips, matching MainWindowUI.cpp's tabElements exactly
        // (tabElements->addTab(..., QIcon(...), QString()) -- empty text, real name as a
        // tooltip) -- not a truncated-text sizing bug to fix, a different (unintentional)
        // design Quick had never matched to begin with.
        TabBar {
            id: categoryTabBar
            Layout.fillWidth: true
            visible: searchField.text.length === 0

            TabButton { icon.source: "qrc:/Components/Input/buttonOff.svg"; ToolTip.text: qsTr("Inputs/Outputs"); ToolTip.visible: hovered }
            TabButton { icon.source: "qrc:/Components/Logic/xor.svg"; ToolTip.text: qsTr("Gates"); ToolTip.visible: hovered }
            TabButton { icon.source: "qrc:/Components/Logic/truthtable-rotated.svg"; ToolTip.text: qsTr("Combinational"); ToolTip.visible: hovered }
            TabButton { icon.source: "qrc:/Components/Memory/Light/D-flipflop.svg"; ToolTip.text: qsTr("Memory"); ToolTip.visible: hovered }
            TabButton { icon.source: "qrc:/Components/Logic/ic-panda.svg"; ToolTip.text: qsTr("Integrated Circuits"); ToolTip.visible: hovered }
            TabButton { icon.source: "qrc:/Components/Misc/text.png"; ToolTip.text: qsTr("Miscellaneous"); ToolTip.visible: hovered }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: searchField.text.length > 0 ? 6 : categoryTabBar.currentIndex

            GridView { cellWidth: 78; cellHeight: 78; clip: true; model: root.controller.ioElements; delegate: paletteItemComponent }
            GridView { cellWidth: 78; cellHeight: 78; clip: true; model: root.controller.gatesElements; delegate: paletteItemComponent }
            GridView { cellWidth: 78; cellHeight: 78; clip: true; model: root.controller.combinationalElements; delegate: paletteItemComponent }
            GridView { cellWidth: 78; cellHeight: 78; clip: true; model: root.controller.memoryElements; delegate: paletteItemComponent }

            ColumnLayout {
                Label { text: qsTr("Files"); leftPadding: 4 }
                // ICDropZone port (Section::FileBased): dropping an embedded IC label here
                // extracts it to a file. drag.source is only meaningful while containsDrag is
                // true (no drag in progress otherwise), hence the "as PaletteItemDelegate" cast
                // + null check rather than trusting it unconditionally.
                Item {
                    id: fileBasedZone
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.max(78, filesGrid.contentHeight)

                    GridView {
                        id: filesGrid
                        anchors.fill: parent
                        cellWidth: 78; cellHeight: 78; clip: true
                        model: root.controller.icElements
                        delegate: paletteItemComponent
                    }

                    DropArea {
                        id: fileBasedDropArea
                        anchors.fill: parent
                        readonly property PaletteItemDelegate dragged: drag.source as PaletteItemDelegate
                        onDropped: (drop) => {
                            const item = drop.source as PaletteItemDelegate;
                            if (item && item.modelData.isEmbedded) {
                                AppController.extractICByBlobName(item.modelData.icFileName);
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

                Label { text: qsTr("Embedded"); leftPadding: 4 }
                // ICDropZone port (Section::Embedded): dropping a file-based IC label here
                // embeds it.
                Item {
                    id: embeddedZone
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    GridView {
                        id: embeddedGrid
                        anchors.fill: parent
                        cellWidth: 78; cellHeight: 78; clip: true
                        model: root.controller.embeddedICElements
                        delegate: paletteItemComponent
                    }

                    DropArea {
                        id: embeddedDropArea
                        anchors.fill: parent
                        readonly property PaletteItemDelegate dragged: drag.source as PaletteItemDelegate
                        onDropped: (drop) => {
                            const item = drop.source as PaletteItemDelegate;
                            if (item && !item.modelData.isEmbedded) {
                                AppController.embedICByFile(item.modelData.icFileName);
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

                    Rectangle {
                        anchors.fill: parent
                        radius: 4
                        color: trashDropArea.containsDrag ? "#cc3333" : "transparent"
                        border.color: trashZone.palette.mid
                        border.width: 1
                    }

                    Label {
                        anchors.centerIn: parent
                        text: qsTr("Drag here to remove")
                    }

                    DropArea {
                        id: trashDropArea
                        anchors.fill: parent
                        onDropped: (drop) => {
                            const item = drop.source as PaletteItemDelegate;
                            if (!item) {
                                return;
                            }
                            if (item.modelData.isEmbedded) {
                                AppController.removeEmbeddedIC(item.modelData.icFileName);
                            } else {
                                AppController.removeICFile(item.modelData.icFileName);
                            }
                        }
                    }
                }
            }

            GridView { cellWidth: 78; cellHeight: 78; clip: true; model: root.controller.miscElements; delegate: paletteItemComponent }
            GridView { cellWidth: 78; cellHeight: 78; clip: true; model: root.controller.searchResults; delegate: paletteItemComponent }
        }
    }
}
