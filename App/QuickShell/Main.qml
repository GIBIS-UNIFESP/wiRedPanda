// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls

import QuickShell

ApplicationWindow {
    id: window

    width: 1024
    height: 768
    visible: true
    title: qsTr("wiRedPanda")

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")

            MenuItem { text: qsTr("New") }
            MenuItem { text: qsTr("Open...") }
            MenuItem { text: qsTr("Save") }
            MenuSeparator {}
            MenuItem {
                text: qsTr("Quit")
                onTriggered: Qt.quit()
            }
        }

        Menu {
            title: qsTr("&Edit")
        }
    }

    // Phase 1 canvas prototype: real GraphicElement/Simulation classes, batched QSGNode
    // rendering, spatial-index hit-testing. Click a switch (the left two boxes) to toggle it —
    // the AND gate and LED update live, driven by the real Simulation engine.
    CanvasItem {
        anchors.fill: parent
        focus: true
    }
}
