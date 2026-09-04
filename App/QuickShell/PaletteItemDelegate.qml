// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// One palette entry: icon + name, draggable onto the canvas or double-click-to-add. Mirrors
// ElementLabel's own QFrame layout exactly (QHBoxLayout: icon, stretch, name, stretch --
// ElementLabel.cpp's constructor): production shows one entry per row, icon at a fixed 64x64
// on the left and the name roughly centered in the remaining width, not a multi-column grid.
// Extracted out of ElementPalette.qml's delegate Component into its own file so Main.qml's
// DropArea can see a named type on drop.source instead of DragEvent's plain QObject --
// canvasWidth/canvasHeight are passed down explicitly by whatever instantiates this (a
// ListView's delegate context, ultimately from ElementPalette.qml's own root), never reached
// for via an implicit ancestor-id lookup across a file boundary.
Item {
    id: delegateRoot

    required property var modelData
    property real canvasWidth: 0
    property real canvasHeight: 0

    width: ListView.view ? ListView.view.width : 76
    height: 76

    RowLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 0

        Image {
            source: delegateRoot.modelData.iconSource
            Layout.preferredWidth: 64
            Layout.preferredHeight: 64
            fillMode: Image.PreserveAspectFit
        }

        Item { Layout.fillWidth: true }

        Label {
            text: delegateRoot.modelData.name
            elide: Text.ElideRight
        }

        Item { Layout.fillWidth: true }
    }

    // Drag ghost: a free-floating copy of just the icon -- mirroring ElementLabel::startDrag()'s
    // QDrag::setPixmap(pixmap()), which drags only the icon pixmap, never the whole row -- that
    // follows the cursor while dragging, so this delegate itself (and its place in the ListView)
    // never visibly moves. Reparented to the window's content item (same idiom as
    // ICPreviewPopup.qml's own root.parent) because every ListView hosting this delegate sets
    // clip: true, which would otherwise cut the ghost off the moment it left this delegate's
    // own small bounds.
    Image {
        id: dragGhost
        parent: delegateRoot.Window.window ? delegateRoot.Window.window.contentItem : delegateRoot
        source: delegateRoot.modelData.iconSource
        width: 64
        height: 64
        visible: dragArea.drag.active
        z: 2001 // above ICPreviewPopup.qml's z: 2000

        Drag.active: dragArea.drag.active
        // Explicitly delegateRoot, not this ghost (the default) -- every "drop.source as
        // PaletteItemDelegate" cast across ElementPalette.qml/Main.qml needs a real
        // PaletteItemDelegate with a usable modelData, not this plain Image.
        Drag.source: delegateRoot
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2
    }

    MouseArea {
        id: dragArea
        anchors.fill: parent
        drag.target: dragGhost
        hoverEnabled: true
        ToolTip.visible: containsMouse && !dragArea.drag.active
        ToolTip.text: delegateRoot.modelData.tooltip
        ToolTip.delay: 500

        // dragGhost's parent is a different coordinate space than this MouseArea's (the
        // window's content item vs. this delegate) -- without this, drag.target's first moved
        // position would be computed relative to wherever dragGhost was last left (its initial
        // (0, 0), or a previous drag's end point), making the ghost jump there instead of
        // starting under the cursor. Centring it on the press point (rather than preserving
        // whatever offset within the icon was actually clicked) mirrors QDrag's own fixed,
        // icon-centre hotspot in ElementLabel::startDrag().
        onPressed: (mouse) => {
            const point = delegateRoot.mapToItem(dragGhost.parent, mouse.x, mouse.y)
            dragGhost.x = point.x - dragGhost.width / 2
            dragGhost.y = point.y - dragGhost.height / 2
        }

        // Drag-free shortcut, mirroring ElementLabel::mouseDoubleClickEvent(). Lands at the
        // centre of the visible canvas -- CanvasItem has no pan/zoom transform yet, so
        // canvas-local coordinates are just canvasHost-local coordinates.
        onDoubleClicked: {
            AppController.addElementToCurrentTab(
                delegateRoot.modelData.type, delegateRoot.modelData.icFileName,
                delegateRoot.modelData.isEmbedded, delegateRoot.canvasWidth / 2, delegateRoot.canvasHeight / 2)
        }

        onReleased: {
            if (dragGhost.Drag.active) {
                dragGhost.Drag.drop()
            }
        }
    }
}
