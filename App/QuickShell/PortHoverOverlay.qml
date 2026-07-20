// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick

import QuickShell

// Port-hover highlight/label overlay, driven by AppController.portHoverOverlay (App/QuickShell/
// Chrome/QuickPortHoverOverlay) -- ports ConnectionManager::setHoverPort()/showHoverLabels()'s
// Widgets-side feature: hovering a port highlights it and every port connected to it via a
// wire, with in-situ name-chip labels for the named ones (see
// project_quick_real_missing_features_found_in_phase7 project memory for why this was deferred
// out of the original Phase 7 test-writing pass). Genuinely canvasHost-local (like Minimap/
// ExerciseOverlay, unlike ICPreviewPopup): CanvasItem::portHoverChanged() already emits
// worldToScreen()-converted coordinates in this item's own coordinate space, so no
// reparenting/mapFromGlobal() is needed. No separate tooltip-delay reveal stage the way
// showHoverLabels() had (Quick has no QToolTip-timed helpEvent() to mirror) -- the highlight
// ring and label chip appear together, immediately, driven by the one portHoverChanged() signal.
Item {
    id: root

    readonly property QuickPortHoverOverlay presenter: AppController.portHoverOverlay

    anchors.fill: parent
    z: 900

    Repeater {
        model: root.presenter.chips

        Item {
            id: chipRoot
            required property var modelData

            x: chipRoot.modelData.screenX - chipRoot.modelData.radius
            y: chipRoot.modelData.screenY - chipRoot.modelData.radius
            width: chipRoot.modelData.radius * 2
            height: chipRoot.modelData.radius * 2

            // Highlight ring around the port glyph itself -- mirrors Port::hoverEnter()'s
            // brush-color swap, as a ring rather than recoloring the glyph fill (the glyph
            // itself is baked into CanvasItem's per-element texture-atlas cache, which this
            // overlay deliberately doesn't touch -- see CanvasItem::buildPortHoverChips()'s own
            // doc comment). Uses modelData.ringColor (a real ThemeManager color CanvasItem
            // already computed), not a generic Qt Quick palette role -- see that method's doc
            // comment for why (root.palette.highlight/toolTipBase/toolTipText leave the tooltip
            // roles unthemed for this app's dark palette).
            Rectangle {
                anchors.fill: parent
                radius: width / 2
                color: "transparent"
                border.color: chipRoot.modelData.ringColor
                border.width: 2
            }

            // In-situ name chip, biased away from the element body per side. Skipped for
            // unnamed ports (still gets the ring above), matching ConnectionManager::
            // showHoverLabels()'s own empty-name skip.
            Rectangle {
                id: label
                visible: chipRoot.modelData.text.length > 0
                color: chipRoot.modelData.labelBgColor
                // Same color pairing as the original PortHoverLabel::paint(): the border uses
                // the text color, not the ring's hover-port color.
                border.color: chipRoot.modelData.labelTextColor
                width: labelText.implicitWidth + 10
                height: labelText.implicitHeight + 4

                readonly property real gap: 8
                x: {
                    switch (chipRoot.modelData.side) {
                    case "left": return -gap - width;
                    case "right": return chipRoot.width + gap;
                    default: return (chipRoot.width - width) / 2;
                    }
                }
                y: {
                    switch (chipRoot.modelData.side) {
                    case "top": return -gap - height;
                    case "bottom": return chipRoot.height + gap;
                    default: return (chipRoot.height - height) / 2;
                    }
                }

                Text {
                    id: labelText
                    anchors.centerIn: parent
                    text: chipRoot.modelData.text
                    color: chipRoot.modelData.labelTextColor
                }
            }
        }
    }
}
