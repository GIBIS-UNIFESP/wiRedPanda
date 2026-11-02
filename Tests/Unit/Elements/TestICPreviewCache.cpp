// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestICPreviewCache.h"

#include <memory>
#include <vector>

#include <QDir>
#include <QFile>
#include <QScopeGuard>
#include <QTemporaryDir>

#include "App/Core/Common.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/IC.h"
#include "App/Element/ICLoader.h"
#include "App/Element/ICPreviewCache.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

namespace {

/// Saves a circuit of \a pairs input switches, each wired to a LED, as a .panda file at \a path.
void saveCircuit(const QString &path, const int pairs)
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    for (int i = 0; i < pairs; ++i) {
        auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
        auto *led = ElementFactory::buildElement(ElementType::Led);
        builder.add(sw, led);
        builder.connect(sw, 0, led, 0);
    }
    QCOMPARE(workspace.save(path), WorkSpace::SaveOutcome::Saved);
}

} // namespace

void TestICPreviewCache::init()
{
    ICPreviewCache::clear();
}

void TestICPreviewCache::testLoadingRendersNoPreview()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("loaded.panda");
    saveCircuit(path, 2);

    const int before = ICPreviewCache::renderCount();
    IC ic;
    ic.loadFile(path, dir.path());
    QCOMPARE(ICPreviewCache::renderCount(), before);
}

void TestICPreviewCache::testFirstAskRendersAndKeeps()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("asked.panda");
    saveCircuit(path, 2);

    IC ic;
    ic.loadFile(path, dir.path());
    const int before = ICPreviewCache::renderCount();
    const QPixmap first = ic.preview();
    QVERIFY2(!first.isNull(), "a circuit with elements renders a preview when first asked");
    QCOMPARE(ICPreviewCache::renderCount() - before, 1);

    const QPixmap second = ic.preview();
    QCOMPARE(ICPreviewCache::renderCount() - before, 1);
    QCOMPARE(second.cacheKey(), first.cacheKey());
}

void TestICPreviewCache::testFileInstancesShareOnePreview()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("shared.panda");
    saveCircuit(path, 2);

    IC first;
    IC second;
    first.loadFile(path, dir.path());
    second.loadFile(path, dir.path());

    const int before = ICPreviewCache::renderCount();
    const QPixmap firstPreview = first.preview();
    const QPixmap secondPreview = second.preview();
    QCOMPARE(ICPreviewCache::renderCount() - before, 1);
    QCOMPARE(secondPreview.cacheKey(), firstPreview.cacheKey());
}

void TestICPreviewCache::testBlobInstancesShareOnePreviewPerDirectory()
{
    // One blob in two directories: its references resolve to two circuits, so it has two previews
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkdir("other"));
    const QString path = dir.filePath("blob.panda");
    saveCircuit(path, 2);
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray blob = file.readAll();

    IC first;
    IC second;
    IC elsewhere;
    ICLoader::loadFromBlob(first, blob, dir.path());
    ICLoader::loadFromBlob(second, blob, dir.path());
    ICLoader::loadFromBlob(elsewhere, blob, dir.filePath("other"));

    const int before = ICPreviewCache::renderCount();
    const QPixmap firstPreview = first.preview();
    QCOMPARE(second.preview().cacheKey(), firstPreview.cacheKey());
    QCOMPARE(ICPreviewCache::renderCount() - before, 1);

    const QPixmap elsewherePreview = elsewhere.preview();
    QCOMPARE(ICPreviewCache::renderCount() - before, 2);
    QVERIFY(elsewherePreview.cacheKey() != firstPreview.cacheKey());
}

void TestICPreviewCache::testReloadAsksForItsOwn()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("reloaded.panda");
    saveCircuit(path, 2);

    IC ic;
    ic.loadFile(path, dir.path());
    QVERIFY(!ic.preview().isNull());

    // The file changes and is reloaded: the definition is new, so its preview is rendered again
    saveCircuit(path, 3);
    ic.loadFile(path, dir.path());
    const int before = ICPreviewCache::renderCount();
    QVERIFY(!ic.preview().isNull());
    QCOMPARE(ICPreviewCache::renderCount() - before, 1);
}

