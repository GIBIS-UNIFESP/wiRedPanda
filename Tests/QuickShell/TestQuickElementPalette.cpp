// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickElementPalette.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/QuickShell/Chrome/QuickElementPalette.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"

namespace {

/// Finds an entry in \a list whose name() case-insensitively contains \a needle, or nullptr.
const ElementDescriptor *findByName(const QList<ElementDescriptor> &list, const QString &needle)
{
    for (const auto &entry : list) {
        if (entry.name().contains(needle, Qt::CaseInsensitive)) {
            return &entry;
        }
    }
    return nullptr;
}

void writeFile(const QString &path, const QByteArray &content = "placeholder")
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(content);
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

    QVERIFY(findByName(palette.gatesElements(), "AND"));
    QVERIFY(findByName(palette.ioElements(), "SWITCH"));

    // The dynamic lists start empty until updateICList()/updateEmbeddedICList() populate them.
    QVERIFY(palette.icElements().isEmpty());
    QVERIFY(palette.embeddedICElements().isEmpty());
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
    QVERIFY(findByName(palette.searchResults(), "AND"));

    palette.setSearchText("AND");
    QVERIFY(findByName(palette.searchResults(), "AND"));
}

void TestQuickElementPalette::testSearchNoMatchYieldsEmptyResults()
{
    QuickElementPalette palette;
    palette.setSearchText("ThisElementDoesNotExistAnywhere");
    QVERIFY(palette.searchResults().isEmpty());
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
    QVERIFY(findByName(list, "SIBLING1"));
    QVERIFY(findByName(list, "SIBLING2"));
    QVERIFY(!findByName(list, "CURRENT"));
    QVERIFY(!findByName(list, "AUTOSAVE"));
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
    QVERIFY(findByName(list, "EMBEDDED_A"));
    QVERIFY(findByName(list, "EMBEDDED_B"));
    for (const auto &entry : list) {
        QVERIFY(entry.isEmbedded());
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
    QVERIFY(findByName(palette.searchResults(), "SPECIAL_ADDER"));
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
    // ICLoader, so it needs real, loadable .panda bytes -- the same initTestCase() fixture
    // every other Quick embedded-IC test file reuses (TestCanvasEmbeddedIC.cpp's
    // placeEmbeddedIC(), TestCanvasInlineIC.cpp's placeInlineTestIC()).
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
