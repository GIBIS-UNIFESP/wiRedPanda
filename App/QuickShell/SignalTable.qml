// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import QuickShell

// Quick-side port of BewavedDolphin's QTableView + SignalDelegate pairing. Binds directly to a
// real SignalModel* (100% reused as-is -- a genuine QAbstractItemModel, no Quick-specific model
// needed) via Qt Quick Controls' TableView, plus stock HorizontalHeaderView/VerticalHeaderView
// for the row/column headers -- both already read real header text from the model's own
// headerData() (SignalModel::setVerticalHeaderLabels()), falling back to bare row/column numbers
// only when a model provides none, so no custom header delegate is needed either.
//
// The cell delegate below reproduces SignalDelegate::drawWaveform()'s band/plateau/edge-bar
// rectangles (SignalDelegate.cpp's own private paint method, not directly reusable -- these are
// the same geometry fractions, decoded from the original 100x30 SVGs, duplicated here rather
// than shared since the source lives in a QItemDelegate-coupled class this file has no
// dependency on otherwise) via a per-cell QML Canvas -- same 2D-paint technique TourOverlay.qml's
// spotlight already established. TableView is already virtualized (only visible cells become
// real delegate items), which is the main defence against per-cell Canvas cost at scale.
Item {
    id: root
    required property QuickDolphinController controller

    // DolphinWindow.qml's own font.pointSize (its ApplicationWindow's, otherwise-untouched,
    // platform-default font) -- read via a plain property set from there instead of
    // Qt.application.font.pointSize directly: qmllint can't see QQuickApplication's own `font`
    // property through the base QQmlApplication type Qt.application statically resolves to (same
    // blind spot Main.qml's toolbar font-scaling comment already documents).
    property real basePointSize: 9

    // Exposed for DolphinWindow.qml's Fit to Screen action, mirroring DolphinZoom::fitScreen()'s
    // own real header-size query (m_view->horizontalHeader()->height()/verticalHeader()->width())
    // -- ElementPalette.qml's categoryTabBar/searchField aliases are the established precedent
    // for passing an internal child's real geometry up through a typed property.
    property alias verticalHeaderWidth: verticalHeader.width
    property alias horizontalHeaderHeight: horizontalHeader.height

    TableView {
        id: tableView
        anchors.fill: parent
        anchors.leftMargin: verticalHeader.width
        anchors.topMargin: horizontalHeader.height
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        model: root.controller.model
        // A real QItemSelectionModel targeting the live model (QuickDolphinController::
        // selectionModel(), constructed fresh per createWaveform() call) -- Cut/Copy/Paste/Clear/
        // Invert/SetTo0/SetTo1/SetClockWave all read this same instance's selection() /
        // selectedIndexes() from C++. Matches QTableView's own un-set defaults (ExtendedSelection
        // + cell-level SelectItems), since BewavedDolphin never overrides them.
        selectionModel: root.controller.selectionModel
        selectionBehavior: TableView.SelectCells
        selectionMode: TableView.ExtendedSelection
        rowHeightProvider: function (row) { return root.controller.rowHeight; }
        columnWidthProvider: function (column) { return root.controller.columnWidth; }

        // Cached sizes must be recomputed whenever the zoom (columnWidth/rowHeight) or the
        // model itself (row/column count) changes -- TableView doesn't poll the provider
        // functions on every frame, only when told to.
        Connections {
            target: root.controller
            function onZoomChanged() { tableView.forceLayout(); }
            function onModelChanged() { tableView.forceLayout(); }
        }

        delegate: Rectangle {
            id: cell
            required property int row
            required property int column
            required property var display

            implicitWidth: root.controller.columnWidth
            implicitHeight: root.controller.rowHeight
            color: (row % 2 === 0) ? root.palette.base : root.palette.alternateBase

            // Show Numbers mode (View menu): a pure display toggle, mirrors SignalDelegate::
            // paint()'s Number-mode fallback (plain centered text, no waveform geometry) --
            // the model's values never change, so no undo/file dependency, unlike this cell's
            // double-click-to-toggle interaction below.
            Text {
                anchors.centerIn: parent
                visible: root.controller.numberMode
                text: cell.display
                // Mirrors DolphinZoom::apply()'s Fit-to-screen font scaling (column-zoom leaves
                // text untouched, matching Widgets exactly). Header text is left at its stock
                // QtQuick.Controls size -- HeaderView exposes no font property to override
                // without replacing its whole delegate (background/border chrome included),
                // a bigger visual-parity risk than the font-size gap itself for this cosmetic item.
                font.pointSize: Math.max(1, Math.round(root.basePointSize * root.controller.fontScale))
            }

            Canvas {
                id: waveCanvas
                anchors.fill: parent
                visible: !root.controller.numberMode

                // Read through root.controller (a real, correctly QML-registered
                // wiredpanda type), not root.controller.model directly: SignalModel is
                // part of wiredpanda_lib, and qt_add_qml_module(wiredpanda ...)'s type
                // scanner only walks wiredpanda's own listed sources -- a wiredpanda_lib
                // class can never be made QML-visible from here, whether via its own
                // QML_ELEMENT or a QML_FOREIGN struct. QuickDolphinController::isInputRow()/
                // cellValue() wrap the model's own methods for exactly this reason.
                readonly property int cellValue: cell.display
                readonly property bool isInput: root.controller.isInputRow(cell.row)

                onCellValueChanged: requestPaint()
                onIsInputChanged: requestPaint()
                onWidthChanged: requestPaint()
                onHeightChanged: requestPaint()
                onVisibleChanged: if (visible) requestPaint()

                onPaint: {
                    const ctx = getContext("2d");
                    ctx.reset();

                    const hasPrev = cell.column > 0;
                    const prevValue = hasPrev ? root.controller.cellValue(cell.row, cell.column - 1) : 0;

                    // Mirrors SignalDelegate::segmentFor() exactly.
                    let high;
                    let risingOrFalling = false;
                    if (cellValue === 0) {
                        risingOrFalling = hasPrev && prevValue === 1; // Falling
                        high = false;
                    } else {
                        risingOrFalling = hasPrev && prevValue === 0; // Rising
                        high = true;
                    }

                    const lineColor = isInput ? Qt.rgba(0x75 / 255, 0x8e / 255, 1.0, 1.0) : Qt.rgba(0, 128 / 255, 0, 1.0);
                    const bandColor = Qt.rgba(lineColor.r, lineColor.g, lineColor.b, 0.5);

                    const w = width;
                    const h = height;
                    const lineTop = (high ? 8 / 30 : 20 / 30) * h;
                    const lineBottom = (high ? 12 / 30 : 24 / 30) * h;
                    const thickness = (4 / 30) * h;

                    ctx.fillStyle = bandColor;
                    ctx.fillRect(0, lineBottom, w, h - lineBottom);
                    ctx.fillStyle = lineColor;
                    ctx.fillRect(0, lineTop, w, thickness);

                    if (risingOrFalling) {
                        const barTop = (8 / 30) * h;
                        const barBottom = (24 / 30) * h;
                        ctx.fillRect(0, barTop, (4 / 100) * w, barBottom - barTop);
                    }
                }
            }

            // TableView has no built-in mouse-driven selection (unlike QTableView), so a plain
            // click here is what populates root.controller.selectionModel in the first place --
            // needed for Cut/Copy/Paste/Clear/Invert/SetTo0/SetTo1/SetClockWave and the
            // double-click handler below to have anything to act on. Mirrors
            // QAbstractItemView::ExtendedSelection's own plain/Ctrl/Shift-click semantics exactly
            // (see QuickDolphinController::selectCell()'s own doc comment).
            //
            // Mirrors BewavedDolphin::on_tableView_cellDoubleClicked(): toggle every selected
            // cell (not just the one under the cursor), as one undoable command -- QML's
            // MouseArea fires the double-click's first press as a plain onClicked too, which
            // (via selectCell() above) already guarantees this cell is part of the selection
            // invertSelected() then acts on.
            MouseArea {
                anchors.fill: parent
                onClicked: (mouse) => root.controller.selectCell(cell.row, cell.column, mouse.modifiers)
                onDoubleClicked: root.controller.invertSelected()
            }
        }
    }

    // Mirrors BewavedDolphin::eventFilter()'s wheel-over-viewport interception: the mouse wheel
    // zooms columns instead of scrolling. acceptedButtons: Qt.NoButton means this MouseArea never
    // grabs press/click events, so they still reach the per-cell MouseArea above -- only wheel
    // events are intercepted here, ahead of TableView's own Flickable wheel-to-scroll handling
    // underneath (declared after tableView, so it renders on top and sees the event first).
    MouseArea {
        anchors.fill: tableView
        acceptedButtons: Qt.NoButton
        onWheel: (wheel) => {
            if (wheel.angleDelta.y > 0) {
                root.controller.zoomIn();
            } else if (wheel.angleDelta.y < 0) {
                root.controller.zoomOut();
            }
        }
    }

    HorizontalHeaderView {
        id: horizontalHeader
        anchors.left: tableView.left
        anchors.top: parent.top
        anchors.right: parent.right
        clip: true
        syncView: tableView
    }

    VerticalHeaderView {
        id: verticalHeader
        anchors.top: tableView.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        clip: true
        syncView: tableView
    }
}
