// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief SceneUiBinder: wires the active tab's scene into the shared editor chrome.
 */

#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>

class ElementPalette;
class ICPreviewPopup;
class MainWindowUi;
class QLabel;
class QShortcut;
class QWidget;
class WorkSpace;

/**
 * \class SceneUiBinder
 * \brief Binds and unbinds the active WorkSpace's scene to the single-instance editor
 * chrome (element editor, Edit-menu undo/redo, scene-property shortcuts, and the
 * zoom/mute/play action state).
 *
 * \details Extracted from MainWindow. The chrome widgets are shared singletons that
 * must be re-pointed at the current scene on every tab switch; that wiring is this
 * class's sole responsibility. It owns the six scene-property QShortcuts (which only
 * ever target the active scene). Tab navigation triggered by scene signals
 * (icOpenRequested) and the file-based IC list stay with the tab owner.
 */
class SceneUiBinder : public QObject
{
    Q_OBJECT

public:
    /// \param ui Shared UI (element editor, menus, actions, status bar).
    /// \param palette Element palette (embedded-IC list reflects the active scene).
    /// \param previewPopup Shared IC hover-preview popup, driven off the bound scene's IC signals.
    /// \param shortcutParent Widget that parents the scene-property shortcuts (the window).
    SceneUiBinder(MainWindowUi *ui, ElementPalette *palette, ICPreviewPopup *previewPopup, QWidget *shortcutParent, QObject *parent = nullptr);

    /// Connects \a tab's scene/view/simulation to the chrome and syncs action state.
    void bind(WorkSpace *tab);

    /// Tears down the connections established by bind() for the currently bound tab.
    void unbind();

signals:
    /// The bound scene asked to open an embedded IC in a tab (forwarded to the tab owner).
    void openICRequested(const QString &blobName, int icElementId, const QByteArray &blob);
    /// The bound scene asked to open a file-based IC by path (forwarded to the tab owner).
    void loadFileRequested(const QString &filePath);

private:
    void addUndoRedoMenu();
    void removeUndoRedoMenu();
    void syncZoomActions();
    /// Refreshes the permanent status-bar indicator (zoom % and selection count) from the
    /// currently bound tab; clears it when no tab is bound.
    void updateStatusInfo();

    MainWindowUi *m_ui;
    ElementPalette *m_palette;
    ICPreviewPopup *m_previewPopup;
    QLabel *m_statusInfo = nullptr; ///< Permanent status-bar label: zoom % + selection count.
    WorkSpace *m_bound = nullptr;

    // Scene-property navigation shortcuts ( [ ] { } < > ), re-targeted to the active scene.
    QShortcut *m_prevMainPropShortcut;
    QShortcut *m_nextMainPropShortcut;
    QShortcut *m_prevSecndPropShortcut;
    QShortcut *m_nextSecndPropShortcut;
    QShortcut *m_changePrevElmShortcut;
    QShortcut *m_changeNextElmShortcut;
};
