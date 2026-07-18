// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickElementPalette: QML-facing data model backing the element palette panel.
 */

#pragma once

#include <QFileInfo>
#include <QList>
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QVariantMap>

#include "App/Core/Enums.h"

class CanvasICRegistry;

/// One entry in any of QuickElementPalette's category/search lists (a builtin gate, an
/// IC file, or an embedded IC). A real QML value type -- not QVariantMap -- so a
/// delegate's `required property elementDescriptor modelData` gives qmlcachegen a
/// concrete field layout to compile against instead of falling back to the interpreter
/// for every `modelData.*` read. See project_qml_aot_compilation_fusion_style_pin memory.
class ElementDescriptor
{
    Q_GADGET
    QML_VALUE_TYPE(elementDescriptor)

    Q_PROPERTY(int type READ type FINAL)
    Q_PROPERTY(QString name READ name FINAL)
    Q_PROPERTY(QString icFileName READ icFileName FINAL)
    Q_PROPERTY(bool isEmbedded READ isEmbedded FINAL)
    Q_PROPERTY(QString iconSource READ iconSource FINAL)
    Q_PROPERTY(QString tooltip READ tooltip FINAL)

public:
    ElementDescriptor() = default;
    ElementDescriptor(int type, QString name, QString icFileName, bool isEmbedded, QString iconSource, QString tooltip)
        : m_type(type)
        , m_name(std::move(name))
        , m_icFileName(std::move(icFileName))
        , m_isEmbedded(isEmbedded)
        , m_iconSource(std::move(iconSource))
        , m_tooltip(std::move(tooltip))
    {
    }

    [[nodiscard]] int type() const { return m_type; }
    [[nodiscard]] QString name() const { return m_name; }
    [[nodiscard]] QString icFileName() const { return m_icFileName; }
    [[nodiscard]] bool isEmbedded() const { return m_isEmbedded; }
    [[nodiscard]] QString iconSource() const { return m_iconSource; }
    [[nodiscard]] QString tooltip() const { return m_tooltip; }

private:
    int m_type = 0;
    QString m_name;
    QString m_icFileName;
    bool m_isEmbedded = false;
    QString m_iconSource;
    QString m_tooltip;
};

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
    QML_ELEMENT
    QML_UNCREATABLE("QuickElementPalette is only ever exposed via AppController.elementPalette")

    // FINAL throughout: QuickElementPalette is never subclassed, so QML's shadow-check
    // can trust these types for chained lookups instead of falling back to untyped
    // "var" -- see project_qml_aot_compilation_fusion_style_pin memory.
    Q_PROPERTY(QList<ElementDescriptor> ioElements READ ioElements CONSTANT FINAL)
    Q_PROPERTY(QList<ElementDescriptor> gatesElements READ gatesElements CONSTANT FINAL)
    Q_PROPERTY(QList<ElementDescriptor> combinationalElements READ combinationalElements CONSTANT FINAL)
    Q_PROPERTY(QList<ElementDescriptor> memoryElements READ memoryElements CONSTANT FINAL)
    Q_PROPERTY(QList<ElementDescriptor> miscElements READ miscElements CONSTANT FINAL)
    Q_PROPERTY(QList<ElementDescriptor> icElements READ icElements NOTIFY icElementsChanged FINAL)
    Q_PROPERTY(QList<ElementDescriptor> embeddedICElements READ embeddedICElements NOTIFY embeddedICElementsChanged FINAL)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged FINAL)
    Q_PROPERTY(QList<ElementDescriptor> searchResults READ searchResults NOTIFY searchResultsChanged FINAL)

public:
    explicit QuickElementPalette(QObject *parent = nullptr);

    // --- Fixed category lists (populated once at construction, mirroring populate()) ---

    [[nodiscard]] QList<ElementDescriptor> ioElements() const { return m_ioElements; }
    [[nodiscard]] QList<ElementDescriptor> gatesElements() const { return m_gatesElements; }
    [[nodiscard]] QList<ElementDescriptor> combinationalElements() const { return m_combinationalElements; }
    [[nodiscard]] QList<ElementDescriptor> memoryElements() const { return m_memoryElements; }
    [[nodiscard]] QList<ElementDescriptor> miscElements() const { return m_miscElements; }

    // --- Dynamic lists ---

    [[nodiscard]] QList<ElementDescriptor> icElements() const { return m_icElements; }
    [[nodiscard]] QList<ElementDescriptor> embeddedICElements() const { return m_embeddedICElements; }

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

    [[nodiscard]] QList<ElementDescriptor> searchResults() const { return m_searchResults; }

    /// Returns the first search result's descriptor, or an empty map if searchResults is
    /// empty. Mirrors ElementPalette::onSearchReturnPressed()'s "add the first visible result".
    /// Stays QVariantMap (not ElementDescriptor): it's a Q_INVOKABLE method, so it can never
    /// be shadow-check-resolvable regardless of its return type (see FINAL comment above),
    /// and ElementPalette.qml's call site relies on `result.type !== undefined` as a
    /// "no match" sentinel, which an always-populated gadget would silently break.
    Q_INVOKABLE QVariantMap firstSearchResult() const;

signals:
    void icElementsChanged();
    void embeddedICElementsChanged();
    void searchTextChanged();
    void searchResultsChanged();

private:
    static ElementDescriptor describeBuiltin(ElementType type);
    static ElementDescriptor describeIC(const QString &filePath);
    static ElementDescriptor describeEmbeddedIC(const QString &blobName);

    /// Concatenation of every fixed/IC/embedded-IC entry -- the pool onSearchTextChanged()
    /// filters. Rebuilt on demand (icElements()/embeddedICElements() change rarely, and this
    /// class has no per-entry stable identity to invalidate a cache against).
    [[nodiscard]] QList<ElementDescriptor> allEntries() const;
    void recomputeSearchResults();

    QList<ElementDescriptor> m_ioElements;
    QList<ElementDescriptor> m_gatesElements;
    QList<ElementDescriptor> m_combinationalElements;
    QList<ElementDescriptor> m_memoryElements;
    QList<ElementDescriptor> m_miscElements;
    QList<ElementDescriptor> m_icElements;
    QList<ElementDescriptor> m_embeddedICElements;
    QString m_searchText;
    QList<ElementDescriptor> m_searchResults;
};
