// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// Circuit-exercise step overlay, driven by AppController.exercise (App/QuickShell/Chrome/
// QuickExerciseController) -- ports ExerciseOverlay.cpp's QWidget tree (QVBoxLayout/QHBoxLayout/
// QLabel/QPushButton, each with a hand-rolled per-theme stylesheet string) onto QML/Qt Quick
// Controls directly, using root.palette for theme-aware colors instead (Item.palette already
// reflects the app's current QPalette -- see Minimap.qml's identical precedent -- so this needs
// no dark/light branch of its own). paintEvent()'s only custom painting (one rounded-rect
// translucent background) is the outer Rectangle below. Positioned bottom-center over the
// canvas, declared as a canvasHost child in Main.qml -- same positioning precedent as
// Minimap.qml.
Item {
    id: root

    readonly property QuickExerciseController presenter: AppController.exercise

    visible: presenter.active
    z: 1500

    width: 480
    height: content.implicitHeight + 24
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? parent.height - height - 12 : 0

    Rectangle {
        anchors.fill: parent
        radius: 8
        color: root.palette.window
        opacity: 0.92
        border.color: root.palette.mid
        border.width: 1
    }

    ColumnLayout {
        id: content
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Label {
            Layout.fillWidth: true
            text: root.presenter.stepCounterText
            color: root.palette.placeholderText
            font.pixelSize: 12
        }

        Label {
            Layout.fillWidth: true
            text: root.presenter.instruction
            wrapMode: Text.WordWrap
            font.pixelSize: 14
            font.bold: true
            color: root.palette.windowText
        }

        Label {
            Layout.fillWidth: true
            visible: root.presenter.hintVisible
            text: root.presenter.hint
            wrapMode: Text.WordWrap
            font.pixelSize: 13
            // A distinct amber tint, not palette-derived -- mirrors ExerciseOverlay.cpp's own
            // m_hintColor, which is likewise hardcoded per-theme rather than a QPalette role.
            color: "#c99a00"
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Button {
                text: qsTr("Exit")
                ToolTip.text: qsTr("Close exercise")
                ToolTip.visible: hovered
                onClicked: root.presenter.close()
            }
            Button {
                text: root.presenter.hintButtonText
                visible: !root.presenter.completed
                onClicked: root.presenter.toggleHint()
            }
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("← Back")
                visible: !root.presenter.completed
                enabled: root.presenter.prevEnabled
                onClicked: root.presenter.goToPreviousStep()
            }
            Button {
                text: root.presenter.nextButtonText
                enabled: root.presenter.nextEnabled
                highlighted: true
                onClicked: root.presenter.advanceStep()
            }
        }
    }
}
