// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ElementPalette: manages the left-panel element palette and search UI.
 */

#pragma once

#include <QFileInfo>
#include <QObject>
#include <QStringList>

class MainWindowUi;
class QLayout;
class QMimeData;
class QSpacerItem;
class Scene;

/**
 * \class ElementPalette
 * \brief Controller for the left-panel element palette, IC list, and search tab.
 *
 * \details Owns all state and logic for the left panel: populating element category
 * tabs with ElementLabel widgets, maintaining the IC file list, running the
 * three-pass search, and managing the search-tab lifecycle.
 *
 * The only cross-boundary action — dropping the first search result into the
 * scene — is exposed as addElementRequested() so MainWindow can route it to
 * the active scene without ElementPalette knowing about Scene.
 */
class ElementPalette : public QObject
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /**
     * \brief Constructs the palette controller.
     * \param ui     The host window's UI descriptor (must outlive this object).
     * \param parent Optional QObject parent.
     *
     * \details Immediately connects lineEditSearch signals to internal slots.
     */
    explicit ElementPalette(MainWindowUi *ui, QObject *parent = nullptr);

    // --- Palette Management ---

    /**
     * \brief Populates all category tabs with ElementLabel widgets and sets tab icons.
     * \details Must be called once after the host window is fully set up.
     */
    void populate();

    /**
     * \brief Refreshes the IC tab to reflect .panda files next to \a currentFile.
     * \param currentFile The currently open project file; the IC tab is cleared
     *                    if this file does not exist.
     */
    void updateICList(const QFileInfo &currentFile);

    /// Refreshes the embedded-IC section of the palette from the scene's blob registry.
    void updateEmbeddedICList(Scene *scene);

    /// Calls updateName() on every ElementLabel in the palette (on language change).
    void retranslateLabels();

    /// Updates the memory tab icon and all memory-tab ElementLabel themes.
    void updateTheme();

signals:
    /**
     * \brief Emitted when the user presses Enter in the search box.
     * \param mimeData MIME data of the first visible search result.
     *                 MainWindow should pass this to Scene::addItem().
     */
    void addElementRequested(QMimeData *mimeData);

private slots:
    void onSearchTextChanged(const QString &text);
    void onSearchReturnPressed();

private:
    // --- Helpers ---

    void populateMenu(QSpacerItem *spacer, const QStringList &names, QLayout *layout);
    void setupTabIcons();
    int  getTabIndex(const QString &objectName) const;

    // --- Members ---

    MainWindowUi *m_ui;                ///< The host window's UI descriptor (not owned).
    int m_lastTabIndex = -1;           ///< Tab index active before a search was initiated; restored on clear.
};

