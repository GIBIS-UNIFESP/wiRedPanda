// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QuickShell

// Phase 4 named-deferrals follow-up: the truth table editor dialog -- production's floating
// QDialog + QTableWidget (App/UI/ElementEditor.cpp's m_tableBox). A pure view over
// AppController.elementEditor's truthTableColumnLabels/truthTableInputCount/truthTableRows
// (rebuilt by openTruthTable()/toggleTruthTableCell()); clicking an output cell calls
// toggleTruthTableCell(row, column) directly, no local editing state of its own. Opened via
// Main.qml's Connections on elementEditor.truthTableRequested, mirroring shortcutsDialog's
// own open()-by-id precedent.
Dialog {
    id: root
    title: qsTr("Truth Table")
    modal: true
    standardButtons: Dialog.Close
    anchors.centerIn: parent

    readonly property QuickElementEditor editor: AppController.elementEditor

    ScrollView {
        implicitWidth: Math.min(500, grid.implicitWidth + 24)
        implicitHeight: Math.min(500, grid.implicitHeight + 4)
        clip: true

        ColumnLayout {
            id: grid
            spacing: 0

            RowLayout {
                spacing: 0
                Repeater {
                    model: root.editor.truthTableColumnLabels
                    delegate: Label {
                        required property string modelData
                        text: modelData
                        horizontalAlignment: Text.AlignHCenter
                        Layout.preferredWidth: 28
                        font.bold: true
                    }
                }
            }

            Repeater {
                model: root.editor.truthTableRows
                delegate: RowLayout {
                    id: rowDelegate
                    required property truthTableRow modelData
                    required property int index
                    spacing: 0

                    Repeater {
                        model: rowDelegate.modelData.cells
                        delegate: Label {
                            id: cellDelegate
                            required property string modelData
                            required property int index
                            readonly property bool isInputColumn: index < root.editor.truthTableInputCount
                            text: modelData
                            horizontalAlignment: Text.AlignHCenter
                            Layout.preferredWidth: 28
                            font.bold: !isInputColumn
                            opacity: isInputColumn ? 0.55 : 1.0

                            MouseArea {
                                anchors.fill: parent
                                enabled: !cellDelegate.isInputColumn
                                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                onClicked: root.editor.toggleTruthTableCell(rowDelegate.index, cellDelegate.index)
                            }
                        }
                    }
                }
            }
        }
    }
}
