// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickElementPalette.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/QuickShell/Chrome/QuickElementPalette.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"

namespace {

/// Finds an entry in \a list (a QVariantList of QVariantMap descriptors) whose "name" field
/// case-insensitively contains \a needle, or an empty map if none matches.
QVariantMap findByName(const QVariantList &list, const QString &needle)
{
    for (const auto &entryVariant : list) {
        const auto entry = entryVariant.toMap();
        if (entry.value(QStringLiteral("name")).toString().contains(needle, Qt::CaseInsensitive)) {
            return entry;
        }
    }
    return {};
}

void writeFile(const QString &path, const QByteArray &content = "placeholder")
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(content);
}

/// Places a real embedded IC on \a canvas, backed by \a fixtureDir's "test_circuit.panda" bytes
/// stored under \a blobName. Mirrors TestCanvasEmbeddedIC.cpp's identical placeEmbeddedIC() --
/// named distinctly (not placeEmbeddedTestIC) to avoid a unity-build symbol collision with
/// TestQuickElementEditor.cpp's own identically-shaped helper.
IC *placeEmbeddedPaletteTestIC(CanvasItem &canvas, const QString &fixtureDir, const QString &blobName,
                                const QPointF &pos = {100, 100})
{
    QFile file(fixtureDir + "/test_circuit.panda");
    if (!file.open(QIODevice::ReadOnly)) {
        return nullptr;
    }
    const QByteArray blob = file.readAll();

    auto *reg = canvas.icRegistry();
    auto *ic = new IC();
    reg->setBlob(blobName, blob);
    ic->setBlobName(blobName);
    ic->loadFromBlob(blob, fixtureDir);
    ic->setPos(pos);
    canvas.receiveCommand(new CanvasAddItemsCommand({ic}, &canvas));
    return ic;
}

} // namespace

void TestQuickElementPalette::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
    m_fixtureDir = m_tempDir.path();

    QuickWorkSpace ws;
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::Led));
    QVERIFY2(ws.save(m_fixtureDir + "/test_circuit.panda") == QuickWorkSpace::SaveOutcome::Saved, "fixture write");
}

void TestQuickElementPalette::testCategoryListsPopulatedAtConstruction()
{
    QuickElementPalette palette;

    QVERIFY(!palette.ioElements().isEmpty());
    QVERIFY(!palette.gatesElements().isEmpty());
    QVERIFY(!palette.combinationalElements().isEmpty());
    QVERIFY(!palette.memoryElements().isEmpty());
    QVERIFY(!palette.miscElements().isEmpty());

    QVERIFY(!findByName(palette.gatesElements(), "AND").isEmpty());
    QVERIFY(!findByName(palette.ioElements(), "SWITCH").isEmpty());

    // The dynamic lists start empty until updateICList()/updateEmbeddedICList() populate them.
    QVERIFY(palette.icElements().isEmpty());
    QVERIFY(palette.embeddedICElements().isEmpty());
}

void TestQuickElementPalette::testCategoryEntriesHaveDescriptiveTooltips()
{
    // Mirrors TestElementPalette::testElementLabelHasDescriptiveTooltip(): each descriptor's
    // "tooltip" field is built from the same ElementFactory::description() call ElementLabel's
    // own Widgets tooltip uses, so it must be non-empty and genuinely element-specific, not a
    // shared placeholder.
    QuickElementPalette palette;

    const QVariantMap andEntry = findByName(palette.gatesElements(), "AND");
    const QVariantMap switchEntry = findByName(palette.ioElements(), "SWITCH");
    QVERIFY(!andEntry.isEmpty());
    QVERIFY(!switchEntry.isEmpty());

    QVERIFY(!andEntry.value("tooltip").toString().isEmpty());
    QVERIFY(!switchEntry.value("tooltip").toString().isEmpty());
    QVERIFY(andEntry.value("tooltip").toString() != switchEntry.value("tooltip").toString());
}

