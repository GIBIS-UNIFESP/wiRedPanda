// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// Lets the grip-dot Repeater's delegate below reference this file's root id -- same reason
// ElementPalette.qml sets this (see its own comment).
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

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
    readonly property int moveHandleHeight: 24 // MinimapWidget::moveHandleRect()'s QRect(0, 0, width(), 24)

    visible: presenter.visible
    z: 1500

    // Mirrors MinimapWidget.cpp's setToolTip()/setAccessibleName()/setWhatsThis() calls exactly
    // -- shown everywhere on the widget, with no per-region suppression. A plain
    // navigateArea.containsMouse binding does NOT achieve that: Qt Quick's hover delivery stops
    // at the first (topmost) MouseArea sibling under the pointer, so navigateArea (declared
    // first, painted/hit-tested underneath moveHandle and the 8 resize handles) never reports
    // containsMouse true while the cursor is over any of those overlapping regions -- verified
    // empirically (a standalone reproduction: two overlapping sibling MouseAreas with
    // hoverEnabled: true, the bottom one's containsMouse stays false under the top one's area).
    // A HoverHandler is a distinct (Pointer Handler) delivery mechanism that isn't subject to
    // that same occlusion -- confirmed in the same reproduction that a HoverHandler sibling of
    // both MouseAreas correctly reports hovered regardless of which MouseArea is on top -- so
    // it's used here instead, covering the item's full bounds independently of the resize/move
    // MouseAreas layered on top of it.
    HoverHandler {
        id: tooltipHoverHandler
    }
    ToolTip.text: qsTr("Mini-map: click or drag to navigate")
    ToolTip.visible: tooltipHoverHandler.hovered
    Accessible.name: qsTr("Circuit minimap")
    Accessible.description: qsTr("A miniature overview of the whole circuit. Click or drag inside it to jump the main canvas to that location.")

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
        restoreTimer.start();
    }

    // One-time restore of the persisted geometry, mirroring WorkSpace::applyMinimapGeometry()'s
    // m_minimapPositioned branch -- deliberately NOT done synchronously inside
    // Component.onCompleted above: QML completes objects bottom-up, so this item's own
    // Component.onCompleted runs before the ApplicationWindow's (Main.qml's `window`), which is
    // what actually applies the persisted WINDOW geometry via AppController.restoreWindowGeometry()
    // -- and canvasHost's own width/height (resolved through several layers of Layout/SplitView
    // bindings) aren't necessarily final yet either. Restoring-then-clampToParent() immediately
    // against that transient, possibly-too-small parent silently shrank/repositioned the
    // persisted geometry on every launch: clampToParent()'s later re-clamps (on canvasHost's own
    // resize once the real window geometry lands) only shrink further, they never recover the
    // original persisted value once it's been clobbered by a bad early clamp. This mirrors
    // WorkSpace.cpp's own identical defense for the identical race (a QTimer::singleShot(100,
    // ...) backstop restoring MinimapWidget's geometry only after the real window resize has had
    // a chance to happen first) -- a never-set geometry (width/height both 0) keeps this item's
    // own QML-declared default (bottom-right anchor, 220x160) instead.
    Timer {
        id: restoreTimer
        interval: 100
        onTriggered: {
            const restored = root.presenter.initialGeometry();
            if (restored.width > 0 && restored.height > 0) {
                root.width = restored.width;
                root.height = restored.height;
                root.x = restored.x;
                root.y = restored.y;
            }
            root.clampToParent();
        }
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

    // Pure clamp arithmetic lives on the C++ presenter (QuickMinimap::clampGeometry()) --
    // deterministic geometry math with no interactivity of its own, unlike the mouse-drag
    // resize/move math this file legitimately owns. Kept testable there instead of duplicated
    // here in untestable QML/JS.
    function clampToParent() {
        if (!parent) {
            return;
        }
        const r = presenter.clampGeometry(Qt.rect(x, y, width, height), parent.width, parent.height);
        x = r.x;
        y = r.y;
        width = r.width;
        height = r.height;
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

    // MinimapWidget::paintEvent() just does painter.fillRect(rect(), palette().window().color())
    // -- no border, no rounded corners. Matched exactly here (this project's own radius:4 +
    // border.width:1 was never in production).
    Rectangle {
        anchors.fill: parent
        color: root.palette.window
    }

    Image {
        anchors.fill: parent
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
        border.width: 1.6 // MinimapWidget::paintEvent()'s pen.setWidthF(1.6) exactly
    }

    // Click/drag-to-navigate: mirrors MinimapWidget::mousePressEvent()/mouseMoveEvent()'s
    // fallback ("neither a resize nor move handle") branch. Declared before (so painted/
    // hit-tested beneath) the move-handle and resize-handle MouseAreas below, so their smaller,
    // more specific areas take priority over this one, matching MinimapWidget::
    // mousePressEvent()'s own resize-then-move-then-navigate precedence.
    MouseArea {
        id: navigateArea
        anchors.fill: parent
        hoverEnabled: true
        // MinimapWidget::mousePressEvent()'s resize/move-handle checks are gated on
        // Qt::LeftButton, but its fallback click-to-navigate branch runs unconditionally for
        // any button that reaches it -- so this base MouseArea accepts all buttons, matching
        // Widgets' right/middle-click-also-navigates behavior exactly.
        acceptedButtons: Qt.AllButtons
        onPressed: (mouse) => root.presenter.navigateTo(mouse.x, mouse.y)
        onPositionChanged: (mouse) => { if (pressed) root.presenter.navigateTo(mouse.x, mouse.y); }
        // MinimapWidget::wheelEvent() calls event->accept() and does NOT call the base
        // implementation, to prevent the main view from zooming while the cursor is over the
        // minimap. A QML MouseArea with no onWheel handler lets the event fall through to
        // whatever's underneath (CanvasItem, which zooms) -- this swallows it instead.
        onWheel: (wheel) => { wheel.accepted = true; }
    }

    // Move handle: a strip along the top, dragged to reposition the whole minimap freely --
    // mirrors MinimapWidget::moveHandleRect()/moveBy()/drawMoveHandle() exactly. Unlike this
    // file's own previous version (fully transparent until hovered/pressed), production's strip
    // is always faintly visible -- setMouseTracking(true) in its constructor gives it real
    // hover feedback (not drag-only), matched here with hoverEnabled: true.
    MouseArea {
        id: moveHandle
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: root.moveHandleHeight
        hoverEnabled: true
        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
        property point lastGlobal
        readonly property bool highlighted: pressed || containsMouse

        Rectangle {
            anchors.fill: parent
            color: moveHandle.highlighted ? root.palette.highlight : root.palette.windowText
            opacity: moveHandle.highlighted ? (90 / 255) : (35 / 255)
        }
        // Grip dots: 2 rows x 3 columns, matching drawMoveHandle()'s exact layout (dotRadius
        // 1.2, spacingX 6, spacingY 5) -- not this file's own previous single row of 3.
        Grid {
            anchors.centerIn: parent
            columns: 3
            rowSpacing: 5
            columnSpacing: 6 - 2.4 // spacingX is a full dot-to-dot center distance (6); Grid's
                                    // columnSpacing is edge-to-edge, so subtract one dot's diameter
            Repeater {
                model: 6
                Rectangle {
                    width: 2.4; height: 2.4; radius: 1.2
                    color: root.palette.windowText
                    opacity: moveHandle.highlighted ? (230 / 255) : (140 / 255)
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
    MouseArea {
        id: resizeNW
        anchors.top: parent.top; anchors.left: parent.left
        width: root.handleSize * 2; height: root.handleSize * 2
        hoverEnabled: true
        cursorShape: Qt.SizeFDiagCursor
        property point lastGlobal
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
        hoverEnabled: true
        cursorShape: Qt.SizeFDiagCursor
        property point lastGlobal
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
        hoverEnabled: true
        cursorShape: Qt.SizeBDiagCursor
        property point lastGlobal
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
        hoverEnabled: true
        cursorShape: Qt.SizeBDiagCursor
        property point lastGlobal
        onPressed: (mouse) => { lastGlobal = mapToItem(root.parent, mouse.x, mouse.y); }
        onPositionChanged: (mouse) => {
            if (!pressed) return;
            const current = mapToItem(root.parent, mouse.x, mouse.y);
            root.performResize(false, true, true, false, current.x - lastGlobal.x, current.y - lastGlobal.y);
            lastGlobal = current;
        }
        onReleased: root.presenter.commitGeometry(root.x, root.y, root.width, root.height)
    }

    // Resize-grip marks at the 4 corners, matching drawResizeGrips() exactly (inset 3, arm
    // length 9, alpha 130/255 idle vs 230/255 + thicker when that corner is hovered or actively
    // being dragged) -- this file's own previous version had no visual grips at all here
    // (cursor-shape-only, a documented simplification). Positioned in root's own coordinates
    // (not the small, easier-to-grab MouseAreas' own bounds above), each as two thin Rectangles
    // forming an L, since QML has no direct line primitive as convenient as QPainter::drawLine().
    component ResizeGripMark: Item {
        id: mark
        required property bool highlighted
        required property int armX // +1 rightwards, -1 leftwards
        required property int armY // +1 downwards, -1 upwards
        readonly property real lineWidth: highlighted ? 2.0 : 1.4
        readonly property color lineColor: highlighted ? root.palette.highlight : root.palette.windowText
        readonly property real lineOpacity: highlighted ? (230 / 255) : (130 / 255)

        Rectangle { // horizontal arm
            x: mark.armX > 0 ? 0 : -9
            y: -mark.lineWidth / 2
            width: 9; height: mark.lineWidth
            color: mark.lineColor
            opacity: mark.lineOpacity
        }
        Rectangle { // vertical arm
            x: -mark.lineWidth / 2
            y: mark.armY > 0 ? 0 : -9
            width: mark.lineWidth; height: 9
            color: mark.lineColor
            opacity: mark.lineOpacity
        }
    }

    ResizeGripMark {
        x: 3; y: 3 // inset 3
        armX: 1; armY: 1
        highlighted: resizeNW.pressed || resizeNW.containsMouse
    }
    ResizeGripMark {
        x: root.width - 1 - 3; y: 3
        armX: -1; armY: 1
        highlighted: resizeNE.pressed || resizeNE.containsMouse
    }
    ResizeGripMark {
        x: 3; y: root.height - 1 - 3
        armX: 1; armY: -1
        highlighted: resizeSW.pressed || resizeSW.containsMouse
    }
    ResizeGripMark {
        x: root.width - 1 - 3; y: root.height - 1 - 3
        armX: -1; armY: -1
        highlighted: resizeSE.pressed || resizeSE.containsMouse
    }
}
