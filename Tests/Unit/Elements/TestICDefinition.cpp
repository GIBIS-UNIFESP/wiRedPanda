// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestICDefinition.h"

#include <memory>

#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QRectF>
#include <QTemporaryDir>

#include "App/Core/Common.h"
#include "App/Element/IC.h"
#include "App/Element/ICDefinition.h"
#include "App/Element/ICFileCache.h"
#include "App/IO/Serialization.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"

namespace {

// A current-format .panda with no elements: the header, then a metadata map whose tag tells one file's bytes from
// another's, and whose padding sets its size.
// With the embedded IC blobs \a blobs, as a file that nests other files holds them.
QByteArray pandaBytes(const QString &tag, const int padding = 0, const QMap<QString, QByteArray> &blobs = {})
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    QMap<QString, QVariant> metadata;
    metadata["dolphinFileName"] = tag;
    metadata["sceneRect"] = QRectF();
    metadata["padding"] = QString(padding, QChar('x'));
    Serialization::serializeBlobRegistry(blobs, metadata);

    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);
    payloadStream << metadata;
    Serialization::writePayload(stream, payload);
    return bytes;
}

// What a definition is charged: its records and blobs, and the cache key's copy of the path and the bytes
qsizetype chargeOf(const QString &path, const QByteArray &bytes, const ICDefinition &definition)
{
    qsizetype blobBytes = 0;
    for (const QByteArray &blob : definition.blobRegistry()) {
        blobBytes += blob.size();
    }
    return definition.records().size() + blobBytes + path.toUtf8().size() + 1 + bytes.size();
}

void writeFile(const QString &path, const QByteArray &bytes)
{
    QFile out(path);
    QVERIFY(out.open(QIODevice::WriteOnly));
    QCOMPARE(out.write(bytes), bytes.size());
}

/// Hands out fixed bytes for any path, as a scene's registry does for the files it holds.
class FakeFileCache : public ICFileCache
{
public:
    explicit FakeFileCache(QByteArray bytes)
        : m_bytes(std::move(bytes))
    {
    }

    const QByteArray &cachedFileBytes(const QString &) override
    {
        return m_bytes;
    }

private:
    QByteArray m_bytes;
};

} // namespace

void TestICDefinition::init()
{
    m_budget = ICDefinition::budget();
    ICDefinition::dropReleased();
}

void TestICDefinition::cleanup()
{
    ICDefinition::setBudget(m_budget);
    ICDefinition::dropReleased();
}

void TestICDefinition::testOneFileAndItsBytesShareOneDefinition()
{
    const QByteArray bytes = pandaBytes("shared");
    const int parses = ICDefinition::parseCount();
    const auto first = ICDefinition::acquire("/circuits/shared.panda", bytes);
    const auto second = ICDefinition::acquire("/circuits/shared.panda", bytes);
    QCOMPARE(second.get(), first.get());
    QCOMPARE(ICDefinition::parseCount() - parses, 1);

    // A blob is keyed by its bytes alone, so it shares the same way
    const QByteArray blob = pandaBytes("blob");
    const int blobParses = ICDefinition::parseCount();
    const auto blobFirst = ICDefinition::acquire(QString(), blob);
    const auto blobSecond = ICDefinition::acquire(QString(), blob);
    QCOMPARE(blobSecond.get(), blobFirst.get());
    QCOMPARE(ICDefinition::parseCount() - blobParses, 1);
}

void TestICDefinition::testSameBytesAtAnotherPathParseAgain()
{
    const QByteArray bytes = pandaBytes("twin");
    const int parses = ICDefinition::parseCount();
    const auto first = ICDefinition::acquire("/circuits/first.panda", bytes);
    const auto second = ICDefinition::acquire("/circuits/second.panda", bytes);
    QVERIFY(first.get() != second.get());
    QCOMPARE(ICDefinition::parseCount() - parses, 2);
}

void TestICDefinition::testOtherBytesAtOnePathParseAgain()
{
    // A file rewritten at the same path gets its own definition, whatever its size or modification time
    const int parses = ICDefinition::parseCount();
    const auto before = ICDefinition::acquire("/circuits/same.panda", pandaBytes("before"));
    const auto after = ICDefinition::acquire("/circuits/same.panda", pandaBytes("after"));
    QVERIFY(before.get() != after.get());
    QCOMPARE(ICDefinition::parseCount() - parses, 2);
}

void TestICDefinition::testDefinitionHoldsTheBytesItParsed()
{
    const QByteArray bytes = pandaBytes("held");
    const auto definition = ICDefinition::acquire("/circuits/held.panda", bytes);
    QCOMPARE(definition->metadata().value("dolphinFileName").toString(), QString("held"));
    QCOMPARE(definition->charge(), chargeOf("/circuits/held.panda", bytes, *definition));
}

void TestICDefinition::testChargeCountsTheBlobsItHolds()
{
    // The blob is held once, in the registry; the cache key holds the file, blob included. Both are charged.
    const QByteArray blob(4000, 'b');
    const QByteArray bytes = pandaBytes("nests", 0, {{"inner.panda", blob}});
    const auto definition = ICDefinition::acquire("/circuits/nests.panda", bytes);
    QCOMPARE(definition->blobRegistry().value("inner.panda"), blob);
    QVERIFY(!definition->metadata().contains("embeddedICs"));
    QCOMPARE(definition->charge(), chargeOf("/circuits/nests.panda", bytes, *definition));
    QVERIFY(definition->charge() >= bytes.size() + blob.size());
}

