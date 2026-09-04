// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// Guided-interface-tour overlay, driven by AppController.tour (App/QuickShell/Chrome/
// QuickTourController). Ports TourOverlay.cpp's QWidget-based dim+spotlight paintEvent() onto a
// QML Canvas (2D context) plus a Qt Quick Controls callout panel -- the callout's own layout
// follows ExerciseOverlay.qml's identical precedent, but its colors are hand-rolled per theme
// (dark/darkTheme below) rather than reusing root.palette the way ExerciseOverlay does: the
// amber-on-dark/blue-on-light accent is a deliberate spotlight identity shared between the
// callout border and the spotlight ring itself, not decomposable into an existing QPalette role.
//
// Instantiated as a direct ApplicationWindow content child in Main.qml (anchors.fill: parent),
// not a canvasHost child like ExerciseOverlay/Minimap -- it needs to spotlight items outside
// canvasHost too (the element palette, its category tabs, the search bar, the element editor
// panel, all in leftPane). This file has no id-scoped access to elementPalette/elementEditor/
// canvasHost themselves, so Main.qml passes them in explicitly as paletteItem/editorItem/
// canvasAreaItem below (same precedent as ElementPalette.qml's own canvasWidth/canvasHeight
// properties) -- target resolution (resolveTarget(), mapping a step's target id to a rect in
// this item's local space) and click dispatch (dispatchClick(), running a step's click ids
// before that target is resolved) both live here, using those typed properties, rather than
// reaching up through the Window attached property's untyped QQuickWindow (which qmllint can't
// see custom functions through).
Item {
    id: root

    readonly property QuickTourController presenter: AppController.tour
    // The three item references resolveTarget()/dispatchClick() need. paletteItem is typed as
    // the real ElementPalette component (not plain Item) so paletteItem.categoryTabBar/
    // .searchField resolve statically; editorItem/canvasAreaItem only ever need width/height
    // (real Item properties), so plain Item is enough for them.
    required property ElementPalette paletteItem
    required property Item editorItem
    required property Item canvasAreaItem

    visible: presenter.active
    z: 2000

    readonly property bool dark: AppController.darkTheme
    readonly property color dimColor: dark ? Qt.rgba(0, 0, 0, 160 / 255) : Qt.rgba(0, 0, 0, 90 / 255)
    readonly property color spotlightColor: dark ? Qt.rgba(1, 200 / 255, 50 / 255, 220 / 255) : Qt.rgba(0, 120 / 255, 210 / 255, 220 / 255)
    readonly property color calloutBg: dark ? Qt.rgba(20 / 255, 20 / 255, 20 / 255, 240 / 255) : Qt.rgba(1, 1, 1, 240 / 255)
    readonly property color calloutBorder: dark ? Qt.rgba(1, 200 / 255, 50 / 255, 120 / 255) : Qt.rgba(0, 100 / 255, 200 / 255, 150 / 255)
    readonly property color counterColor: dark ? Qt.rgba(1, 200 / 255, 50 / 255, 200 / 255) : Qt.rgba(0, 100 / 255, 200 / 255, 200 / 255)
    readonly property color titleColor: dark ? "white" : "#141414"
    readonly property color bodyColor: dark ? Qt.rgba(220 / 255, 220 / 255, 220 / 255, 230 / 255) : Qt.rgba(50 / 255, 50 / 255, 50 / 255, 230 / 255)

    readonly property int spotlightPad: 6
    readonly property int calloutWidth: 360
    readonly property int calloutMargin: 12

    property rect highlightRect: Qt.rect(0, 0, 0, 0)
    readonly property bool hasHighlight: highlightRect.width > 0 && highlightRect.height > 0

    // Maps \a id (App/Resources/Tours/README.md's closed target vocabulary) to a rect in this
    // item's local coordinate space, or null if \a id is empty/"none"/unresolvable. "toolbar"
    // (this chrome has no toolbar yet -- a still-open gap) and every "bwd:"-prefixed id still
    // fall through to null:
    // BeWavedDolphin's DolphinWindow is a genuinely separate top-level QQuickWindow, and Qt
    // Quick has no cross-window item-to-item coordinate mapping/compositing the way Widgets'
    // mapToGlobal()-based overlay-reparenting trick (MainWindow::resolveTourTarget()'s own
    // overlay->setParentWindow(m_bwd) branch) relies on -- this overlay, living in the main
    // window, can never draw a spotlight rect over a different window's surface. A real,
    // architecturally sound fix would need a second overlay instance living inside
    // DolphinWindow.qml itself.
    function resolveTarget(id) {
        function mapRect(item) {
            if (!item) {
                return null;
            }
            const topLeft = item.mapToItem(root, 0, 0);
            return Qt.rect(topLeft.x, topLeft.y, item.width, item.height);
        }

        if (!id || id === "none") {
            return null;
        }
        if (id === "elementPalette") return mapRect(paletteItem);
        if (id === "ioTab") return mapRect(paletteItem.categoryTabBar.itemAt(0));
        if (id === "gatesTab") return mapRect(paletteItem.categoryTabBar.itemAt(1));
        if (id === "canvasArea") return mapRect(canvasAreaItem);
        if (id === "elementEditor") return mapRect(editorItem);
        if (id === "searchBar") return mapRect(paletteItem.searchField);
        return null;
    }

    // Runs \a id (App/Resources/Exercises/README.md's closed click vocabulary, shared with
    // Tours) before this step's target is resolved -- see QuickTourController::
    // clicksRequested()'s doc comment for the emission-order guarantee that makes "click
    // executes before target resolution" hold. "actionWaveform"/"bwd:actionCombinational":
    // AppController.dolphin is the same QuickDolphinController instance DolphinWindow.qml
    // binds to, so calling combinational() on it here works regardless of which window is
    // currently focused -- no reference to the DolphinWindow instance itself needed, unlike
    // resolveTarget()'s spotlight-rect problem above.
    function dispatchClick(id) {
        switch (id) {
        case "ioTab": paletteItem.categoryTabBar.currentIndex = 0; break;
        case "gatesTab": paletteItem.categoryTabBar.currentIndex = 1; break;
        case "combinational": paletteItem.categoryTabBar.currentIndex = 2; break;
        case "memoryTab": paletteItem.categoryTabBar.currentIndex = 3; break;
        case "actionPlay": AppController.simulationRunning = !AppController.simulationRunning; break;
        case "actionWaveform": AppController.openWaveform(); break;
        case "bwd:actionCombinational": AppController.dolphin.combinational(); break;
        case "setupElementEditorDemo":
        case "setupWaveformDemo":
            AppController.runTourDemoAction(id);
            break;
        }
    }

    function refreshHighlight() {
        const targetId = presenter.currentTarget;
        const resolved = (!targetId || targetId.length === 0 || targetId === "none")
            ? null
            : resolveTarget(targetId);
        highlightRect = resolved ? resolved : Qt.rect(0, 0, 0, 0);
        spotlightCanvas.requestPaint();
        repositionCallout();
    }

    // clicksRequested() fires before stepChanged() (see QuickTourController's own doc comment on
    // that signal) -- dispatching every click id here, synchronously, before onStepChanged below
    // re-resolves the highlight is what makes "click executes before target resolution" hold.
    Connections {
        target: root.presenter
        function onClicksRequested(ids) {
            for (const id of ids) {
                root.dispatchClick(id);
            }
        }
        function onStepChanged() { root.refreshHighlight(); }
    }

    onWidthChanged: refreshHighlight()
    onHeightChanged: refreshHighlight()

    function repositionCallout() {
        const pad = calloutMargin;
        const cw = callout.width;
        const ch = callout.height;
        const ow = width;
        const oh = height;

        if (!hasHighlight) {
            callout.x = (ow - cw) / 2;
            callout.y = (oh - ch) / 2;
            return;
        }

        const spot = Qt.rect(highlightRect.x - spotlightPad, highlightRect.y - spotlightPad,
                              highlightRect.width + 2 * spotlightPad, highlightRect.height + 2 * spotlightPad);

        let cx = Math.max(pad, Math.min(spot.x + (spot.width - cw) / 2, ow - cw - pad));
        let cy = spot.y + spot.height + pad;

        if (cy + ch + pad > oh) {
            cy = spot.y - ch - pad;
        }
        if (cy < pad) {
            cy = (oh - ch) / 2;
            cx = Math.min(spot.x + spot.width + pad, ow - cw - pad);
            if (cx + cw + pad > ow) {
                cx = Math.max(pad, spot.x - cw - pad);
            }
        }

        callout.x = cx;
        callout.y = Math.max(pad, Math.min(cy, oh - ch - pad));
    }

    Shortcut {
        sequence: "Escape"
        enabled: root.visible
        onActivated: root.presenter.close()
    }

    Canvas {
        id: spotlightCanvas
        anchors.fill: parent

        onPaint: {
            const ctx = getContext("2d");
            ctx.reset();
            ctx.fillStyle = root.dimColor;

            if (!root.hasHighlight) {
                ctx.fillRect(0, 0, width, height);
                return;
            }

            const spot = Qt.rect(root.highlightRect.x - root.spotlightPad, root.highlightRect.y - root.spotlightPad,
                                  root.highlightRect.width + 2 * root.spotlightPad, root.highlightRect.height + 2 * root.spotlightPad);

            // Four strips around the spotlight hole, not a single subtracted path: avoids
            // depending on this Canvas's fill-rule/compound-subpath support for something that
            // isn't documented/verified here, unlike QPainterPath::subtracted() on the Widgets
            // side. Leaves the hole's corners square instead of rounded (a minor, deliberate
            // visual simplification) -- the spotlight ring drawn below is still rounded, since
            // stroke() doesn't depend on fill-rule at all.
            ctx.fillRect(0, 0, width, spot.y);
            ctx.fillRect(0, spot.y + spot.height, width, height - spot.y - spot.height);
            ctx.fillRect(0, spot.y, spot.x, spot.height);
            ctx.fillRect(spot.x + spot.width, spot.y, width - spot.x - spot.width, spot.height);

            ctx.lineWidth = 2;
            ctx.strokeStyle = root.spotlightColor;
            ctx.beginPath();
            const r = 6;
            ctx.moveTo(spot.x + r, spot.y);
            ctx.lineTo(spot.x + spot.width - r, spot.y);
            ctx.quadraticCurveTo(spot.x + spot.width, spot.y, spot.x + spot.width, spot.y + r);
            ctx.lineTo(spot.x + spot.width, spot.y + spot.height - r);
            ctx.quadraticCurveTo(spot.x + spot.width, spot.y + spot.height, spot.x + spot.width - r, spot.y + spot.height);
            ctx.lineTo(spot.x + r, spot.y + spot.height);
            ctx.quadraticCurveTo(spot.x, spot.y + spot.height, spot.x, spot.y + spot.height - r);
            ctx.lineTo(spot.x, spot.y + r);
            ctx.quadraticCurveTo(spot.x, spot.y, spot.x + r, spot.y);
            ctx.closePath();
            ctx.stroke();
        }
    }

    Rectangle {
        id: callout
        width: root.calloutWidth
        height: content.implicitHeight + 28
        radius: 8
        color: root.calloutBg
        border.color: root.calloutBorder
        border.width: 1

        onWidthChanged: root.repositionCallout()
        onHeightChanged: root.repositionCallout()

        ColumnLayout {
            id: content
            anchors.fill: parent
            anchors.margins: 14
            spacing: 8

            Label {
                Layout.fillWidth: true
                text: root.presenter.stepCounterText
                color: root.counterColor
                font.pixelSize: 12
            }

            Label {
                Layout.fillWidth: true
                text: root.presenter.title
                wrapMode: Text.WordWrap
                font.pixelSize: 16
                font.bold: true
                color: root.titleColor
            }

            Label {
                Layout.fillWidth: true
                text: root.presenter.body
                wrapMode: Text.WordWrap
                font.pixelSize: 14
                color: root.bodyColor
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Button {
                    text: qsTr("Exit")
                    ToolTip.text: qsTr("Stop the tour")
                    ToolTip.visible: hovered
                    onClicked: root.presenter.close()
                }
                Button {
                    text: qsTr("← Back")
                    enabled: root.presenter.prevEnabled
                    onClicked: root.presenter.goToPreviousStep()
                }
                Item { Layout.fillWidth: true }
                Button {
                    text: root.presenter.nextButtonText
                    highlighted: true
                    onClicked: root.presenter.advanceStep()
                }
            }
        }
    }
}