void TestQuickElementPalette::testSearchEmptyTextYieldsNoResults()
{
    QuickElementPalette palette;
    palette.setSearchText("AND");
    QVERIFY(!palette.searchResults().isEmpty());

    palette.setSearchText("");
    QVERIFY(palette.searchResults().isEmpty());
}

void TestQuickElementPalette::testSearchMatchesByNameCaseInsensitive()
{
    QuickElementPalette palette;

    palette.setSearchText("and");
    QVERIFY(!findByName(palette.searchResults(), "AND").isEmpty());

    palette.setSearchText("AND");
    QVERIFY(!findByName(palette.searchResults(), "AND").isEmpty());
}

void TestQuickElementPalette::testSearchNoMatchYieldsEmptyResults()
{
    QuickElementPalette palette;
    palette.setSearchText("ThisElementDoesNotExistAnywhere");
    QVERIFY(palette.searchResults().isEmpty());
}

void TestQuickElementPalette::testSearchMatchesInternalTypeKeywordEvenWhenDisplayNameDiffers()
{
    // SRLatch's translated display name is "SR-Latch" (a hyphen) but its internal, untranslated
    // type keyword is "SRLatch" (no hyphen) -- a real, demonstrable case where the two diverge,
    // proving pass 1 (internalName) actually contributes results pass 2 (name) alone can't find.
    // Mirrors ElementPalette.cpp's populateMenu() "label_" + name.toLower() objectName match.
    QuickElementPalette palette;

    QVERIFY(findByName(palette.memoryElements(), "SRLatch").isEmpty());

    palette.setSearchText("SRLatch");
    QVERIFY(!palette.searchResults().isEmpty());

    bool foundSRLatch = false;
    for (const auto &entryVariant : palette.searchResults()) {
        if (entryVariant.toMap().value("internalName").toString() == QStringLiteral("SRLatch")) {
            foundSRLatch = true;
        }
    }
    QVERIFY2(foundSRLatch, "search by the internal type keyword must find SR-Latch");
}

void TestQuickElementPalette::testSearchMatchesFileBasedICFullFileNameIncludingExtension()
{
    // Mirrors ElementPalette.cpp's third search pass: a query including the ".panda" extension
    // matches a file-based IC by its full file name, which the basename-only "name" field
    // (uppercased, no extension) can't match on its own.
    QVERIFY(m_tempDir.isValid());
    const QString dir = m_tempDir.path() + "/ic_filename_search";
    QVERIFY(QDir().mkpath(dir));
    writeFile(dir + "/current.panda");
    writeFile(dir + "/adder.panda");

    QuickElementPalette palette;
    palette.updateICList(QFileInfo(dir + "/current.panda"));

    palette.setSearchText("adder.panda");
    bool foundByFileName = false;
    for (const auto &entryVariant : palette.searchResults()) {
        if (entryVariant.toMap().value("icFileName").toString() == QStringLiteral("adder.panda")) {
            foundByFileName = true;
        }
    }
    QVERIFY2(foundByFileName, "search including the .panda extension must match the IC's full file name");
}

void TestQuickElementPalette::testSearchMatchesGenericIcKeywordRegardlessOfFileName()
{
    // ElementPalette.cpp gives every file-based/embedded IC label the same constant objectName
    // ("label_ic"/"label_embedded_ic"), not one derived from the real file/blob name -- so
    // searching the bare keyword "ic" matches every IC entry via pass 1, independent of its
    // actual name. Neither fixture file name below contains "ic" anywhere, isolating this from
    // the unrelated name/file-name passes.
    QVERIFY(m_tempDir.isValid());
    const QString dir = m_tempDir.path() + "/generic_ic_keyword_search";
    QVERIFY(QDir().mkpath(dir));
    writeFile(dir + "/current.panda");
    writeFile(dir + "/adder.panda");

    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.icRegistry()->setBlob("gizmo", QByteArray("blob"));

    QuickElementPalette palette;
    palette.updateICList(QFileInfo(dir + "/current.panda"));
    palette.updateEmbeddedICList(canvas.icRegistry());

    palette.setSearchText("ic");

    bool foundFileBased = false;
    bool foundEmbedded = false;
    for (const auto &entryVariant : palette.searchResults()) {
        const auto entry = entryVariant.toMap();
        if (entry.value("icFileName").toString() == QStringLiteral("adder.panda")) {
            foundFileBased = true;
        }
        if (entry.value("icFileName").toString() == QStringLiteral("gizmo")) {
            foundEmbedded = true;
        }
    }
    QVERIFY2(foundFileBased, "the generic \"ic\" keyword must match a file-based IC entry regardless of its file name");
    QVERIFY2(foundEmbedded, "the generic \"ic\" keyword must match an embedded IC entry regardless of its blob name");
}

