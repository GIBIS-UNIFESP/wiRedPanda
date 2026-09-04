// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

import QuickShell

// Real menu/toolbar/action shell.
// AppController (App/QuickShell/Chrome/QuickAppController, exposed as the AppController QML
// singleton -- see AppControllerForeign in QuickAppController.h) is constructed in Main.cpp
// before this file loads -- it owns the tab list (QuickWorkspaceManager) and exposes every
// action below as a plain, directly-callable method, so this file is close to pure
// layout/wiring: no business logic lives here.
ApplicationWindow {
    id: window

    // A disabled Control's own palette.disabled.<role> only takes effect when set directly on
    // that Control (or, for a plain non-popup Item like ToolBar, on a shared ancestor it's a
    // real child of) -- setting palette.disabled.* on this window
    // (or even on MenuBar/a Menu) never reaches a descendant Control's resolved palette at all,
    // even though every *non*-disabled role set here (palette.windowText, palette.buttonText,
    // etc.) correctly cascades to every Control in the app, extensively relied on elsewhere in
    // this file. Menu popups break the chain hardest: a Menu's own MenuItem children don't even
    // inherit a palette.disabled override set directly on that same Menu -- only setting it
    // directly on the individual MenuItem itself actually works. This constant exists so every
    // one of those individual leaf-Control overrides (ToolBar's disabled ToolButtons; every
    // conditionally-enabled MenuItem/submenu below) uses the same value instead of re-deriving
    // it, not because the cascading actually reaches them from here.
    readonly property color paletteDisabledText: AppController.darkTheme ? "#787878" : "#808080"

    // Matches MainWindowUI.cpp's MainWindow->resize(886, 765) exactly -- Quick's own arbitrary
    // 1280x800 default (~51% more area) makes every fixed-pixel-size control (the 64x64 palette
    // icons especially, which render at the same 64x64 size regardless of window size) look
    // proportionally smaller simply for sitting in a bigger window with more surrounding empty
    // space, not because any control actually shrank.
    width: 886
    height: 765
    visible: true
    title: AppController.windowTitle

    // Matches production's exact Fusion QPalette (App/Core/ThemeManager.cpp's hand-built
    // Light/Dark palettes, applied there via QApplication::setPalette() -- a call Quick
    // Controls never sees, since the Fusion QQC2 style resolves its own palette
    // independently of QGuiApplication::setPalette() and of this project's statically-linked
    // style plugin, so neither of those two mechanisms reach it.
    // This is the one mechanism that does: an explicit palette set on an ApplicationWindow (or
    // any Item) propagates down to every Control that reads control.palette.*
    // (ToolBar.qml/MenuBar.qml/ToolButton.qml/TabButton.qml/MenuBarItem.qml all do), and is
    // documented behavior, not a private API.
    // Values are the exact resolved QPalette::ColorRole colors read back from a live Widgets
    // process, not re-derived from ThemeManager.cpp's setColor() calls directly, so roles it
    // leaves to QPalette's own automatic derivation (light/dark/mid/midlight/shadow) are
    // captured exactly. Bound to AppController.darkTheme (themeChanged-backed) so switching
    // View > Theme at runtime re-themes already-instantiated Controls too, not just a fresh
    // launch -- avoiding the private QQuickTheme API, which cannot do this without crashing.
    palette.window: AppController.darkTheme ? "#353535" : "#ffffff"
    palette.windowText: AppController.darkTheme ? "#ffffff" : "#000000"
    palette.base: AppController.darkTheme ? "#191919" : "#ffffff"
    palette.alternateBase: AppController.darkTheme ? "#353535" : "#e9e7e3"
    palette.text: AppController.darkTheme ? "#ffffff" : "#000000"
    palette.button: AppController.darkTheme ? "#353535" : "#ffffff"
    palette.buttonText: AppController.darkTheme ? "#ffffff" : "#000000"
    palette.brightText: "#ff0000"
    palette.highlight: AppController.darkTheme ? "#2a82da" : "#0078d7"
    palette.highlightedText: AppController.darkTheme ? "#f0f0f0" : "#ffffff"
    palette.link: AppController.darkTheme ? "#2a82da" : "#0000ff"
    // Theme-invariant, not AppController.darkTheme-conditioned like every other role above:
    // ThemeManager.cpp's raw QPalette::ToolTipBase/ToolTipText for dark theme really is
    // white/white (a real, on-its-own white-on-white pair) -- Widgets never actually renders
    // that, because ThemeManager::setTheme() *also* force-applies qApp->setStyleSheet("QToolTip
    // { color: #ffffff; background-color: #2a82da; border: 1px solid white; }"), unconditionally
    // on both themes, which overrides the raw palette for every real QToolTip popup. Quick
    // Controls' Fusion ToolTip.qml has no stylesheet layer to intercept -- it reads
    // control.palette.toolTipBase/toolTipText directly (reusing toolTipText again for its
    // border) -- so copying the raw pre-stylesheet palette values here reproduced the
    // blank-white-tooltip bug the stylesheet exists specifically to prevent. These two values
    // match the stylesheet's own colors instead, which is what Widgets actually shows.
    palette.toolTipBase: "#2a82da"
    palette.toolTipText: "#ffffff"
    palette.placeholderText: AppController.darkTheme ? "#f0f0f0" : "#646464"
    palette.light: AppController.darkTheme ? "#4b4b4b" : "#ffffff"
    palette.midlight: AppController.darkTheme ? "#2a2a2a" : "#ffffff"
    palette.dark: AppController.darkTheme ? "#212121" : "#dddddd"
    palette.mid: AppController.darkTheme ? "#262626" : "#ffffff"
    palette.shadow: AppController.darkTheme ? "#191919" : "#0d0d0d"
    // No palette.disabled block here -- see paletteDisabledText's own doc comment above for why
    // that would be a no-op for every descendant Control anyway.

    onActiveChanged: AppController.handleWindowActiveChanged(active)

    Component.onCompleted: {
        const geom = AppController.restoreWindowGeometry()
        if (geom.width > 0 && geom.height > 0) {
            window.x = geom.x
            window.y = geom.y
            window.width = geom.width
            window.height = geom.height
        }
    }

    onClosing: (close) => {
        if (!AppController.confirmClose()) {
            close.accepted = false
            return
        }
        AppController.saveWindowGeometry(window.x, window.y, window.width, window.height)
    }

    // Mirrors MainWindowUI.cpp's retranslateUi() shortcut assignments for the File menu's
    // actions -- MenuItem has no `shortcut` property of its own in QtQuick.Controls (only
    // Action does, and these aren't Action-backed), so a plain Shortcut item is this project's
    // established mechanism instead (see DolphinWindow.qml's identical pattern). Each one calls
    // the exact same AppController method the matching MenuItem below already calls.
    Shortcut { sequence: "Ctrl+N"; onActivated: AppController.newTab() }
    Shortcut { sequence: "Ctrl+O"; onActivated: AppController.openFile() }
    Shortcut { sequence: "Ctrl+S"; onActivated: AppController.saveFile() }
    Shortcut { sequence: "Ctrl+Shift+S"; onActivated: AppController.saveFileAs() }
    Shortcut { sequence: "Ctrl+F5"; onActivated: AppController.reloadFile() }
    // Mirrors MainWindow.cpp's setupShortcuts(): a global shortcut that focuses the element
    // palette's search field from anywhere in the main window.
    Shortcut { sequence: "Ctrl+F"; onActivated: elementPalette.searchField.forceActiveFocus() }

    // Mirrors MainWindowUI.cpp's actual menu structure exactly: File, Edit, View, Simulation,
    // Examples, Learn, Language, Help -- there is no separate top-level Transform or Export
    // menu in Widgets (rotate/flip/align/distribute live in Edit; the 4 export actions are
    // flat entries in File; Examples is its own top-level menu, not nested under File).
    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem { text: qsTr("&New"); icon.source: "qrc:/Interface/Toolbar/new.svg"; onTriggered: AppController.newTab() }
            MenuItem { text: qsTr("&Open..."); icon.source: "qrc:/Interface/Toolbar/folder.svg"; onTriggered: AppController.openFile() }
            MenuSeparator {}
            Menu {
                title: qsTr("&Recent files:")
                icon.source: "qrc:/Interface/Toolbar/recentFiles.svg"
                // Every sibling MenuItem gets its 16x16 icon size for free from Fusion's own
                // MenuItem.qml default -- but QQuickMenuItemPrivate::setSubMenu() (Qt's own
                // qquickmenuitem.cpp) wraps a nested Menu like this one in a MenuItem via
                // `q->setIcon(newSubMenu->icon())`, replacing that MenuItem's whole icon group
                // wholesale rather than merging into it. This Menu's own icon.width/height were
                // never set (only .source), so the replacement icon carries width/height 0 --
                // IconImage's "no explicit size" case -- and renders recentFiles.svg at its
                // native 64x64 canvas size instead of the standard menu-icon size.
                icon.width: 16
                icon.height: 16
                enabled: AppController.recentFiles.length > 0
                palette.disabled.text: window.paletteDisabledText
                Repeater {
                    model: AppController.recentFiles
                    MenuItem {
                        required property string modelData
                        required property int index
                        // Mirrors MainWindow::updateRecentFileActions()'s exact text:
                        // "&<n> <basename>", a numeric mnemonic plus just the file name -- the
                        // full path (modelData) is still what's passed to openRecentFile() below,
                        // it's only ever shown in full as this item's tooltip.
                        text: "&" + (index + 1) + " " + AppController.recentFileBaseName(modelData)
                        ToolTip.visible: hovered
                        ToolTip.text: modelData
                        onTriggered: AppController.openRecentFile(modelData)
                    }
                }
            }
            MenuItem { text: qsTr("Re&load File"); icon.source: "qrc:/Interface/Toolbar/reloadFile.svg"; onTriggered: AppController.reloadFile() }
            MenuSeparator {}
            MenuItem { text: qsTr("&Save"); icon.source: "qrc:/Interface/Toolbar/save.svg"; onTriggered: AppController.saveFile() }
            MenuItem { text: qsTr("Save &As..."); icon.source: "qrc:/Interface/Toolbar/save.svg"; onTriggered: AppController.saveFileAs() }
            MenuItem { text: qsTr("E&xport to Arduino"); icon.source: "qrc:/Interface/Toolbar/arduino.svg"; onTriggered: AppController.exportArduino() }
            MenuItem { text: qsTr("Export to &SystemVerilog"); icon.source: "qrc:/Interface/Toolbar/verilog.svg"; onTriggered: AppController.exportSystemVerilog() }
            MenuItem { text: qsTr("Export to &PDF"); icon.source: "qrc:/Interface/Toolbar/pdf.svg"; onTriggered: AppController.exportPdf() }
            MenuItem { text: qsTr("Export to &Image"); icon.source: "qrc:/Interface/Toolbar/png.svg"; onTriggered: AppController.exportImage() }
            MenuSeparator {}
            MenuItem { text: qsTr("Make file self-contained"); onTriggered: AppController.makeSelfContained() }
            MenuSeparator {}
            MenuItem { text: qsTr("&Exit"); icon.source: "qrc:/Interface/Toolbar/exit.svg"; onTriggered: Qt.quit() }
        }

        Menu {
            title: qsTr("&Edit")
            // palette.disabled.text set directly on each MenuItem below, not once on this Menu
            // (or MenuBar, or window) -- see paletteDisabledText's own doc comment: a Menu
            // popup's MenuItem children don't inherit a palette.disabled override from their
            // declaring Menu at all, only from being set directly on the MenuItem itself.
            MenuItem { text: qsTr("Undo") + (AppController.undoText ? " (" + AppController.undoText + ")" : ""); enabled: AppController.canUndo; onTriggered: AppController.undo(); palette.disabled.text: window.paletteDisabledText }
            MenuItem { text: qsTr("Redo") + (AppController.redoText ? " (" + AppController.redoText + ")" : ""); enabled: AppController.canRedo; onTriggered: AppController.redo(); palette.disabled.text: window.paletteDisabledText }
            MenuSeparator {}
            MenuItem { text: qsTr("Cu&t"); icon.source: "qrc:/Interface/Toolbar/cut.svg"; onTriggered: AppController.cut() }
            MenuItem { text: qsTr("&Copy"); icon.source: "qrc:/Interface/Toolbar/copy.svg"; onTriggered: AppController.copy() }
            MenuItem { text: qsTr("&Paste"); icon.source: "qrc:/Interface/Toolbar/paste.svg"; onTriggered: AppController.paste() }
            MenuItem { text: qsTr("Dupli&cate"); onTriggered: AppController.duplicateSelection() }
            MenuItem { text: qsTr("&Delete"); icon.source: "qrc:/Interface/Toolbar/delete.svg"; onTriggered: AppController.deleteSelection() }
            MenuSeparator {}
            MenuItem {
                text: qsTr("&Rename")
                icon.source: "qrc:/Interface/Toolbar/rename.svg"
                enabled: AppController.elementEditor.labelVisible
                onTriggered: AppController.elementEditor.requestRenameFocus()
                palette.disabled.text: window.paletteDisabledText
            }
            MenuItem {
                text: qsTr("Cha&nge Trigger")
                icon.source: "qrc:/Components/Input/buttonOff.svg"
                enabled: AppController.elementEditor.triggerVisible
                onTriggered: AppController.elementEditor.requestTriggerFocus()
                palette.disabled.text: window.paletteDisabledText
            }
            MenuSeparator {}
            MenuItem { text: qsTr("Rotate &left"); icon.source: "qrc:/Interface/Toolbar/rotateL.svg"; onTriggered: AppController.rotateLeft() }
            MenuItem { text: qsTr("R&otate right"); icon.source: "qrc:/Interface/Toolbar/rotateR.svg"; onTriggered: AppController.rotateRight() }
            MenuItem { text: qsTr("&Flip horizontally"); onTriggered: AppController.flipHorizontal() }
            MenuItem { text: qsTr("Flip &vertically"); onTriggered: AppController.flipVertical() }
            MenuSeparator {}
            MenuItem { text: qsTr("Align &Left"); onTriggered: AppController.alignLeft() }
            MenuItem { text: qsTr("Align Horizontal &Centers"); onTriggered: AppController.alignHorizontalCenter() }
            MenuItem { text: qsTr("Align &Right"); onTriggered: AppController.alignRight() }
            MenuItem { text: qsTr("Align &Top"); onTriggered: AppController.alignTop() }
            MenuItem { text: qsTr("Align &Vertical Centers"); onTriggered: AppController.alignVerticalCenter() }
            MenuItem { text: qsTr("Align &Bottom"); onTriggered: AppController.alignBottom() }
            MenuSeparator {}
            MenuItem { text: qsTr("Distribute &Horizontally"); onTriggered: AppController.distributeHorizontally() }
            MenuItem { text: qsTr("Distribute Verti&cally"); onTriggered: AppController.distributeVertically() }
            MenuSeparator {}
            MenuItem { text: qsTr("&Select all"); icon.source: "qrc:/Interface/Toolbar/selectAll.svg"; onTriggered: AppController.selectAll() }
            MenuItem {
                text: qsTr("Cl&ear selection")
                icon.source: "qrc:/Interface/Toolbar/clearSelection.svg"
                onTriggered: AppController.currentTab.canvas().clearSelection()
            }
        }

        // Mirrors MainWindowUi's View menu. Fast Mode is deliberately not ported: it's a
        // QGraphicsView-specific rendering-quality knob (GraphicsView.cpp toggles
        // antialiasing/caching hints) with no equivalent on Quick's already-GPU-accelerated
        // scene graph renderer.
        Menu {
            title: qsTr("&View")
            Menu {
                title: qsTr("Zoom")
                MenuItem { text: qsTr("Zoom &in"); icon.source: "qrc:/Interface/Toolbar/zoomIn.svg"; onTriggered: AppController.zoomIn() }
                MenuItem { text: qsTr("Zoom &out"); icon.source: "qrc:/Interface/Toolbar/zoomOut.svg"; onTriggered: AppController.zoomOut() }
                MenuItem { text: qsTr("&Reset Zoom"); icon.source: "qrc:/Interface/Toolbar/zoomReset.svg"; onTriggered: AppController.resetZoom() }
                MenuItem { text: qsTr("Zoom to &Fit"); icon.source: "qrc:/Interface/Dolphin/zoomRange.svg"; onTriggered: AppController.zoomToFit() }
            }
            MenuSeparator {}
            MenuItem {
                text: qsTr("&Wires")
                icon.source: "qrc:/Interface/Toolbar/wires.svg"
                checkable: true
                checked: AppController.wiresVisible
                onTriggered: AppController.wiresVisible = checked
            }
            MenuItem {
                text: qsTr("&Gates")
                icon.source: "qrc:/Components/Logic/nor.svg"
                checkable: true
                checked: AppController.gatesVisible
                onTriggered: AppController.gatesVisible = checked
            }
            MenuSeparator {}
            MenuItem {
                text: qsTr("Show Minimap")
                checkable: true
                checked: AppController.minimap.visible
                onTriggered: AppController.minimap.visible = checked
            }
            MenuSeparator {}
            Menu {
                title: qsTr("&Theme")
                MenuItem { text: qsTr("&System"); checkable: true; checked: AppController.theme === 2; onTriggered: AppController.theme = 2 }
                MenuItem { text: qsTr("&Light"); checkable: true; checked: AppController.theme === 0; onTriggered: AppController.theme = 0 }
                MenuItem { text: qsTr("&Dark"); checkable: true; checked: AppController.theme === 1; onTriggered: AppController.theme = 1 }
            }
            MenuItem {
                text: qsTr("F&ullscreen")
                checkable: true
                checked: window.visibility === Window.FullScreen
                onTriggered: window.visibility = checked ? Window.FullScreen : Window.Windowed
            }
            MenuItem {
                text: qsTr("Labels under icons")
                checkable: true
                checked: AppController.labelsUnderIcons
                onTriggered: AppController.labelsUnderIcons = checked
            }
            MenuItem {
                text: qsTr("Show IC Preview")
                checkable: true
                checked: AppController.icPreviewEnabled
                onTriggered: AppController.icPreviewEnabled = checked
            }
        }

        Menu {
            title: qsTr("Sim&ulation")
            MenuItem {
                text: qsTr("&Play/Pause")
                icon.source: AppController.simulationRunning
                    ? "qrc:/Interface/Toolbar/pause.svg" : "qrc:/Interface/Toolbar/play.svg"
                checkable: true
                checked: AppController.simulationRunning
                onTriggered: AppController.simulationRunning = checked
            }
            MenuItem { text: qsTr("&Restart"); icon.source: "qrc:/Interface/Toolbar/reset.svg"; onTriggered: AppController.restartSimulation() }
            MenuItem { text: qsTr("&Waveform"); icon.source: "qrc:/Interface/Toolbar/dolphin_icon.svg"; onTriggered: AppController.openWaveform() }
            MenuItem {
                text: qsTr("Mute")
                checkable: true
                checked: AppController.muted
                onTriggered: AppController.muted = checked
            }
            MenuSeparator {}
            MenuItem {
                text: qsTr("Background Simulation")
                checkable: true
                checked: AppController.backgroundSimulationEnabled
                onTriggered: AppController.backgroundSimulationEnabled = checked
            }
        }

        Menu {
            title: qsTr("Examples")
            Repeater {
                model: AppController.examplesList()
                MenuItem {
                    required property exampleEntry modelData
                    text: modelData.title
                    onTriggered: AppController.openRecentFile(modelData.path)
                }
            }
        }

        Menu {
            title: qsTr("&Learn")
            Menu {
                id: exercisesMenu
                title: qsTr("&Exercises")
                // Re-scanned on every open (not just once, unlike examplesList()'s bundled-only
                // precedent): mirrors MainWindow::setupExercisesMenu()'s aboutToShow-triggered
                // rescan, since discover() also picks up custom content a teacher/user may have
                // dropped into a real folder while the app is running.
                onAboutToShow: exercisesRepeater.model = AppController.exercisesList()
                MenuItem {
                    text: qsTr("Open My Exercises Folder")
                    onTriggered: AppController.openMyContentFolder("Exercises")
                }
                // Only meaningful as a divider when there's something below it to divide from --
                // mirrors populateContentMenu()'s own "if (!entries.isEmpty()) addSeparator()".
                MenuSeparator { visible: exercisesRepeater.count > 0 }
                Repeater {
                    id: exercisesRepeater
                    model: []
                    MenuItem {
                        required property learnEntry modelData
                        text: (modelData.completed ? "✓ " : "") + modelData.title
                        ToolTip.text: modelData.description
                        ToolTip.visible: hovered && modelData.description.length > 0
                        onTriggered: AppController.exercise.start(modelData.path)
                    }
                }
            }
            Menu {
                id: toursMenu
                title: qsTr("&Tours")
                // Mirrors exercisesMenu's own aboutToShow-triggered rescan, MainWindow::
                // setupToursMenu()'s equivalent -- ExerciseTourResources::discover("Tours")
                // picks up custom content the same way discover("Exercises") does above.
                onAboutToShow: toursRepeater.model = AppController.toursList()
                MenuItem {
                    text: qsTr("Open My Tours Folder")
                    onTriggered: AppController.openMyContentFolder("Tours")
                }
                MenuSeparator { visible: toursRepeater.count > 0 }
                Repeater {
                    id: toursRepeater
                    model: []
                    MenuItem {
                        required property learnEntry modelData
                        text: (modelData.completed ? "✓ " : "") + modelData.title
                        ToolTip.text: modelData.description
                        ToolTip.visible: hovered && modelData.description.length > 0
                        onTriggered: AppController.tour.start(modelData.path)
                    }
                }
            }
        }

        Menu {
            title: qsTr("&Language")
            Repeater {
                model: AppController.languages
                MenuItem {
                    required property languageEntry modelData
                    text: modelData.displayName
                    icon.source: modelData.flagIcon
                    checkable: true
                    checked: AppController.currentLanguage === modelData.code
                    onTriggered: AppController.switchLanguage(modelData.code)
                }
            }
        }

        Menu {
            title: qsTr("&Help")
            MenuItem { text: qsTr("&About"); onTriggered: aboutDialog.open() }
            MenuItem { text: qsTr("About &Qt"); onTriggered: aboutQtDialog.open() }
            MenuItem { text: qsTr("About this version"); onTriggered: aboutThisVersionDialog.open() }
            MenuItem { text: qsTr("Shortcuts and Tips"); onTriggered: shortcutsDialog.open() }
            MenuSeparator {}
            // Mirrors MainWindowUi's checkable actionCheckForUpdates exactly -- an auto-check
            // preference toggle, not a manual "check now" trigger (Widgets has no such action
            // either: MainWindow::show() always calls checkForUpdates() once at startup,
            // unconditionally, gated internally by this same setting via
            // UpdateChecker::checkForUpdates()).
            MenuItem {
                text: qsTr("Check for updates automatically")
                checkable: true
                checked: AppController.updateController.autoCheckEnabled
                onTriggered: AppController.updateController.autoCheckEnabled = checked
            }
            MenuSeparator {}
            MenuItem { text: qsTr("Report Translation Error"); onTriggered: AppController.reportTranslationError() }
        }
    }

    // mainToolBar spans the full window width in Widgets too (MainWindow::addToolBar(Qt::
    // TopToolBarArea, mainToolBar) -- a real QToolBar, not scoped to the canvas splitter pane).
    // Button set/order/icons mirror mainToolBar->addAction(...)'s exact sequence; every action
    // already exists as a plain AppController method/property (the menu above calls the same
    // ones), so this is pure UI-surface work, not new functionality. The canvas tab strip,
    // unlike this toolbar, is NOT full-width in Widgets (tab = new QTabWidget(splitter) --
    // scoped to the splitter's canvas pane) -- see its own doc comment further down, next to
    // canvasHost, for where it actually lives now.
    header: ColumnLayout {
        spacing: 0

        ToolBar {
            Layout.fillWidth: true
            // Set once here, not per-ToolButton below: unlike Menu (see paletteDisabledText's
            // own doc comment), a plain Control like ToolBar correctly cascades its own
            // palette.disabled.* to every descendant Control reading control.palette.buttonText.
            // Without it, a disabled ToolButton (Undo/Redo with nothing to undo/redo, most
            // visibly) would render in the exact same full-brightness color as an enabled one.
            palette.disabled.buttonText: window.paletteDisabledText
            // Widgets' toolbar labels are 0.75x the app's default font (ThemeManager.cpp's
            // setStyleSheet("QToolBar QToolButton { font-size: %1pt }", Application::instance()
            // ->font().pointSizeF() * 0.75) -- Quick Controls' own ToolButton default was
            // noticeably smaller *still*, not just un-shrunk. `font` propagates from a Control
            // to its descendant Controls the same way `palette` does (Fusion's own
            // ToolButton.qml: `contentItem: IconLabel { font: control.font }`), so setting it
            // once here reaches every ToolButton below.
            // window.font (not Qt.application.font -- qmllint can't see QQuickApplication's own
            // `font` property through the base QQmlApplication type Qt.application statically
            // resolves to) is this window's own, otherwise-untouched font, i.e. the same
            // platform default Application::instance()->font() reads on the Widgets side.
            font.pointSize: Math.round(window.font.pointSize * 0.75)

            RowLayout {
                anchors.fill: parent
                spacing: 0

                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/new.svg"
                    text: qsTr("New")
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.newTab()
                }
                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/folder.svg"
                    text: qsTr("Open")
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.openFile()
                }
                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/save.svg"
                    text: qsTr("Save")
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.saveFile()
                }

                ToolSeparator {}

                // Inserted here (not after New/Open/Save) to mirror
                // SceneUiBinder::bind()'s own mainToolBar->insertAction(actionRotateLeft, ...)
                // calls for scene->undoAction()/redoAction() -- both land right before Rotate
                // Left, i.e. after this first separator, on every real tab bind.
                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/undo.svg"
                    text: qsTr("Undo")
                    enabled: AppController.canUndo
                    // Widgets' disabled QIcon is auto-generated by the style as a desaturated,
                    // washed-out pixmap -- icon.source here is just a plain full-color SVG Image
                    // with no such automatic treatment, so the icon itself stayed full-brightness
                    // even once the label text above correctly dimmed. A flat opacity dim isn't
                    // pixel-identical to QIcon's real disabled generation, but reads the same way
                    // at a glance and needs no shader/color-overlay effect.
                    opacity: enabled ? 1.0 : 0.5
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.undo()
                }
                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/redo.svg"
                    text: qsTr("Redo")
                    enabled: AppController.canRedo
                    opacity: enabled ? 1.0 : 0.5
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.redo()
                }
                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/rotateL.svg"
                    text: qsTr("Rotate left")
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.rotateLeft()
                }
                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/rotateR.svg"
                    text: qsTr("Rotate right")
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.rotateRight()
                }
                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/cut.svg"
                    text: qsTr("Cut")
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.cut()
                }
                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/copy.svg"
                    text: qsTr("Copy")
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.copy()
                }
                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/paste.svg"
                    text: qsTr("Paste")
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.paste()
                }
                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/delete.svg"
                    text: qsTr("Delete")
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.deleteSelection()
                }

                ToolSeparator {}

                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/zoomIn.svg"
                    text: qsTr("Zoom in")
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.zoomIn()
                }
                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/zoomOut.svg"
                    text: qsTr("Zoom out")
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.zoomOut()
                }
                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/zoomReset.svg"
                    text: qsTr("Reset Zoom")
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.resetZoom()
                }
                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Dolphin/zoomRange.svg"
                    text: qsTr("Zoom to Fit")
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.zoomToFit()
                }

                ToolSeparator {}

                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    // Two-state icon, matching actionPlay's QIcon::On/Off pair in
                    // MainWindowUI.cpp: play.svg unchecked, pause.svg checked.
                    icon.source: AppController.simulationRunning
                        ? "qrc:/Interface/Toolbar/pause.svg" : "qrc:/Interface/Toolbar/play.svg"
                    text: qsTr("Play/Pause")
                    checkable: true
                    checked: AppController.simulationRunning
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.simulationRunning = checked
                }
                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/reset.svg"
                    text: qsTr("Restart")
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.restartSimulation()
                }
                ToolButton {
                    display: AppController.labelsUnderIcons ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
                    icon.source: "qrc:/Interface/Toolbar/dolphin_icon.svg"
                    text: qsTr("Waveform")
                    ToolTip.text: text
                    ToolTip.visible: hovered
                    onClicked: AppController.openWaveform()
                }

                Item { Layout.fillWidth: true }
            }
        }
    }

    // Mirrors QStatusBar's own layout: a transient auto-clearing message
    // (AppController.statusMessage/showStatusMessage()) on the left, and a permanent
    // zoom/selection indicator (AppController.statusInfo, mirroring
    // SceneUiBinder::updateStatusInfo()'s addPermanentWidget() label) on the right. The bar
    // itself is always present, matching QStatusBar's own persistent dock -- only the
    // transient text on the left is sometimes empty.
    footer: Rectangle {
        implicitHeight: statusRow.implicitHeight
        color: window.palette.window

        RowLayout {
            id: statusRow
            anchors.fill: parent
            spacing: 6

            Label {
                Layout.fillWidth: true
                leftPadding: 6
                topPadding: 2
                bottomPadding: 2
                text: AppController.statusMessage
            }
            Label {
                rightPadding: 6
                topPadding: 2
                bottomPadding: 2
                text: AppController.statusInfo
            }
        }
    }

    // Mirrors MainWindowUI.cpp's splitter (leftPannel = palette + element editor stacked
    // vertically | tab = canvas), including the draggable handle and leftPannel's 280-600px
    // clamp / canvas's 570px floor.
    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        // Widgets' splitter has setHandleWidth(8) and Fusion's default QSplitterHandle paints
        // a small dotted grip in the middle of it -- a low-contrast 3-dot grip only 6px tall,
        // deep in the middle of a mostly plain-colored strip, reads as "not obviously
        // interactive" to most users. Deliberately not 1:1 with Widgets here, for a more
        // discoverable affordance:
        // - the whole handle band is tinted with palette.highlight at all times (not just on
        //   hover/press), so the seam between the two panes reads as a distinct, interactive
        //   strip on sight, the way VS Code's splitters do -- rather than blending into the
        //   plain mid-gray borders used everywhere else in this window.
        // - the grip itself is a denser 2-column x 4-row dot grid (the common macOS/VS Code
        //   drag-handle look) at roughly 3x the old dots' size, not 3 dots in a single column.
        handle: Rectangle {
            implicitWidth: 10
            // `palette` (bare, self-referential) rather than the outer `window` id: it's
            // inherited from ApplicationWindow via Item.palette propagation regardless, and
            // staying self-referential avoids needing "pragma ComponentBehavior: Bound" (and
            // the outer-id-capture rewrite that would mean for every other nested Component in
            // this file) just for this one handle delegate.
            color: SplitHandle.pressed ? palette.highlight
                : (SplitHandle.hovered ? Qt.lighter(palette.highlight, 1.2)
                    : Qt.rgba(palette.highlight.r, palette.highlight.g, palette.highlight.b, 0.35))

            Grid {
                anchors.centerIn: parent
                columns: 2
                rowSpacing: 3
                columnSpacing: 3
                Repeater {
                    model: 8
                    Rectangle { width: 3; height: 3; radius: 1.5; color: palette.buttonText }
                }
            }
        }

        ColumnLayout {
            SplitView.preferredWidth: 280
            SplitView.minimumWidth: 280
            SplitView.maximumWidth: 600
            spacing: 0

            ElementPalette {
                id: elementPalette
                Layout.fillWidth: true
                Layout.fillHeight: true
                canvasWidth: canvasHost.width
                canvasHeight: canvasHost.height
            }

            ElementEditor {
                id: elementEditor
                Layout.fillWidth: true
            }
        }

        // Canvas tab strip + canvas, stacked vertically -- this is the SplitView's right-side
        // "tab" pane from MainWindowUI.cpp (`tab = new QTabWidget(splitter)`), i.e. scoped to
        // this pane's own width, not the full window (unlike mainToolBar above).
        // Wrapped in a Frame for the same reason as ElementPalette.qml's tabElements Frame:
        // `tab` (MainWindowUI.cpp) is a real QTabWidget, whose Fusion style draws a border
        // around the tab strip + pane as one unit.
        Frame {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 570
            padding: 0

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                TabBar {
                    id: tabBar
                    Layout.fillWidth: true
                    visible: AppController.tabCount > 0
                    currentIndex: AppController.currentIndex
                    onCurrentIndexChanged: AppController.currentIndex = currentIndex

                    Repeater {
                        model: AppController.tabCount
                        // tab->setTabsClosable(true) (MainWindowUI.cpp): a close (X) button on
                        // every tab -- Quick Controls' own TabButton has no built-in close
                        // affordance, so this adds one as a plain child Item (not routed
                        // through contentItem, so the label/background stay the stock
                        // TabButton look).
                        delegate: TabButton {
                            id: tabButton
                            required property int index
                            // AppController.tabCount is read here purely so this binding has a
                            // NOTIFY-backed dependency to re-evaluate on: tabAt()/tabTitle() are
                            // plain invokables (no property-change tracking QML can hook into),
                            // so without this the label would never refresh after moveTab()
                            // reorders the underlying list (tabsChanged, which also drives
                            // tabCount, fires on every add/remove/reorder).
                            text: AppController.tabCount > 0 ? AppController.tabTitle(AppController.tabAt(index)) : ""
                            // Mirrors WorkspaceManager::setCurrentFile()'s setTabToolTip() --
                            // shows the full path when the tab title (possibly truncated, or
                            // ambiguous if two open files share a basename) doesn't say enough.
                            ToolTip.text: AppController.tabCount > 0 ? AppController.tabFilePath(AppController.tabAt(index)) : ""
                            ToolTip.visible: hovered && ToolTip.text.length > 0
                            // Fusion TabButton.qml's own default (padding: 2, horizontalPadding: 4)
                            // renders a ~20px-tall tab -- the reference build's real QTabBar tabs
                            // (Fusion widget style, not the QQC2 Fusion style above) measure ~41px
                            // tall, roughly double. Widened slightly too (~115px vs ~129px at
                            // matching content) to close most of the rest of the gap.
                            topPadding: 12
                            bottomPadding: 12
                            leftPadding: 8
                            rightPadding: closeButton.width + 8
                            // QQuickTabBar's own doc comment: "By default, TabBar resizes its
                            // buttons to fit the width of the control... overridden by setting
                            // an explicit width for the buttons" -- without this, a single tab
                            // stretches to fill the whole bar (unlike QTabWidget's compact,
                            // content-sized tab chip), putting closeButton below at the far
                            // edge of the window instead of right next to the label.
                            width: implicitWidth

                            // tab->setMovable(true) (MainWindowUI.cpp): drag a tab to reorder
                            // it. Qt Widgets gets live drag-and-reflow for free from QTabBar's
                            // own built-in support; TabBar's contentItem here is a ListView
                            // that positions each delegate itself, so fighting that to get the
                            // same live-follow-the-cursor animation would mean re-implementing
                            // the ListView's own layout. This instead commits the reorder on
                            // release, based on whichever tab is under the drop point -- same
                            // end result (drag a tab, tabs end up reordered), simpler mechanism.
                            // `target: null` -- this only reads the gesture, it doesn't drive
                            // tabButton's own position.
                            DragHandler {
                                id: dragHandler
                                target: null
                                onActiveChanged: {
                                    if (dragHandler.active) {
                                        return
                                    }
                                    // TabBar.tabBar/TabBar.index (QtQuick.Templates'
                                    // TabBar-attached properties, automatic on any Control
                                    // parented under a TabBar) reach the owning TabBar and
                                    // this tab's own index without an outer-id capture, which
                                    // would need "pragma ComponentBehavior: Bound" (and the
                                    // rewrite that implies for every other nested Component in
                                    // this file) just for this one delegate.
                                    const bar = tabButton.TabBar.tabBar
                                    const myIndex = tabButton.TabBar.index
                                    for (let i = 0; i < bar.count; ++i) {
                                        if (i === myIndex) {
                                            continue
                                        }
                                        const other = bar.itemAt(i)
                                        if (!other) {
                                            continue
                                        }
                                        const p = tabButton.mapToItem(
                                            other, dragHandler.centroid.position.x, dragHandler.centroid.position.y)
                                        if (p.x >= 0 && p.x <= other.width && p.y >= 0 && p.y <= other.height) {
                                            AppController.moveTab(myIndex, i)
                                            break
                                        }
                                    }
                                }
                            }

                            // tab->tabCloseRequested is also wired to a middle-click anywhere on
                            // the tab in Widgets (WorkspaceManager's own tab-bar eventFilter()),
                            // not just the close (X) button -- mirrored here as a separate
                            // TapHandler so it doesn't interfere with DragHandler's left-button
                            // reorder gesture above or closeButton's own left-click below.
                            TapHandler {
                                acceptedButtons: Qt.MiddleButton
                                onTapped: AppController.closeTab(tabButton.index)
                            }

                            ToolButton {
                                id: closeButton
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.rightMargin: 2
                                implicitWidth: 22
                                implicitHeight: 22
                                text: "✕"
                                // Widgets' close tab uses QTabBar's own built-in icon (a real,
                                // fixed-pixel-size close glyph from PM_TabCloseIndicatorWidth,
                                // independent of the tab's text font) -- that icon renders ~17px
                                // versus this button's plain "✕" text glyph at the app's default
                                // font rendering ~10px, well under half the size. Not derived from
                                // window.font like the toolbar fix above (that base font size is
                                // already correct -- "New Project" reads the same size in both
                                // builds) -- this glyph specifically needs its own larger size to
                                // match Widgets' fixed-size icon.
                                font.pixelSize: 16
                                onClicked: AppController.closeTab(tabButton.index)
                            }
                        }
                    }
                }

                // Hosts the current tab's CanvasItem; reparented here on every tab switch (see
                // Connections below) since each open QuickWorkSpace owns its own CanvasItem
                // instance rather than one shared canvas whose content gets swapped.
                Item {
                    id: canvasHost
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    function showCurrentTab() {
                        const tab = AppController.currentTab
                        if (!tab) {
                            return
                        }
                        const canvas = tab.canvas()
                        canvas.parent = canvasHost
                        canvas.anchors.fill = canvasHost
                        canvas.forceActiveFocus()
                    }

                    Component.onCompleted: showCurrentTab()

                    Connections {
                        target: AppController
                        function onCurrentTabChanged() { canvasHost.showCurrentTab() }
                    }

                    // Drop target for ElementPalette.qml's PaletteItemDelegate instances -- drop.source
                    // is the dragged delegate itself (Drag.source defaults to the Item the Drag
                    // attached property is set on), so its modelData carries everything
                    // addElementToCurrentTab() needs. Cast to PaletteItemDelegate (DragEvent.source's
                    // declared type is plain QObject, since a drag source can be any Item) so
                    // modelData resolves to a real property instead of an unqualified access.
                    // drop.x/drop.y are canvasHost-local (DragEvent coordinates are relative to the
                    // DropArea) -- i.e. CanvasItem's own screen space -- which addElementFromPalette()
                    // converts to world coordinates internally via screenToWorld(), so drops land
                    // correctly under whatever pan/zoom is active.
                    DropArea {
                        anchors.fill: parent
                        onDropped: (drop) => {
                            const item = drop.source as PaletteItemDelegate
                            if (item) {
                                AppController.addElementToCurrentTab(
                                    item.modelData.type, item.modelData.icFileName,
                                    item.modelData.isEmbedded, drop.x, drop.y)
                                return
                            }
                            // Not a palette drag (drop.source is null/not a PaletteItemDelegate for an
                            // OS-level file drag) -- a .panda file dragged from the file manager opens
                            // as a project, like File > Open. Mirrors Scene::dropEvent()'s
                            // droppedPandaFile() check; validation itself lives in C++
                            // (openDroppedPandaFile()), not here, matching this chrome's own
                            // no-business-logic-in-QML convention.
                            if (drop.hasUrls) {
                                AppController.openDroppedPandaFile(drop.urls)
                            }
                        }
                    }

                    // Right-click / inline-label-edit gestures. target re-evaluates whenever
                    // currentTab changes since the expression reads AppController.currentTab (a
                    // property) before calling the plain .canvas() method.
                    Connections {
                        target: AppController.currentTab ? AppController.currentTab.canvas() : null

                        function onElementContextMenuRequested(element, pos) {
                            AppController.elementEditor.prepareContextMenu(element)
                            elementContextMenu.popup(pos.x, pos.y)
                        }

                        function onEmptyContextMenuRequested(pos) {
                            emptyContextMenu.popup(pos.x, pos.y)
                        }

                        function onInlineEditRequested(element, currentLabel, targetRect) {
                            inlineLabelEditor.targetElement = element
                            inlineLabelEditor.text = currentLabel
                            inlineLabelEditor.x = targetRect.x
                            inlineLabelEditor.y = targetRect.y
                            inlineLabelEditor.width = Math.max(80, targetRect.width)
                            inlineLabelEditor.visible = true
                            inlineLabelEditor.forceActiveFocus()
                            inlineLabelEditor.selectAll()
                        }
                    }

                    Menu {
                        id: elementContextMenu

                        MenuItem {
                            text: qsTr("Rename")
                            icon.source: "qrc:/Interface/Toolbar/rename.svg"
                            visible: AppController.elementEditor.labelVisible
                            onTriggered: AppController.elementEditor.requestRenameFocus()
                        }
                        MenuItem {
                            text: qsTr("Change trigger")
                            icon.source: "qrc:/Components/Input/buttonOff.svg"
                            visible: AppController.elementEditor.triggerVisible
                            onTriggered: AppController.elementEditor.requestTriggerFocus()
                        }
                        MenuItem {
                            text: qsTr("Change appearance to ...")
                            visible: AppController.elementEditor.appearanceVisible
                            onTriggered: AppController.elementEditor.changeAppearance()
                        }
                        MenuItem {
                            text: qsTr("Restore default appearance")
                            visible: AppController.elementEditor.appearanceVisible
                            onTriggered: AppController.elementEditor.resetAppearance()
                        }
                        MenuItem { text: qsTr("Rotate left"); icon.source: "qrc:/Interface/Toolbar/rotateL.svg"; onTriggered: AppController.rotateLeft() }
                        MenuItem { text: qsTr("Rotate right"); icon.source: "qrc:/Interface/Toolbar/rotateR.svg"; onTriggered: AppController.rotateRight() }
                        MenuItem { text: qsTr("Flip horizontally"); onTriggered: AppController.flipHorizontal() }
                        MenuItem { text: qsTr("Flip vertically"); onTriggered: AppController.flipVertical() }
                        MenuItem {
                            text: qsTr("Change frequency")
                            icon.source: "qrc:/Components/Input/clock1.svg"
                            visible: AppController.elementEditor.frequencyVisible
                            onTriggered: AppController.elementEditor.requestFrequencyFocus()
                        }
                        Menu {
                            title: qsTr("Change color to...")
                            // Mirrors ElementContextMenu.cpp:83's own structural add-only-when-
                            // applicable: Widgets never renders this row at all for a
                            // non-colorable element, it doesn't just gray it out.
                            visible: AppController.elementEditor.colorVisible
                            palette.disabled.text: window.paletteDisabledText
                            Repeater {
                                // Mirrors ElementContextMenu.cpp:83-88's own `colorCombo->
                                // currentIndex() != i` filter: the element's own current color
                                // is a redundant no-op entry here, unlike the property panel's
                                // ComboBox (ElementEditor.qml), which needs the full list to
                                // show the current color as already selected.
                                model: AppController.elementEditor.colorOptions.filter(
                                    c => c.name !== AppController.elementEditor.color)
                                MenuItem {
                                    required property var modelData
                                    text: modelData.translatedName
                                    onTriggered: AppController.elementEditor.color = modelData.name
                                }
                            }
                        }
                        Menu {
                            title: qsTr("Morph to...")
                            // Same structural-absence reasoning as "Change color to..." above --
                            // mirrors ElementContextMenu.cpp:94-166's own submenu-removed-if-empty
                            // handling for a non-morphable element (e.g. no morph candidates).
                            visible: AppController.elementEditor.canMorph && AppController.elementEditor.morphCandidates.length > 0
                            palette.disabled.text: window.paletteDisabledText
                            Repeater {
                                model: AppController.elementEditor.morphCandidates
                                MenuItem {
                                    required property var modelData
                                    text: modelData.name
                                    onTriggered: AppController.elementEditor.morphSelectionTo(modelData.type)
                                }
                            }
                        }
                        MenuItem {
                            text: qsTr("Edit sub-circuit")
                            visible: AppController.elementEditor.editSubcircuitVisible
                            onTriggered: AppController.editSelectedSubcircuit()
                        }
                        MenuItem {
                            text: qsTr("Embed sub-circuit")
                            visible: AppController.elementEditor.embedIcVisible
                            onTriggered: AppController.embedSelectedIC()
                        }
                        MenuItem {
                            text: qsTr("Extract to file")
                            visible: AppController.elementEditor.extractIcVisible
                            onTriggered: AppController.extractSelectedIC()
                        }
                        MenuSeparator {}
                        MenuItem { text: qsTr("Copy"); icon.source: "qrc:/Interface/Toolbar/copy.svg"; onTriggered: AppController.copy() }
                        MenuItem { text: qsTr("Cut"); icon.source: "qrc:/Interface/Toolbar/cut.svg"; onTriggered: AppController.cut() }
                        MenuItem { text: qsTr("Delete"); icon.source: "qrc:/Interface/Toolbar/delete.svg"; onTriggered: AppController.deleteSelection() }
                    }

                    // Right-click on empty canvas: mirrors Scene::contextMenu()'s "no item" branch.
                    Menu {
                        id: emptyContextMenu

                        MenuItem {
                            text: qsTr("Paste")
                            icon.source: "qrc:/Interface/Toolbar/paste.svg"
                            enabled: AppController.canPaste()
                            palette.disabled.text: window.paletteDisabledText
                            onTriggered: AppController.paste()
                        }
                        MenuItem {
                            text: qsTr("Select all")
                            enabled: AppController.hasElements
                            palette.disabled.text: window.paletteDisabledText
                            onTriggered: AppController.selectAll()
                        }
                    }

                    // Inline label editing (double-click an element with a label). Positioned/sized by
                    // onInlineEditRequested above; canvas-local coordinates match canvasHost directly.
                    TextField {
                        id: inlineLabelEditor
                        visible: false
                        z: 1000

                        property var targetElement: null

                        function commit() {
                            if (targetElement) {
                                AppController.currentTab.canvas().commitInlineLabelEdit(targetElement, text)
                                targetElement = null
                            }
                            visible = false
                        }

                        function cancelEdit() {
                            targetElement = null
                            visible = false
                        }

                        onEditingFinished: commit()
                        Keys.onEscapePressed: cancelEdit()
                    }

                    // IC hover preview. Reparents itself to the window's content area (see its own
                    // parent: binding) since its screenPos is a global coordinate, not canvasHost-local
                    // -- declared here only for proximity to the other canvasHost-adjacent overlays
                    // above.
                    ICPreviewPopup {}

                    // Circuit-overview minimap -- see Minimap.qml's own doc comment. Genuinely
                    // canvasHost-local (unlike ICPreviewPopup), so no reparenting is needed here.
                    Minimap {}

                    // Circuit-exercise step overlay -- see ExerciseOverlay.qml's own doc comment.
                    // Genuinely canvasHost-local, same as Minimap.
                    ExerciseOverlay {}

                    // GraphicsView is a real QAbstractScrollArea, with always-visible scrollbars
                    // (WidgetsView -> QGraphicsView default scrollbar policy); this pan/zoom-only
                    // canvas has no Flickable underneath for QML's ScrollBar to attach to
                    // automatically, so both bars bind to CanvasItem's own scroll* properties by
                    // hand (see their own doc comment for the position/size semantics). The
                    // Binding elements' `when: !pressed` is what keeps this properly two-way:
                    // while not being dragged, the bar tracks the canvas (so panning by any
                    // other means -- mouse-drag, the minimap, zoom-to-fit -- moves the thumb
                    // too); while dragged, the Binding steps aside and onPositionChanged pushes
                    // the drag back into the canvas instead. ThickScrollBar (not the bare
                    // QtQuick.Controls ScrollBar) for width -- see its own doc comment.
                    ThickScrollBar {
                        id: hScrollBar
                        z: 1500 // above the reparented CanvasItem, same reasoning as Minimap/ExerciseOverlay
                        anchors.left: parent.left
                        anchors.right: vScrollBar.left
                        anchors.bottom: parent.bottom
                        orientation: Qt.Horizontal
                        policy: ScrollBar.AlwaysOn
                        visible: AppController.currentTab !== null

                        Binding on position {
                            value: AppController.currentTab ? AppController.currentTab.canvas().horizontalScrollPosition : 0
                            when: !hScrollBar.pressed
                        }
                        onPositionChanged: {
                            if (hScrollBar.pressed && AppController.currentTab) {
                                AppController.currentTab.canvas().horizontalScrollPosition = position
                            }
                        }
                        size: AppController.currentTab ? AppController.currentTab.canvas().horizontalScrollSize : 1
                    }

                    ThickScrollBar {
                        id: vScrollBar
                        z: 1500
                        anchors.top: parent.top
                        anchors.bottom: hScrollBar.top
                        anchors.right: parent.right
                        orientation: Qt.Vertical
                        policy: ScrollBar.AlwaysOn
                        visible: AppController.currentTab !== null

                        Binding on position {
                            value: AppController.currentTab ? AppController.currentTab.canvas().verticalScrollPosition : 0
                            when: !vScrollBar.pressed
                        }
                        onPositionChanged: {
                            if (vScrollBar.pressed && AppController.currentTab) {
                                AppController.currentTab.canvas().verticalScrollPosition = position
                            }
                        }
                        size: AppController.currentTab ? AppController.currentTab.canvas().verticalScrollSize : 1
                    }
                }
            }
        }
    }

    // Guided-interface-tour overlay. Unlike ExerciseOverlay/Minimap, this is a direct content
    // child, not a canvasHost child: it needs
    // to spotlight items outside canvasHost too (elementPalette, its category tabs, the search
    // bar, elementEditor). anchors.fill: parent covers the same content area the RowLayout
    // above does (below the menu bar/tab bar header, above footer). paletteItem/editorItem/
    // canvasAreaItem are passed down explicitly rather than reached for via an implicit
    // ancestor-id lookup -- same precedent as ElementPalette.qml's own canvasWidth/canvasHeight
    // properties -- so TourOverlay.qml's target-resolution/click-dispatch functions can be
    // statically typed against real item types instead of an untyped cross-file id reference
    // (qmllint can't see custom QML functions through the generic Window attached property's
    // QQuickWindow type, unlike a typed property).
    TourOverlay {
        anchors.fill: parent
        paletteItem: elementPalette
        editorItem: elementEditor
        canvasAreaItem: canvasHost
    }

    Dialog {
        id: shortcutsDialog
        title: qsTr("Shortcuts and Tips")
        modal: true
        standardButtons: Dialog.Ok
        anchors.centerIn: parent
        // A fixed width breaks the implicitWidth<->contentItem.implicitWidth binding loop a
        // bare, unconstrained Label as contentItem would otherwise create (Dialog sizes to
        // its content, content wraps to the Dialog's width -- circular without an anchor).
        width: 480
        contentItem: Label {
            text: AppController.shortcutsHelpHtml()
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            // availableWidth (= this Dialog's own fixed width above, minus its style's fixed
            // padding) rather than parent.width: parent.width resolves to the *content holder*'s
            // own implicit size, which for a Dialog with no explicit height is itself derived
            // from this same Label's implicitHeight -- a real, live binding loop for tall enough
            // content: About's own longer content hits it in practice ("Binding loop detected
            // for property 'implicitHeight'"), even though this dialog's own shorter table
            // content happens to converge without tripping the detector. availableWidth depends
            // only on this Dialog's fixed width/padding, never on contentItem, so it can't be
            // part of any such cycle.
            width: shortcutsDialog.availableWidth
        }
    }

    // Mirrors MainWindow::on_actionAbout_triggered()'s QMessageBox::about() exactly.
    Dialog {
        id: aboutDialog
        title: qsTr("wiRedPanda")
        modal: true
        standardButtons: Dialog.Ok
        anchors.centerIn: parent
        width: 480
        contentItem: Label {
            text: AppController.aboutHtml()
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            width: aboutDialog.availableWidth // see shortcutsDialog's own doc comment
            onLinkActivated: (link) => Qt.openUrlExternally(link)
        }
    }

    // Widgets shows Qt's own built-in QMessageBox::aboutQt() here -- see
    // QuickAppController::aboutQtHtml()'s own doc comment for why this is a smaller substitute,
    // not a line-for-line port.
    Dialog {
        id: aboutQtDialog
        title: qsTr("About Qt")
        modal: true
        standardButtons: Dialog.Ok
        anchors.centerIn: parent
        width: 480
        contentItem: Label {
            text: AppController.aboutQtHtml()
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            width: aboutQtDialog.availableWidth // see shortcutsDialog's own doc comment
            onLinkActivated: (link) => Qt.openUrlExternally(link)
        }
    }

    // Mirrors MainWindow::aboutThisVersion()'s QMessageBox exactly.
    Dialog {
        id: aboutThisVersionDialog
        title: qsTr("wiRedPanda") + " " + AppController.appVersion()
        modal: true
        standardButtons: Dialog.Ok
        anchors.centerIn: parent
        width: 480
        contentItem: Label {
            text: AppController.aboutThisVersionText()
            wrapMode: Text.WordWrap
            width: aboutThisVersionDialog.availableWidth // see shortcutsDialog's own doc comment
        }
    }

    // Opened via AppController.elementEditor.openTruthTable() (ElementEditor.qml's "Edit Truth
    // Table..." button), mirroring shortcutsDialog's own open()-by-id precedent.
    TruthTableDialog {
        id: truthTableDialog
    }
    Connections {
        target: AppController.elementEditor
        function onTruthTableRequested() { truthTableDialog.open() }
    }

    // Opened whenever a startup or manual update check finds a newer release, mirroring
    // shortcutsDialog/truthTableDialog's own open()-by-id precedent.
    UpdateDialog {
        id: updateDialog
    }
    Connections {
        target: AppController.updateController
        function onUpdateAvailable(latestVersion, downloadUrl, releaseUrl) {
            updateDialog.latestVersion = latestVersion
            updateDialog.downloadUrl = downloadUrl
            updateDialog.releaseUrl = releaseUrl
            updateDialog.open()
        }
    }

    // A single, reused top-level window (like truthTableDialog/shortcutsDialog above) rather
    // than a dynamically created-and-destroyed one -- AppController.openWaveform() decides
    // whether to rebuild the waveform (QuickDolphinController::createWaveform()) or just
    // re-show/raise an already-open one; this instance's own visible/close lifecycle only ever
    // hides it (DolphinWindow.qml's onClosing calls AppController.notifyWaveformClosed(), never
    // destroys the QML object), so the next openWaveform() has a real instance ready either way.
    DolphinWindow {
        id: dolphinWindow
        visible: false
        controller: AppController.dolphin
    }
    Connections {
        target: AppController
        function onWaveformOpenRequested() {
            dolphinWindow.visible = true;
            dolphinWindow.raise();
            dolphinWindow.requestActivate();
        }
    }
}
