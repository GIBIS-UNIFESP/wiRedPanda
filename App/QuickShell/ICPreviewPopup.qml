// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls

import QuickShell

// IC hover-preview popup, driven entirely by AppController.icPreview (App/QuickShell/Chrome/
// QuickICPreview) -- see that class's own doc comment for the show/hide-delay state machine
// this file displays. A plain Item, not a Popup/Dialog: it needs to render above the canvas
// without modal/dismiss-on-click-outside semantics, closer to a native tooltip than a dialog.
// Instantiated once in Main.qml alongside elementContextMenu/emptyContextMenu/
// inlineLabelEditor; needs no Connections block wired to CanvasItem itself since every property
// read below is a direct, live binding to AppController.icPreview -- QuickICPreview already
// owns that Connections-equivalent logic in C++ via setCanvas().
Item {
    id: root

    readonly property QuickICPreview preview: AppController.icPreview
    // root.Window.window (not root.Window.contentItem -- Qt Quick Controls' own docs call this
    // exact distinction out: "The ApplicationWindow content item is different from the Window
    // content item, although both are accessible as contentItem") deliberately goes through the
    // real ApplicationWindow instance so ".contentItem" resolves dynamically, by name, to
    // QQuickApplicationWindow's own override -- the area below the menu bar that this popup is
    // actually auto-parented into (declared directly inside ApplicationWindow in Main.qml, same
    // as elementContextMenu's canvasHost siblings). The plain Window attached-property's own
    // contentItem is the whole window's root scenegraph item instead, offset by the menu bar's
    // height from where this popup lives -- the wrong coordinate space to map into.
    readonly property Item contentArea: root.Window.window ? root.Window.window.contentItem : null

    // Explicit reparenting (the same idiom Main.qml's own canvasHost.showCurrentTab() uses for
    // CanvasItem, and that Popup/Menu use internally for Overlay.overlay) rather than relying on
    // wherever this component happens to be declared in Main.qml's tree: this item's x/y are set
    // in contentArea-local coordinates below, so its real QQuickItem parent must be contentArea
    // itself, not whatever Item structurally contains its QML declaration (e.g. canvasHost,
    // which is itself offset from contentArea by ElementPalette's width).
    parent: contentArea

    visible: preview.visible
    z: 2000
    width: background.width
    height: background.height

    // preview.screenPos is a *global* screen position (mirroring ICPreviewPopup's own
    // screen-space QWidget positioning) -- contentArea.mapFromGlobal() converts it into this
    // item's own parent coordinate space before clamping to the content area's bounds, the
    // window-space analog of ICPreviewPopup::clampedPopupPos()'s screen-bounds clamp.
    function reposition() {
        if (!visible || !contentArea) {
            return;
        }
        const local = contentArea.mapFromGlobal(preview.screenPos);
        const maxX = Math.max(0, contentArea.width - width);
        const maxY = Math.max(0, contentArea.height - height);
        x = Math.min(Math.max(0, local.x + 16), maxX);
        y = Math.min(Math.max(0, local.y + 16), maxY);
    }

    onVisibleChanged: reposition()
    onWidthChanged: reposition()
    onHeightChanged: reposition()

    Connections {
        target: root.preview
        function onRefreshed() { root.reposition(); }
    }

    Rectangle {
        id: background
        width: content.implicitWidth + 16
        height: content.implicitHeight + 16
        color: root.palette.toolTipBase
        border.color: root.palette.mid

        Column {
            id: content
            x: 8
            y: 8
            spacing: 4

            Label {
                text: root.preview.title
                visible: text.length > 0
                color: root.palette.toolTipText
                font.bold: true
            }

            Image {
                source: root.preview.imageUrl
                visible: source.toString().length > 0
                fillMode: Image.PreserveAspectFit
            }
        }
    }
}