void TestQuickElementPalette::testSearchByExtensionDoesNotMatchEmbeddedICEntries()
{
    // The third pass (full file name match) applies only to file-based ICs, mirroring
    // ElementPalette.cpp's own `item->objectName() == "label_ic"` guard. An embedded IC's
    // "name"/"icFileName" are both just its blob name (never ".panda"-suffixed), so if it
    // doesn't contain the query anywhere, an embedded entry must not appear in the results --
    // proving the file-based-only pass didn't leak into matching it some other way.
    QVERIFY(m_tempDir.isValid());
    const QString dir = m_tempDir.path() + "/ic_filename_search_embedded";
    QVERIFY(QDir().mkpath(dir));
    writeFile(dir + "/current.panda");
    writeFile(dir + "/adder.panda");

    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.icRegistry()->setBlob("specialblob", QByteArray("blob"));

    QuickElementPalette palette;
    palette.updateICList(QFileInfo(dir + "/current.panda"));
    palette.updateEmbeddedICList(canvas.icRegistry());

    palette.setSearchText(".panda");

    bool foundFileBased = false;
    for (const auto &entryVariant : palette.searchResults()) {
        const auto entry = entryVariant.toMap();
        QVERIFY2(!entry.value("isEmbedded").toBool(), "an embedded IC entry must never match via the file-based-only filename pass");
        if (entry.value("icFileName").toString() == QStringLiteral("adder.panda")) {
            foundFileBased = true;
        }
    }
    QVERIFY2(foundFileBased, "the file-based IC must still match by its full file name");
}

void TestQuickElementPalette::testFirstSearchResultReturnsEmptyMapWhenNoMatch()
{
    QuickElementPalette palette;
    palette.setSearchText("ThisElementDoesNotExistAnywhere");

    const QVariantMap result = palette.firstSearchResult();
    QVERIFY(result.isEmpty());
}

void TestQuickElementPalette::testFirstSearchResultReturnsMatchingEntryFields()
{
    QuickElementPalette palette;
    palette.setSearchText("AND");

    const QVariantMap result = palette.firstSearchResult();
    QVERIFY(!result.isEmpty());
    QCOMPARE(result["type"].toInt(), static_cast<int>(ElementType::And));
    QVERIFY(!result["isEmbedded"].toBool());
    QVERIFY(result["icFileName"].toString().isEmpty());
    QVERIFY(!result["iconSource"].toString().isEmpty());
}

void TestQuickElementPalette::testUpdateICListScansPandaFilesExcludingCurrentAndHidden()
{
    QVERIFY(m_tempDir.isValid());
    const QString dir = m_tempDir.path() + "/ic_scan";
    QVERIFY(QDir().mkpath(dir));

    writeFile(dir + "/current.panda");
    writeFile(dir + "/sibling1.panda");
    writeFile(dir + "/sibling2.panda");
    writeFile(dir + "/.autosave.panda");
    writeFile(dir + "/not_an_ic.txt");

    QuickElementPalette palette;
    palette.updateICList(QFileInfo(dir + "/current.panda"));

    const auto list = palette.icElements();
    QCOMPARE(list.size(), 2);
    QVERIFY(!findByName(list, "SIBLING1").isEmpty());
    QVERIFY(!findByName(list, "SIBLING2").isEmpty());
    QVERIFY(findByName(list, "CURRENT").isEmpty());
    QVERIFY(findByName(list, "AUTOSAVE").isEmpty());
}

