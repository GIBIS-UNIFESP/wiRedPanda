// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Phase 4 sub-step 4: element palette panel, backed by App/QuickShell/Chrome/
// QuickElementPalette (appController.elementPalette). Every category grid and the search
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

    readonly property QtObject controller: appController.elementPalette

    Component {
        id: paletteItemComponent

        Item {
            id: delegateRoot
            required property var modelData
            width: 76
            height: 76

            Drag.active: dragArea.drag.active
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 2
                spacing: 2

                Image {
                    source: delegateRoot.modelData.iconSource
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 44
                    Layout.preferredHeight: 44
                    fillMode: Image.PreserveAspectFit
                }

                Label {
                    Layout.fillWidth: true
                    text: delegateRoot.modelData.name
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: 9
                    elide: Text.ElideRight
                }
            }

            MouseArea {
                id: dragArea
                anchors.fill: parent
                drag.target: delegateRoot
                hoverEnabled: true
                ToolTip.visible: containsMouse && !dragArea.drag.active
                ToolTip.text: delegateRoot.modelData.tooltip
                ToolTip.delay: 500

                // Drag-free shortcut, mirroring ElementLabel::mouseDoubleClickEvent(). Lands
                // at the centre of the visible canvas -- CanvasItem has no pan/zoom transform
                // yet, so canvas-local coordinates are just canvasHost-local coordinates.
                onDoubleClicked: {
                    appController.addElementToCurrentTab(
                        delegateRoot.modelData.type, delegateRoot.modelData.icFileName,
                        delegateRoot.modelData.isEmbedded, canvasHost.width / 2, canvasHost.height / 2)
                }

                onReleased: {
                    if (delegateRoot.Drag.active) {
                        delegateRoot.Drag.drop()
                    }
                    // Snap back to the grid cell -- this delegate is the real dragged item
                    // (no separate drag-ghost), so it must return to its layout position
                    // rather than staying wherever the drag left it.
                    delegateRoot.x = 0
                    delegateRoot.y = 0
                }
            }
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
                const result = root.controller.firstSearchResult()
                if (result.type !== undefined) {
                    appController.addElementToCurrentTab(
                        result.type, result.icFileName, result.isEmbedded,
                        canvasHost.width / 2, canvasHost.height / 2)
                    searchField.text = ""
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