void TestICPreviewCache::testNullPreviewIsKept()
{
    // An empty circuit has no preview; the null is kept, so it is not built again
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("empty.panda");
    saveCircuit(path, 0);

    IC ic;
    ic.loadFile(path, dir.path());
    const int before = ICPreviewCache::renderCount();
    QVERIFY(ic.preview().isNull());
    QCOMPARE(ICPreviewCache::renderCount() - before, 1);

    QVERIFY(ic.preview().isNull());
    QCOMPARE(ICPreviewCache::renderCount() - before, 1);
}

void TestICPreviewCache::testCircuitThatNoLongerBuildsHasANullPreviewKept()
{
    // A circuit holding an IC from a file that is gone since the load no longer builds: its preview is null, kept
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString innerPath = dir.filePath("inner.panda");
    saveCircuit(innerPath, 1);

    WorkSpace outerWorkspace;
    CircuitBuilder outerBuilder(outerWorkspace.scene());
    auto *outerSw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *innerIc = new IC;
    innerIc->loadFile(innerPath, dir.path());
    auto *outerLed = ElementFactory::buildElement(ElementType::Led);
    outerBuilder.add(outerSw, innerIc, outerLed);
    outerBuilder.connect(outerSw, 0, innerIc, 0);
    outerBuilder.connect(innerIc, 0, outerLed, 0);
    const QString outerPath = dir.filePath("outer.panda");
    QCOMPARE(outerWorkspace.save(outerPath), WorkSpace::SaveOutcome::Saved);

    IC outer;
    outer.loadFile(outerPath, dir.path());
    QVERIFY(QFile::remove(innerPath));

    const int before = ICPreviewCache::renderCount();
    QVERIFY2(outer.preview().isNull(), "a circuit whose IC file is gone renders no preview");
    QCOMPARE(ICPreviewCache::renderCount() - before, 1);

    QVERIFY(outer.preview().isNull());
    QCOMPARE(ICPreviewCache::renderCount() - before, 1);
}

void TestICPreviewCache::testLeastRecentlyUsedPreviewGoes()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    constexpr int capacity = static_cast<int>(ICPreviewCache::kCapacity);

    // One more circuit than the cache holds
    std::vector<std::unique_ptr<IC>> ics;
    for (int i = 0; i <= capacity; ++i) {
        const QString path = dir.filePath(QString("circuit%1.panda").arg(i));
        saveCircuit(path, 1);
        auto ic = std::make_unique<IC>();
        ic->loadFile(path, dir.path());
        ics.push_back(std::move(ic));
    }

    const int before = ICPreviewCache::renderCount();
    for (int i = 0; i < capacity; ++i) {
        QVERIFY(!ics[static_cast<std::size_t>(i)]->preview().isNull());
    }
    QCOMPARE(ICPreviewCache::renderCount() - before, capacity);

    // Asking for the first makes it the most recent; the circuit after the last evicts the least recent, the second
    QVERIFY(!ics[0]->preview().isNull());
    QCOMPARE(ICPreviewCache::renderCount() - before, capacity);
    QVERIFY(!ics[static_cast<std::size_t>(capacity)]->preview().isNull());
    QCOMPARE(ICPreviewCache::renderCount() - before, capacity + 1);
    QCOMPARE(ICPreviewCache::entryCount(), capacity);

    QVERIFY(!ics[0]->preview().isNull());
    QCOMPARE(ICPreviewCache::renderCount() - before, capacity + 1);
    QVERIFY(!ics[1]->preview().isNull());
    QCOMPARE(ICPreviewCache::renderCount() - before, capacity + 2);
}

void TestICPreviewCache::testThemeChangeDropsEveryPreview()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("themed.panda");
    saveCircuit(path, 2);

    IC ic;
    ic.loadFile(path, dir.path());
    QVERIFY(!ic.preview().isNull());
    QCOMPARE(ICPreviewCache::entryCount(), 1);

    const Theme original = ThemeManager::theme();
    ThemeManager::setTheme(original == Theme::Dark ? Theme::Light : Theme::Dark);
    const auto restore = qScopeGuard([original] { ThemeManager::setTheme(original); });
    QCOMPARE(ICPreviewCache::entryCount(), 0);

    const int before = ICPreviewCache::renderCount();
    QVERIFY(!ic.preview().isNull());
    QCOMPARE(ICPreviewCache::renderCount() - before, 1);
}
