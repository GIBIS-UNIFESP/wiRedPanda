// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// Lets the grip-dot Repeater's delegate below reference this file's root id -- same reason
// ElementPalette.qml sets this (see its own comment).
pragma ComponentBehavior: Bound

import QtQuick

import QuickShell

// Circuit-overview minimap, driven by AppController.minimap (App/QuickShell/Chrome/
// QuickMinimap) -- see that class's own doc comment for what it computes (thumbnail image,
// viewport-rect overlay, content aspect ratio, click/drag-to-navigate) vs. what this file owns
// (resize/move interaction, background/border chrome, geometry persistence timing). Positioned
// in canvasHost-local coordinates -- declared directly as a canvasHost child in Main.qml, no
// reparenting needed (unlike ICPreviewPopup.qml, whose screenPos is a global coordinate).
//
// Resize (8 edge/corner handles, aspect-locked to the circuit's own bounding-box ratio) and
// move (a top strip) are QML-native MouseArea drag math, not a port of MinimapWidget's
// ResizeMode/moveHandleRect() C++ machinery -- the underlying math (dominant-axis diagonal
// resize, anchor-preserving edge math, min-size clamping) mirrors MinimapWidget::applyResize()
// exactly, just written in QML/JS since this is interactive UI chrome, not domain logic.
// Decorative affordances are simplified from production's hand-drawn grip-dot/arm overlays to
// cursor-shape-only feedback plus a light move-strip tint -- a deliberate, documented
// simplification (the cursor change alone gives the same "grab here" discoverability), not a
// functional gap.
Item {
    id: root

    readonly property QuickMinimap presenter: AppController.minimap
    readonly property int margin: 12
    readonly property int minW: 160
    readonly property int minH: 120
    readonly property int handleSize: 8
    readonly property int moveHandleHeight: 22

    visible: presenter.visible
    z: 1500

    width: 220
    height: 160
    x: parent ? parent.width - width - margin : 0
    y: parent ? parent.height - height - margin : 0

    // Reported to QuickMinimap so it knows what pixel size to render its thumbnail at --
    // mirrors ElementPalette.qml's canvasWidth/canvasHeight-reported-by-Main.qml precedent
    // (QML owns the actual on-screen geometry; C++ needs to be told what it is).
    onWidthChanged: presenter.minimapWidth = width
    onHeightChanged: presenter.minimapHeight = height
    Component.onCompleted: {
        presenter.minimapWidth = width;
        presenter.minimapHeight = height;

        // One-time restore of the persisted geometry, mirroring
        // WorkSpace::applyMinimapGeometry()'s m_minimapPositioned branch -- a never-set
        // geometry (width/height both 0) keeps this item's own QML-declared default
        // (bottom-right anchor, 220x160) instead.
        const restored = presenter.initialGeometry();
        if (restored.width > 0 && restored.height > 0) {
            width = restored.width;
            height = restored.height;
            x = restored.x;
            y = restored.y;
        }
        clampToParent();
    }

    // Re-clamp into the new bounds whenever canvasHost itself resizes (e.g. the main window
    // resizes) -- mirrors WorkSpace::applyMinimapGeometry()'s "subsequent resizes" branch,
    // which deliberately re-clamps the minimap's own current geometry rather than re-reading
    // Settings (only user-driven moves/resizes persist there).
    Connections {
        target: root.parent
        function onWidthChanged() { root.clampToParent(); }
        function onHeightChanged() { root.clampToParent(); }
    }

    function clampToParent() {
        if (!parent) {
            return;
        }
        const maxWidth = Math.max(minW, parent.width - 2 * margin);
        const maxHeight = Math.max(minH, parent.height - 2 * margin);
        width = Math.min(Math.max(minW, width), maxWidth);
        height = Math.min(Math.max(minH, height), maxHeight);
        x = Math.min(Math.max(margin, x), Math.max(margin, parent.width - width - margin));
        y = Math.min(Math.max(margin, y), Math.max(margin, parent.height - height - margin));
    }

    // Aspect-locked resize, mirroring MinimapWidget::applyResize() exactly: a diagonal handle
    // follows whichever axis moved further, an edge handle derives the other axis from the
    // content's own aspect ratio, and the fixed (non-dragged) edge is re-anchored from this
    // item's own *current* x/y/width/height each call -- the same "read live geometry every
    // move event, not a drag-start snapshot" approach applyResize() itself uses.
    function performResize(top, bottom, left, right, deltaX, deltaY) {
        const aspect = presenter.contentAspectRatio();
        let newWidth = width;
        let newHeight = height;

        if ((top || bottom) && (left || right)) {
            if (Math.abs(deltaX) >= Math.abs(deltaY)) {
                newWidth = left ? width - deltaX : width + deltaX;
                newHeight = newWidth / aspect;
            } else {
                newHeight = top ? height - deltaY : height + deltaY;
                newWidth = newHeight * aspect;
            }
        } else if (top || bottom) {
            newHeight = top ? height - deltaY : height + deltaY;
            newWidth = newHeight * aspect;
        } else if (left || right) {
            newWidth = left ? width - deltaX : width + deltaX;
            newHeight = newWidth / aspect;
        }

        newWidth = Math.max(minW, newWidth);
        newHeight = Math.max(minH, newHeight);

        const oldRight = x + width;
        const oldBottom = y + height;
        width = newWidth;
        height = newHeight;
        if (left) {
            x = oldRight - newWidth;
        }
        if (top) {
            y = oldBottom - newHeight;
        }
        clampToParent();
    }

    Rectangle {
        anchors.fill: parent
        radius: 4
        color: root.palette.window
        border.color: root.palette.mid
        border.width: 1
    }

    Image {
        anchors.fill: parent
        anchors.margins: 1
        source: root.presenter.imageUrl
        // renderMinimapImage() already renders at exactly this item's own pixel size (its
        // source rect is pre-grown to match this target's aspect ratio) -- no letterboxing to
        // preserve, unlike a general-purpose PreserveAspectFit image.
        fillMode: Image.Stretch
        cache: false
    }

    Rectangle {
        x: root.presenter.viewportRect.x
        y: root.presenter.viewportRect.y
        width: root.presenter.viewportRect.width
        height: root.presenter.viewportRect.height
        color: "transparent"
        border.color: root.palette.highlight
        border.width: 2
    }

    // Click/drag-to-navigate: mirrors MinimapWidget::mousePressEvent()/mouseMoveEvent()'s
    // fallback ("neither a resize nor move handle") branch. Declared before (so painted/
    // hit-tested beneath) the move-handle and resize-handle MouseAreas below, so their smaller,
    // more specific areas take priority over this one, matching MinimapWidget::
    // mousePressEvent()'s own resize-then-move-then-navigate precedence.
    MouseArea {
        anchors.fill: parent
        onPressed: (mouse) => root.presenter.navigateTo(mouse.x, mouse.y)
        onPositionChanged: (mouse) => { if (pressed) root.presenter.navigateTo(mouse.x, mouse.y); }
    }

    // Move handle: a strip along the top, dragged to reposition the whole minimap freely --
    // mirrors MinimapWidget::moveHandleRect()/moveBy().
    MouseArea {
        id: moveHandle
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: root.moveHandleHeight
        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
        property point lastGlobal

        // Mirrors MinimapWidget::drawMoveHandle() exactly: a real, non-zero idle-state fill
        // (bg.setAlpha(35) there == 35/255 here), not hover-only -- production's strip is
        // always visible, just recolored/brightened on hover/drag. The earlier idle-opacity-0
        // version (found via direct screenshot comparison against the real app) is why the
        // grip dots below used to look like they were floating above the border with nothing
        // to sit inside.
        readonly property bool highlighted: moveHandle.pressed || moveHandle.containsMouse
        Rectangle {
            anchors.fill: parent
            color: moveHandle.highlighted ? root.palette.highlight : root.palette.windowText
            opacity: moveHandle.highlighted ? 0.35 : 0.14 // 90/255 : 35/255
        }
        Row {
            anchors.centerIn: parent
            spacing: 4
            Repeater {
                model: 3
                Rectangle {
                    width: 3; height: 3; radius: 1.5
                    color: root.palette.windowText
                    opacity: 0.6
                }
            }
        }

        onPressed: (mouse) => { lastGlobal = mapToItem(root.parent, mouse.x, mouse.y); }
        onPositionChanged: (mouse) => {
            if (!pressed) {
                return;
            }
            const current = mapToItem(root.parent, mouse.x, mouse.y);
            root.x += current.x - lastGlobal.x;
            root.y += current.y - lastGlobal.y;
            lastGlobal = current;
            root.clampToParent();
        }
        onReleased: root.presenter.commitGeometry(root.x, root.y, root.width, root.height)
    }

    // 8 resize handles (edges + corners), each a thin MouseArea along the item's own border --
    // mirrors MinimapWidget::resizeModeAt()'s handleSize-based hit zones and
    // cursorForResizeMode()'s cursor-per-handle mapping exactly.
    MouseArea {
        id: resizeN
        anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right
        height: root.handleSize
        cursorShape: Qt.SizeVerCursor
        property point lastGlobal
        onPressed: (mouse) => { lastGlobal = mapToItem(root.parent, mouse.x, mouse.y); }
        onPositionChanged: (mouse) => {
            if (!pressed) return;
            const current = mapToItem(root.parent, mouse.x, mouse.y);
            root.performResize(true, false, false, false, current.x - lastGlobal.x, current.y - lastGlobal.y);
            lastGlobal = current;
        }
        onReleased: root.presenter.commitGeometry(root.x, root.y, root.width, root.height)
    }
    MouseArea {
        id: resizeS
        anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.right: parent.right
        height: root.handleSize
        cursorShape: Qt.SizeVerCursor
        property point lastGlobal
        onPressed: (mouse) => { lastGlobal = mapToItem(root.parent, mouse.x, mouse.y); }
        onPositionChanged: (mouse) => {
            if (!pressed) return;
            const current = mapToItem(root.parent, mouse.x, mouse.y);
            root.performResize(false, true, false, false, current.x - lastGlobal.x, current.y - lastGlobal.y);
            lastGlobal = current;
        }
        onReleased: root.presenter.commitGeometry(root.x, root.y, root.width, root.height)
    }
    MouseArea {
        id: resizeW
        anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom
        width: root.handleSize
        cursorShape: Qt.SizeHorCursor
        property point lastGlobal
        onPressed: (mouse) => { lastGlobal = mapToItem(root.parent, mouse.x, mouse.y); }
        onPositionChanged: (mouse) => {
            if (!pressed) return;
            const current = mapToItem(root.parent, mouse.x, mouse.y);
            root.performResize(false, false, true, false, current.x - lastGlobal.x, current.y - lastGlobal.y);
            lastGlobal = current;
        }
        onReleased: root.presenter.commitGeometry(root.x, root.y, root.width, root.height)
    }
    MouseArea {
        id: resizeE
        anchors.right: parent.right; anchors.top: parent.top; anchors.bottom: parent.bottom
        width: root.handleSize
        cursorShape: Qt.SizeHorCursor
        property point lastGlobal
        onPressed: (mouse) => { lastGlobal = mapToItem(root.parent, mouse.x, mouse.y); }
        onPositionChanged: (mouse) => {
            if (!pressed) return;
            const current = mapToItem(root.parent, mouse.x, mouse.y);
            root.performResize(false, false, false, true, current.x - lastGlobal.x, current.y - lastGlobal.y);
            lastGlobal = current;
        }
        onReleased: root.presenter.commitGeometry(root.x, root.y, root.width, root.height)
    }
    // Corner tick marks (an "L" of two thin rectangles per corner), restored to match
    // MinimapWidget::drawResizeGrips() -- production always shows these (alpha 130/255 idle,
    // 230/255 highlighted), not cursor-feedback-only; found worth restoring alongside the
    // move-handle strip fix above, since both are the same "production keeps a visible
    // idle-state affordance" pattern (armLength/inset match drawResizeGrips() exactly).
    MouseArea {
        id: resizeNW
        anchors.top: parent.top; anchors.left: parent.left
        width: root.handleSize * 2; height: root.handleSize * 2
        cursorShape: Qt.SizeFDiagCursor
        property point lastGlobal
        readonly property bool highlighted: pressed || containsMouse
        readonly property color tickColor: highlighted ? root.palette.highlight : root.palette.windowText
        Rectangle { x: 3; y: 3; width: 9; height: 1.4; color: resizeNW.tickColor; opacity: resizeNW.highlighted ? 0.9 : 0.51 }
        Rectangle { x: 3; y: 3; width: 1.4; height: 9; color: resizeNW.tickColor; opacity: resizeNW.highlighted ? 0.9 : 0.51 }
        onPressed: (mouse) => { lastGlobal = mapToItem(root.parent, mouse.x, mouse.y); }
        onPositionChanged: (mouse) => {
            if (!pressed) return;
            const current = mapToItem(root.parent, mouse.x, mouse.y);
            root.performResize(true, false, true, false, current.x - lastGlobal.x, current.y - lastGlobal.y);
            lastGlobal = current;
        }
        onReleased: root.presenter.commitGeometry(root.x, root.y, root.width, root.height)
    }
    MouseArea {
        id: resizeSE
        anchors.bottom: parent.bottom; anchors.right: parent.right
        width: root.handleSize * 2; height: root.handleSize * 2
        cursorShape: Qt.SizeFDiagCursor
        property point lastGlobal
        readonly property bool highlighted: pressed || containsMouse
        readonly property color tickColor: highlighted ? root.palette.highlight : root.palette.windowText
        Rectangle { x: width - 3 - 9; y: height - 3 - 1.4; width: 9; height: 1.4; color: resizeSE.tickColor; opacity: resizeSE.highlighted ? 0.9 : 0.51 }
        Rectangle { x: width - 3 - 1.4; y: height - 3 - 9; width: 1.4; height: 9; color: resizeSE.tickColor; opacity: resizeSE.highlighted ? 0.9 : 0.51 }
        onPressed: (mouse) => { lastGlobal = mapToItem(root.parent, mouse.x, mouse.y); }
        onPositionChanged: (mouse) => {
            if (!pressed) return;
            const current = mapToItem(root.parent, mouse.x, mouse.y);
            root.performResize(false, true, false, true, current.x - lastGlobal.x, current.y - lastGlobal.y);
            lastGlobal = current;
        }
        onReleased: root.presenter.commitGeometry(root.x, root.y, root.width, root.height)
    }
    MouseArea {
        id: resizeNE
        anchors.top: parent.top; anchors.right: parent.right
        width: root.handleSize * 2; height: root.handleSize * 2
        cursorShape: Qt.SizeBDiagCursor
        property point lastGlobal
        readonly property bool highlighted: pressed || containsMouse
        readonly property color tickColor: highlighted ? root.palette.highlight : root.palette.windowText
        Rectangle { x: width - 3 - 9; y: 3; width: 9; height: 1.4; color: resizeNE.tickColor; opacity: resizeNE.highlighted ? 0.9 : 0.51 }
        Rectangle { x: width - 3 - 1.4; y: 3; width: 1.4; height: 9; color: resizeNE.tickColor; opacity: resizeNE.highlighted ? 0.9 : 0.51 }
        onPressed: (mouse) => { lastGlobal = mapToItem(root.parent, mouse.x, mouse.y); }
        onPositionChanged: (mouse) => {
            if (!pressed) return;
            const current = mapToItem(root.parent, mouse.x, mouse.y);
            root.performResize(true, false, false, true, current.x - lastGlobal.x, current.y - lastGlobal.y);
            lastGlobal = current;
        }
        onReleased: root.presenter.commitGeometry(root.x, root.y, root.width, root.height)
    }
    MouseArea {
        id: resizeSW
        anchors.bottom: parent.bottom; anchors.left: parent.left
        width: root.handleSize * 2; height: root.handleSize * 2
        cursorShape: Qt.SizeBDiagCursor
        property point lastGlobal
        readonly property bool highlighted: pressed || containsMouse
        readonly property color tickColor: highlighted ? root.palette.highlight : root.palette.windowText
        Rectangle { x: 3; y: height - 3 - 1.4; width: 9; height: 1.4; color: resizeSW.tickColor; opacity: resizeSW.highlighted ? 0.9 : 0.51 }
        Rectangle { x: 3; y: height - 3 - 9; width: 1.4; height: 9; color: resizeSW.tickColor; opacity: resizeSW.highlighted ? 0.9 : 0.51 }
        onPressed: (mouse) => { lastGlobal = mapToItem(root.parent, mouse.x, mouse.y); }
        onPositionChanged: (mouse) => {
            if (!pressed) return;
            const current = mapToItem(root.parent, mouse.x, mouse.y);
            root.performResize(false, true, true, false, current.x - lastGlobal.x, current.y - lastGlobal.y);
            lastGlobal = current;
        }
        onReleased: root.presenter.commitGeometry(root.x, root.y, root.width, root.height)
    }
}