void TestQuickElementPalette::testUpdateICListClearsWhenCurrentFileDoesNotExist()
{
    QVERIFY(m_tempDir.isValid());
    const QString dir = m_tempDir.path() + "/ic_clear";
    QVERIFY(QDir().mkpath(dir));
    writeFile(dir + "/current.panda");
    writeFile(dir + "/sibling.panda");

    QuickElementPalette palette;
    palette.updateICList(QFileInfo(dir + "/current.panda"));
    QVERIFY(!palette.icElements().isEmpty());

    palette.updateICList(QFileInfo(dir + "/does_not_exist.panda"));
    QVERIFY(palette.icElements().isEmpty());
}

void TestQuickElementPalette::testUpdateEmbeddedICListReflectsRegistryBlobs()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.icRegistry()->setBlob("embedded_a", QByteArray("blob-a"));
    canvas.icRegistry()->setBlob("embedded_b", QByteArray("blob-b"));

    QuickElementPalette palette;
    palette.updateEmbeddedICList(canvas.icRegistry());

    const auto list = palette.embeddedICElements();
    QCOMPARE(list.size(), 2);
    QVERIFY(!findByName(list, "EMBEDDED_A").isEmpty());
    QVERIFY(!findByName(list, "EMBEDDED_B").isEmpty());
    for (const auto &entryVariant : list) {
        QVERIFY(entryVariant.toMap().value("isEmbedded").toBool());
    }
}

void TestQuickElementPalette::testUpdateEmbeddedICListClearsWhenRegistryNull()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.icRegistry()->setBlob("embedded_a", QByteArray("blob-a"));

    QuickElementPalette palette;
    palette.updateEmbeddedICList(canvas.icRegistry());
    QVERIFY(!palette.embeddedICElements().isEmpty());

    palette.updateEmbeddedICList(nullptr);
    QVERIFY(palette.embeddedICElements().isEmpty());
}

void TestQuickElementPalette::testSearchIncludesDynamicallyLoadedICEntries()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.icRegistry()->setBlob("my_special_adder", QByteArray("blob"));

    QuickElementPalette palette;
    palette.updateEmbeddedICList(canvas.icRegistry());

    palette.setSearchText("SPECIAL_ADDER");
    QVERIFY(!findByName(palette.searchResults(), "SPECIAL_ADDER").isEmpty());
}

void TestQuickElementPalette::testAddElementToCurrentTabAddsBuiltinElement()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    const int before = static_cast<int>(canvas->elements().size());

    // Mirrors ElementPalette.qml's Keys.onReturnPressed handler exactly: search, take the
    // first result, hand its fields straight to addElementToCurrentTab().
    controller.elementPalette()->setSearchText("AND");
    const QVariantMap result = controller.elementPalette()->firstSearchResult();
    QVERIFY(!result.isEmpty());

    controller.addElementToCurrentTab(result["type"].toInt(), result["icFileName"].toString(),
                                       result["isEmbedded"].toBool(), 100, 150);

    QCOMPARE(canvas->elements().size(), before + 1);
    auto *added = canvas->elements().last();
    QCOMPARE(added->elementType(), ElementType::And);
    QCOMPARE(added->pos(), QPointF(100, 150));
}

