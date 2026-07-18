// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickElementPalette.h"

#include <QDir>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementMetadata.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
#include "App/UI/ElementPalette.h"

namespace {

// Qt resource paths (":/Components/...") need the "qrc" scheme prefix to be usable as a QML
// Image.source URL.
QString toQmlUrl(const QString &resourcePath)
{
    return resourcePath.startsWith(QLatin1Char(':')) ? QStringLiteral("qrc") + resourcePath : resourcePath;
}

} // namespace

QuickElementPalette::QuickElementPalette(QObject *parent)
    : QObject(parent)
{
    const auto build = [](const QStringList &names) {
        QList<ElementDescriptor> list;
        for (const auto &name : names) {
            list.append(describeBuiltin(ElementFactory::textToType(name)));
        }
        return list;
    };

    // Same five category lists and order as ElementPalette::populate().
    m_ioElements = build({"InputVcc", "InputGnd", "InputButton", "InputSwitch", "InputRotary", "Clock", "Led", "Display7", "Display14", "Display16", "Buzzer", "AudioBox"});
    m_gatesElements = build({"And", "Or", "Not", "Nand", "Nor", "Xor", "Xnor", "Node"});
    m_combinationalElements = build({"TruthTable", "Mux", "Demux"});
    m_memoryElements = build({"DLatch", "SRLatch", "DFlipFlop", "JKFlipFlop", "TFlipFlop"});
    m_miscElements = build({"Text", "Line"});
}

ElementDescriptor QuickElementPalette::describeBuiltin(ElementType type)
{
    return ElementDescriptor(
        static_cast<int>(type), ElementFactory::translatedName(type), QString(), false,
        toQmlUrl(ElementMetadataRegistry::metadata(type).pixmapPath()), ElementFactory::description(type));
}

ElementDescriptor QuickElementPalette::describeIC(const QString &filePath)
{
    return ElementDescriptor(
        static_cast<int>(ElementType::IC), QFileInfo(filePath).baseName().toUpper(), filePath, false,
        toQmlUrl(QStringLiteral(":/Components/Logic/ic-panda.svg")),
        tr("IC from file: %1").arg(QFileInfo(filePath).fileName()));
}

ElementDescriptor QuickElementPalette::describeEmbeddedIC(const QString &blobName)
{
    return ElementDescriptor(
        static_cast<int>(ElementType::IC), blobName.toUpper(), blobName, true,
        toQmlUrl(QStringLiteral(":/Components/Logic/ic-panda-embedded.svg")),
        tr("Embedded IC: %1").arg(blobName));
}

void QuickElementPalette::updateICList(const QFileInfo &currentFile)
{
    m_icElements.clear();

    if (currentFile.exists()) {
        const QDir directory(currentFile.absoluteDir());
        // Enumerate all .panda files in the same directory -- they are candidate ICs.
        QStringList files = directory.entryList({"*.panda", "*.PANDA"}, QDir::Files);
        // Exclude the project file itself and hidden/autosave files.
        files.removeAll(currentFile.fileName());
        for (qsizetype i = files.size() - 1; i >= 0; --i) {
            if (files.at(i).at(0) == '.') {
                files.removeAt(i);
            }
        }

        // Bare file names, not absolute paths: GraphicElement::loadFromDrop() resolves them
        // against a separate contextDir argument, mirroring SceneDropHandler's contract.
        for (const QString &file : std::as_const(files)) {
            m_icElements.append(describeIC(file));
        }
    }

    emit icElementsChanged();
    recomputeSearchResults();
}

void QuickElementPalette::updateEmbeddedICList(CanvasICRegistry *registry)
{
    m_embeddedICElements.clear();

    if (registry) {
        const auto blobNames = registry->blobMapRef().keys();
        for (const auto &name : blobNames) {
            m_embeddedICElements.append(describeEmbeddedIC(name));
        }
    }

    emit embeddedICElementsChanged();
    recomputeSearchResults();
}

void QuickElementPalette::setSearchText(const QString &text)
{
    if (m_searchText == text) {
        return;
    }
    m_searchText = text;
    emit searchTextChanged();
    recomputeSearchResults();
}

QVariantMap QuickElementPalette::firstSearchResult() const
{
    if (m_searchResults.isEmpty()) {
        return {};
    }
    const ElementDescriptor &entry = m_searchResults.first();
    QVariantMap map;
    map[QStringLiteral("type")] = entry.type();
    map[QStringLiteral("name")] = entry.name();
    map[QStringLiteral("icFileName")] = entry.icFileName();
    map[QStringLiteral("isEmbedded")] = entry.isEmbedded();
    map[QStringLiteral("iconSource")] = entry.iconSource();
    map[QStringLiteral("tooltip")] = entry.tooltip();
    return map;
}

QList<ElementDescriptor> QuickElementPalette::allEntries() const
{
    QList<ElementDescriptor> all;
    all += m_ioElements;
    all += m_gatesElements;
    all += m_combinationalElements;
    all += m_memoryElements;
    all += m_miscElements;
    all += m_icElements;
    all += m_embeddedICElements;
    return all;
}

void QuickElementPalette::recomputeSearchResults()
{
    QList<ElementDescriptor> results;

    if (!m_searchText.isEmpty()) {
        // Single effective pass (match against the entry's display name), not ElementPalette::
        // onSearchTextChanged()'s three passes -- see this class's doc comment for why passes
        // 1 and 3 are dead code in production and are deliberately not reproduced.
        const auto all = allEntries();
        for (const auto &entry : all) {
            if (ElementPalette::nameMatchesSearch(entry.name(), m_searchText)) {
                results.append(entry);
            }
        }
    }

    m_searchResults = results;
    emit searchResultsChanged();
}
