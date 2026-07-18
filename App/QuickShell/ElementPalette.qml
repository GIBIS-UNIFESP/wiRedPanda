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
// confirmed by reading MainWindowUI.cpp's layout construction). Add/remove-embedded-IC
// buttons (pushButtonAddEmbeddedIC/pushButtonRemoveEmbeddedIC in production) are not here --
// that's ICController/ICDropZone/TrashButton, deferred to sub-step 7.
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

        TabBar {
            id: categoryTabBar
            Layout.fillWidth: true
            visible: searchField.text.length === 0

            TabButton { text: qsTr("I/O") }
            TabButton { text: qsTr("Gates") }
            TabButton { text: qsTr("Combinational") }
            TabButton { text: qsTr("Memory") }
            TabButton { text: qsTr("IC") }
            TabButton { text: qsTr("Misc") }
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
                GridView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.max(78, contentHeight)
                    cellWidth: 78; cellHeight: 78; clip: true
                    model: root.controller.icElements
                    delegate: paletteItemComponent
                }
                Label { text: qsTr("Embedded"); leftPadding: 4 }
                GridView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    cellWidth: 78; cellHeight: 78; clip: true
                    model: root.controller.embeddedICElements
                    delegate: paletteItemComponent
                }
            }

            GridView { cellWidth: 78; cellHeight: 78; clip: true; model: root.controller.miscElements; delegate: paletteItemComponent }
            GridView { cellWidth: 78; cellHeight: 78; clip: true; model: root.controller.searchResults; delegate: paletteItemComponent }
        }
    }
}
