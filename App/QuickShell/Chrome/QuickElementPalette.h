// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickElementPalette: QML-facing data model backing the element palette panel.
 */

#pragma once

#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include "App/Core/Enums.h"

class CanvasICRegistry;

/**
 * \class QuickElementPalette
 * \brief Copy-and-adapted port of App/UI/ElementPalette.h for the Quick chrome.
 *
 * \details ElementPalette's real job -- constructing ElementLabel QWidgets and manipulating
 * QLayout/QTabWidget directly -- has no Quick equivalent to adapt; a QML Repeater/GridView
 * over a data model replaces all of that widget-construction machinery outright. What DOES
 * carry over as real, reused logic: the five fixed category lists from populate(), the
 * updateICList()/updateEmbeddedICList() file-system/blob-registry scans, and
 * ElementPalette::nameMatchesSearch() itself (called directly -- it's already static and
 * needs no MainWindowUi, so there is no reason to duplicate its regex-escaping logic here).
 *
 * The three-pass search in ElementPalette::onSearchTextChanged() is NOT reproduced faithfully:
 * passes 1 and 3 match against QObject::objectName() values ("label_and", "label_ic") that no
 * code path in the shipped app ever actually sets on a freshly constructed ElementLabel --
 * confirmed by reading every ElementLabel/ElementPalette call site; only "label_embedded_ic"
 * is ever set, and that objectName is never checked by the search itself. Passes 1 and 3 are
 * therefore dead code in production: every real search result comes from pass 2 alone
 * (nameMatchesSearch() against ElementLabel::name(), which already holds the translated type
 * name / IC basename / embedded blob name, all uppercased where relevant in the constructor).
 * This class reproduces the real, observable behavior -- a single name-based match -- not the
 * unreachable object-name passes. See project memory project_element_palette_dead_search_passes.md.
 */
class QuickElementPalette : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList ioElements READ ioElements CONSTANT)
    Q_PROPERTY(QVariantList gatesElements READ gatesElements CONSTANT)
    Q_PROPERTY(QVariantList combinationalElements READ combinationalElements CONSTANT)
    Q_PROPERTY(QVariantList memoryElements READ memoryElements CONSTANT)
    Q_PROPERTY(QVariantList miscElements READ miscElements CONSTANT)
    Q_PROPERTY(QVariantList icElements READ icElements NOTIFY icElementsChanged)
    Q_PROPERTY(QVariantList embeddedICElements READ embeddedICElements NOTIFY embeddedICElementsChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(QVariantList searchResults READ searchResults NOTIFY searchResultsChanged)

public:
    explicit QuickElementPalette(QObject *parent = nullptr);

    // --- Fixed category lists (populated once at construction, mirroring populate()) ---

    [[nodiscard]] QVariantList ioElements() const { return m_ioElements; }
    [[nodiscard]] QVariantList gatesElements() const { return m_gatesElements; }
    [[nodiscard]] QVariantList combinationalElements() const { return m_combinationalElements; }
    [[nodiscard]] QVariantList memoryElements() const { return m_memoryElements; }
    [[nodiscard]] QVariantList miscElements() const { return m_miscElements; }

    // --- Dynamic lists ---

    [[nodiscard]] QVariantList icElements() const { return m_icElements; }
    [[nodiscard]] QVariantList embeddedICElements() const { return m_embeddedICElements; }

    /// Refreshes the IC list to reflect .panda files next to \a currentFile. Mirrors
    /// ElementPalette::updateICList(); the inline-IC-tab parent-chain walk
    /// WorkspaceManager::icListFile() does is not reproduced -- inline IC tabs have no UI
    /// trigger in the Quick chrome yet (a real, separate deferral, not an oversight).
    void updateICList(const QFileInfo &currentFile);

    /// Refreshes the embedded-IC list from \a registry's blob map, or clears it if \a registry
    /// is nullptr. Mirrors ElementPalette::updateEmbeddedICList().
    void updateEmbeddedICList(CanvasICRegistry *registry);

    // --- Search ---

    [[nodiscard]] QString searchText() const { return m_searchText; }
    void setSearchText(const QString &text);

    [[nodiscard]] QVariantList searchResults() const { return m_searchResults; }

    /// Returns the first search result's descriptor, or an empty map if searchResults is
    /// empty. Mirrors ElementPalette::onSearchReturnPressed()'s "add the first visible result".
    Q_INVOKABLE QVariantMap firstSearchResult() const;

signals:
    void icElementsChanged();
    void embeddedICElementsChanged();
    void searchTextChanged();
    void searchResultsChanged();

private:
    static QVariantMap describeBuiltin(ElementType type);
    static QVariantMap describeIC(const QString &filePath);
    static QVariantMap describeEmbeddedIC(const QString &blobName);

    /// Concatenation of every fixed/IC/embedded-IC entry -- the pool onSearchTextChanged()
    /// filters. Rebuilt on demand (icElements()/embeddedICElements() change rarely, and this
    /// class has no per-entry stable identity to invalidate a cache against).
    [[nodiscard]] QVariantList allEntries() const;
    void recomputeSearchResults();

    QVariantList m_ioElements;
    QVariantList m_gatesElements;
    QVariantList m_combinationalElements;
    QVariantList m_memoryElements;
    QVariantList m_miscElements;
    QVariantList m_icElements;
    QVariantList m_embeddedICElements;
    QString m_searchText;
    QVariantList m_searchResults;
};