void TestICDefinition::testReleasedDefinitionIsKeptAndReused()
{
    const QByteArray bytes = pandaBytes("kept");
    auto first = ICDefinition::acquire("/circuits/kept.panda", bytes);
    const ICDefinition *const address = first.get();
    const int parses = ICDefinition::parseCount();
    first.reset();

    const auto again = ICDefinition::acquire("/circuits/kept.panda", bytes);
    QCOMPARE(again.get(), address);
    QCOMPARE(ICDefinition::parseCount(), parses);
}

void TestICDefinition::testInUseDefinitionIsNeverEvicted()
{
    // Whatever the budget, a definition an instance holds is found again, not parsed again
    ICDefinition::setBudget(0);
    const QByteArray bytes = pandaBytes("busy");
    const auto held = ICDefinition::acquire("/circuits/busy.panda", bytes);
    const int parses = ICDefinition::parseCount();

    const auto again = ICDefinition::acquire("/circuits/busy.panda", bytes);
    QCOMPARE(again.get(), held.get());
    QCOMPARE(ICDefinition::parseCount(), parses);
}

void TestICDefinition::testReleasedDefinitionIsChargedItsSize()
{
    // Kept at exactly the budget, and not kept one byte under it
    const QByteArray bytes = pandaBytes("sized", 4000);
    auto held = ICDefinition::acquire("/circuits/sized.panda", bytes);
    const qsizetype size = held->charge();
    QCOMPARE(size, chargeOf("/circuits/sized.panda", bytes, *held));

    ICDefinition::setBudget(size);
    const ICDefinition *const address = held.get();
    held.reset();
    const int parses = ICDefinition::parseCount();
    auto again = ICDefinition::acquire("/circuits/sized.panda", bytes);
    QCOMPARE(again.get(), address);
    QCOMPARE(ICDefinition::parseCount(), parses);

    again.reset();
    ICDefinition::dropReleased();
    ICDefinition::setBudget(size - 1);
    auto oversized = ICDefinition::acquire("/circuits/sized.panda", bytes);
    oversized.reset();
    const int reparses = ICDefinition::parseCount();
    const auto reparsed = ICDefinition::acquire("/circuits/sized.panda", bytes);
    QCOMPARE(ICDefinition::parseCount() - reparses, 1);
    QVERIFY(reparsed != nullptr);
}

void TestICDefinition::testLeastRecentlyReleasedGoesFirst()
{
    // Alpha is inserted first, beta is released first: under a budget for one, beta goes, not the older insertion
    const QByteArray alphaBytes = pandaBytes("alpha");
    const QByteArray betaBytes = pandaBytes("beta");
    auto alpha = ICDefinition::acquire("/circuits/alpha.panda", alphaBytes);
    auto beta = ICDefinition::acquire("/circuits/beta.panda", betaBytes);
    const qsizetype alphaSize = alpha->charge();

    beta.reset();
    alpha.reset();
    ICDefinition::setBudget(alphaSize);

    const int parses = ICDefinition::parseCount();
    const auto alphaAgain = ICDefinition::acquire("/circuits/alpha.panda", alphaBytes);
    QCOMPARE(ICDefinition::parseCount(), parses);
    const auto betaAgain = ICDefinition::acquire("/circuits/beta.panda", betaBytes);
    QCOMPARE(ICDefinition::parseCount() - parses, 1);
    QVERIFY(betaAgain != nullptr);
}

void TestICDefinition::testFailedParseKeepsNothing()
{
    const QByteArray junk("not a panda file at all");
    const int parses = ICDefinition::parseCount();

    bool threw = false;
    try {
        ICDefinition::acquire("/circuits/junk.panda", junk);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY(threw);
    QCOMPARE(ICDefinition::parseCount(), parses);

    // Nothing was kept, so the same bytes are parsed again, and fail again
    threw = false;
    try {
        ICDefinition::acquire("/circuits/junk.panda", junk);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY(threw);
    QCOMPARE(ICDefinition::parseCount(), parses);
}

void TestICDefinition::testLoadedICHoldsItsDefinition()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("held.panda");
    writeFile(path, pandaBytes("held"));

    // Nothing released is kept, so the second load finds the definition only through the first IC, which holds it
    ICDefinition::setBudget(0);
    const int parses = ICDefinition::parseCount();
    auto first = std::make_unique<IC>();
    first->loadFile(path, dir.path());
    QCOMPARE(ICDefinition::parseCount() - parses, 1);

    auto second = std::make_unique<IC>();
    second->loadFile(path, dir.path());
    QCOMPARE(ICDefinition::parseCount() - parses, 1);
}

void TestICDefinition::testFileCacheSuppliesTheBytes()
{
    // The file on disk is not a panda file: the load succeeds only if the bytes come from the cache
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("cached.panda");
    writeFile(path, QByteArray("garbage on disk, not a panda"));

    FakeFileCache cache(pandaBytes("cached"));
    auto ic = std::make_unique<IC>();
    ic->setFileCache(&cache);
    ic->loadFile(path, dir.path());
    QCOMPARE(QFileInfo(ic->file()).absoluteFilePath(), QFileInfo(path).absoluteFilePath());
}

void TestICDefinition::testSceneRemovalDropsTheFileCache()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    auto *ic = new IC();
    scene->addItem(ic);
    QCOMPARE(ic->fileCache(), static_cast<ICFileCache *>(scene->icRegistry()));

    scene->removeItem(ic);
    QVERIFY(ic->fileCache() == nullptr);
    delete ic;
}
