// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickElementPalette: QML-facing data model backing the element palette panel.
 */

#pragma once

#include <QFileInfo>
#include <QObject>
#include <QQmlEngine>
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
 * updateICList()/updateEmbeddedICList() file-system/blob-registry scans, and (duplicated
 * rather than called, since App/UI/ElementPalette.cpp is WIDGETS_ONLY) the four-line
 * regex-escaped name match ElementPalette::nameMatchesSearch() implements.
 *
 * The three-pass search in ElementPalette::onSearchTextChanged() is reproduced: pass 1 matches
 * the untranslated internal type keyword (e.g. "and" -- ElementPalette.cpp's populateMenu()
 * sets this as every built-in ElementLabel's objectName, "label_" + name.toLower()), pass 2
 * matches the translated display name, and pass 3 matches a file-based IC's full file name
 * including its .panda extension ("label_ic" is set on every file-based IC label; embedded ICs
 * use a different objectName and are correctly excluded from this pass, same as production).
 * See describeBuiltin()'s "internalName" entry and recomputeSearchResults() for where each pass
 * lives here.
 */
class QuickElementPalette : public QObject
{
    Q_OBJECT
    // ElementPalette.qml spells this out as an explicit property type (property
    // QuickElementPalette controller: ...) so qmllint can check member access against it --
    // QML_ELEMENT gives it a name to do that with; QML_UNCREATABLE since QML never
    // constructs one itself (QuickAppController owns the only instance).
    QML_ELEMENT
    QML_UNCREATABLE("Created by QuickAppController")

    Q_PROPERTY(QVariantList ioElements READ ioElements CONSTANT FINAL)
    Q_PROPERTY(QVariantList gatesElements READ gatesElements CONSTANT FINAL)
    Q_PROPERTY(QVariantList combinationalElements READ combinationalElements CONSTANT FINAL)
    Q_PROPERTY(QVariantList memoryElements READ memoryElements CONSTANT FINAL)
    Q_PROPERTY(QVariantList miscElements READ miscElements CONSTANT FINAL)
    Q_PROPERTY(QVariantList icElements READ icElements NOTIFY icElementsChanged FINAL)
    Q_PROPERTY(QVariantList embeddedICElements READ embeddedICElements NOTIFY embeddedICElementsChanged FINAL)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged FINAL)
    Q_PROPERTY(QVariantList searchResults READ searchResults NOTIFY searchResultsChanged FINAL)

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