void TestQuickElementPalette::testAddElementToCurrentTabAddsEmbeddedIC()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();

    // initEmbeddedIC() (addElementFromPalette()'s embedded-IC branch) loads the blob through
    // ICLoader, so it needs real, loadable .panda bytes.
    QFile fixture(m_fixtureDir + "/test_circuit.panda");
    QVERIFY(fixture.open(QIODevice::ReadOnly));
    canvas->icRegistry()->setBlob("palette_embedded_test", fixture.readAll());

    const int before = static_cast<int>(canvas->elements().size());
    controller.addElementToCurrentTab(static_cast<int>(ElementType::IC), "palette_embedded_test",
                                       /*isEmbedded=*/true, 50, 60);

    QCOMPARE(canvas->elements().size(), before + 1);
    auto *added = canvas->elements().last();
    QVERIFY(added->isEmbedded());
    QCOMPARE(added->blobName(), QString("palette_embedded_test"));
    QCOMPARE(added->pos(), QPointF(50, 60));
}

void TestQuickElementPalette::testAddElementToCurrentTabWithMissingEmbeddedBlobIsNoOp()
{
    // addElementFromPalette()'s embedded-IC branch calls CanvasICRegistry::initEmbeddedIC(),
    // which returns false (and adds nothing) when the blob name isn't registered.
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    const int before = static_cast<int>(canvas->elements().size());

    controller.addElementToCurrentTab(static_cast<int>(ElementType::IC), "no_such_blob",
                                       /*isEmbedded=*/true, 50, 60);

    QCOMPARE(canvas->elements().size(), before);
}

void TestQuickElementPalette::testAddElementToCurrentTabIgnoresEmbeddedFlagForNonICTypes()
{
    // A non-IC type is added normally regardless of the isEmbedded flag's value --
    // addElementFromPalette() only ever consults isEmbedded when type == ElementType::IC.
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    const int before = static_cast<int>(canvas->elements().size());

    controller.addElementToCurrentTab(static_cast<int>(ElementType::And), "bogus",
                                       /*isEmbedded=*/true, 50, 60);

    QCOMPARE(canvas->elements().size(), before + 1);
    QCOMPARE(canvas->elements().last()->elementType(), ElementType::And);
}

void TestQuickElementPalette::testUpdateEmbeddedICListDedupesByBlobName()
{
    // Multiple IC instances sharing one blob name must still surface as exactly one palette
    // entry. QuickElementPalette::updateEmbeddedICList() iterates the registry's own blob map
    // keys (a name can only ever be registered once -- QMap/QHash key uniqueness) rather than
    // scanning scene elements and de-duping labels the way the Widgets original does -- a
    // different implementation strategy that guarantees the same outcome. Three real IC elements
    // referencing the same already-registered blob confirm this holds in practice, not just by
    // map-key construction.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *ic1 = placeEmbeddedPaletteTestIC(canvas, m_fixtureDir, "same_name", {0, 100});
    placeEmbeddedPaletteTestIC(canvas, m_fixtureDir, "same_name", {100, 100});
    placeEmbeddedPaletteTestIC(canvas, m_fixtureDir, "same_name", {200, 100});
    QVERIFY(ic1);

    QuickElementPalette palette;
    palette.updateEmbeddedICList(canvas.icRegistry());

    QCOMPARE(palette.embeddedICElements().size(), 1);
    QCOMPARE(palette.embeddedICElements().first().toMap().value("icFileName").toString(), QString("same_name"));
}

void TestQuickElementPalette::testUpdateEmbeddedICListReflectsRemovalAfterRemoveEmbeddedIC()
{
    // A full add -> refresh -> remove -> refresh round trip through
    // QuickWorkSpace::removeEmbeddedIC() (the production entry point for "remove this embedded
    // IC everywhere").
    QuickWorkSpace ws;
    placeEmbeddedPaletteTestIC(*ws.canvas(), m_fixtureDir, "removable");

    QuickElementPalette palette;
    palette.updateEmbeddedICList(ws.canvas()->icRegistry());
    QCOMPARE(palette.embeddedICElements().size(), 1);

    ws.removeEmbeddedIC("removable");

    palette.updateEmbeddedICList(ws.canvas()->icRegistry());
    QCOMPARE(palette.embeddedICElements().size(), 0);
}
