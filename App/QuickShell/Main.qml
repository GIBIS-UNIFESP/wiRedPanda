// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls

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

    Label {
        anchors.centerIn: parent
        text: qsTr("wiRedPanda — Qt Quick rewrite (Phase 0 shell)")
    }
}
