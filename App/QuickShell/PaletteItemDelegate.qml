// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// One palette grid entry: drag-to-canvas or double-click-to-add. Extracted from
// ElementPalette.qml's paletteItemComponent into its own named type so Main.qml's
// DropArea.onDropped can cast drop.source to a real, qmllint-known type instead of
// treating it as a plain QObject (DragEvent.source's declared type) with an unresolvable
// modelData member.
Item {
    id: delegateRoot
    required property var modelData
    // Canvas-area size, for centering a newly-added element (double-click below) --
    // bound explicitly by ElementPalette.qml's Component wrapper, mirroring how
    // ElementPalette itself receives it from Main.qml (see ElementPalette.qml).
    property real canvasWidth: 0
    property real canvasHeight: 0
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
            AppController.addElementToCurrentTab(
                delegateRoot.modelData.type, delegateRoot.modelData.icFileName,
                delegateRoot.modelData.isEmbedded, delegateRoot.canvasWidth / 2, delegateRoot.canvasHeight / 2)
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
