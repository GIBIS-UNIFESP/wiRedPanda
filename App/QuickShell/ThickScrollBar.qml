// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls

// Fusion/ScrollBar.qml's own handle (contentItem: Rectangle { implicitWidth: 6 }) renders only
// ~10px thick overall (6px handle + 2px padding on each side) -- next to Widgets' native
// QScrollBar (Fusion widget style), which has a full up/down arrow button at each end plus a
// track, all within a ~15-17px-wide column (pixel-compared against the reference build).
// Reproducing the arrow buttons is a separate, much larger undertaking (a full custom
// ScrollBar reimplementation) -- not attempted here. This only thickens the handle to ~16px
// (12px + the same 2px padding on each side) so it's comfortably visible/grabbable, closing
// the most visible part of the size gap; everything else (opacity states, AlwaysOn-vs-transient
// show/hide, colors) is copied unchanged from Fusion/ScrollBar.qml.
//
// Used everywhere this project shows a ScrollBar (ElementPalette.qml's category lists,
// Main.qml's canvas scrollbars) instead of the bare QtQuick.Controls ScrollBar, so the size
// fix lives in one place.
ScrollBar {
    id: control

    contentItem: Rectangle {
        implicitWidth: 12
        implicitHeight: 12

        radius: Math.min(width, height) / 2
        color: control.pressed ? control.palette.dark : control.palette.mid
        opacity: 0.0

        states: State {
            name: "active"
            when: control.policy === ScrollBar.AlwaysOn || (control.active && control.size < 1.0)
            PropertyChanges { control.contentItem.opacity: 0.75 }
        }

        transitions: Transition {
            from: "active"
            SequentialAnimation {
                PauseAnimation { duration: 450 }
                NumberAnimation { target: control.contentItem; duration: 200; property: "opacity"; to: 0.0 }
            }
        }
    }
}
