// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickElementPalette.h"

#include <QDir>
#include <QRegularExpression>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementMetadata.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"

namespace {

// Qt resource paths (":/Components/...") need the "qrc" scheme prefix to be usable as a QML
// Image.source URL.
QString toQmlUrl(const QString &resourcePath)
{
    return resourcePath.startsWith(QLatin1Char(':')) ? QStringLiteral("qrc") + resourcePath : resourcePath;
}

// Mirrors ElementLabel.cpp's constructor exactly: every ElementLabel's tooltip is its
// description (if any) plus this trailing usage hint on its own line, unconditionally.
QString appendAddHint(const QString &description)
{
    const QString addHint = QuickElementPalette::tr("Drag or double-click to add.");
    return description.isEmpty() ? addHint : description + QLatin1Char('\n') + addHint;
}

// Duplicates App/UI/ElementPalette.h's static nameMatchesSearch() rather than calling it
// directly: that class's .cpp is WIDGETS_ONLY (pulls in QLayout/QScrollArea/MainWindowUI.h),
// while wiredpanda must stay Widgets-free. The function itself is a self-contained,
// four-line regex match with no Widgets dependency -- escapes the raw query so regex
// metacharacters ('(', '[', '\\', '+', ...) are matched literally, since an unescaped query
// can form an invalid pattern whose match() never succeeds, silently returning zero results.
bool nameMatchesSearch(const QString &name, const QString &query)
{
    const QRegularExpression regex(".*" + QRegularExpression::escape(query) + ".*",
                                   QRegularExpression::CaseInsensitiveOption);
    return regex.match(name).hasMatch();
}

} // namespace

QuickElementPalette::QuickElementPalette(QObject *parent)
    : QObject(parent)
{
    const auto build = [](const QStringList &names) {
        QVariantList list;
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

QVariantMap QuickElementPalette::describeBuiltin(ElementType type)
{
    QVariantMap entry;
    entry[QStringLiteral("type")] = static_cast<int>(type);
    entry[QStringLiteral("name")] = ElementFactory::translatedName(type);
    // The untranslated, locale-independent type keyword (e.g. "and") -- ElementPalette.cpp's
    // populateMenu() sets this as the ElementLabel's objectName ("label_" + name.toLower()),
    // which onSearchTextChanged()'s first pass matches against. Kept here under its own key
    // (not reusing "name", which is the translated display name) so recomputeSearchResults()
    // can reproduce that same fallback: search still finds "and" by typing its English/internal
    // name even in a non-English locale where the translated name no longer contains it.
    entry[QStringLiteral("internalName")] = ElementFactory::typeToText(type);
    entry[QStringLiteral("icFileName")] = QString();
    entry[QStringLiteral("isEmbedded")] = false;
    entry[QStringLiteral("iconSource")] = toQmlUrl(ElementMetadataRegistry::metadata(type).pixmapPath());
    entry[QStringLiteral("tooltip")] = appendAddHint(ElementFactory::description(type));
    return entry;
}

QVariantMap QuickElementPalette::describeIC(const QString &filePath)
{
    QVariantMap entry;
    entry[QStringLiteral("type")] = static_cast<int>(ElementType::IC);
    entry[QStringLiteral("name")] = QFileInfo(filePath).baseName().toUpper();
    // ElementPalette.cpp gives every file-based IC label the same constant objectName
    // ("label_ic"), not one derived from the file name -- so pass 1 of onSearchTextChanged()
    // matches ANY file-based IC entry against a query like "ic", independent of its real file
    // name. Mirrored here the same way built-ins mirror their "label_" suffix: as internalName.
    entry[QStringLiteral("internalName")] = QStringLiteral("ic");
    entry[QStringLiteral("icFileName")] = filePath;
    entry[QStringLiteral("isEmbedded")] = false;
    entry[QStringLiteral("iconSource")] = toQmlUrl(QStringLiteral(":/Components/Logic/ic-panda.svg"));
    entry[QStringLiteral("tooltip")] = appendAddHint(tr("IC from file: %1").arg(QFileInfo(filePath).fileName()));
    return entry;
}

QVariantMap QuickElementPalette::describeEmbeddedIC(const QString &blobName)
{
    QVariantMap entry;
    entry[QStringLiteral("type")] = static_cast<int>(ElementType::IC);
    entry[QStringLiteral("name")] = blobName.toUpper();
    // Mirrors ElementPalette.cpp's constant "label_embedded_ic" objectName the same way
    // describeIC() mirrors "label_ic" above.
    entry[QStringLiteral("internalName")] = QStringLiteral("embedded_ic");
    entry[QStringLiteral("icFileName")] = blobName;
    entry[QStringLiteral("isEmbedded")] = true;
    entry[QStringLiteral("iconSource")] = toQmlUrl(QStringLiteral(":/Components/Logic/ic-panda-embedded.svg"));
    entry[QStringLiteral("tooltip")] = appendAddHint(tr("Embedded IC: %1").arg(blobName));
    return entry;
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
    return m_searchResults.isEmpty() ? QVariantMap() : m_searchResults.first().toMap();
}

QVariantList QuickElementPalette::allEntries() const
{
    QVariantList all;
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
    QVariantList results;

    if (!m_searchText.isEmpty()) {
        // Mirrors ElementPalette::onSearchTextChanged()'s three passes: (1) the untranslated
        // internal type keyword ("label_and" objectName -- letting a query find "And" by typing
        // its English name even under a non-English translation), (2) the translated display
        // name, and (3) a file-based IC's full file name including its .panda extension
        // ("label_ic" objectName), which the basename-only display name in (2) can't match on
        // its own.
        const auto all = allEntries();
        for (const auto &entryVariant : all) {
            const auto entry = entryVariant.toMap();
            const bool matchesInternalName = nameMatchesSearch(entry.value(QStringLiteral("internalName")).toString(), m_searchText);
            const bool matchesDisplayName = nameMatchesSearch(entry.value(QStringLiteral("name")).toString(), m_searchText);
            const bool isFileBasedIC = entry.value(QStringLiteral("type")).toInt() == static_cast<int>(ElementType::IC)
                && !entry.value(QStringLiteral("isEmbedded")).toBool();
            const bool matchesICFileName = isFileBasedIC && nameMatchesSearch(entry.value(QStringLiteral("icFileName")).toString(), m_searchText);
            if (matchesInternalName || matchesDisplayName || matchesICFileName) {
                results.append(entry);
            }
        }
    }

    m_searchResults = results;
    emit searchResultsChanged();
}
