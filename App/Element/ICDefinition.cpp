// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICDefinition.h"

#include <unordered_map>
#include <utility>

#include <QCoreApplication>
#include <QDataStream>
#include <QHash>
#include <QMap>
#include <QThread>

#include "App/IO/Serialization.h"

namespace {

/// The budget, in bytes, of the released definitions kept, until a test sets another.
constexpr qsizetype kDefaultBudget = 32 * 1024 * 1024;

/// One definition the cache knows: the key it was parsed from, the weak reference of the instances holding it, and, once
/// released, the definition itself with the stamp of its release.
struct Entry {
    QByteArray key;
    std::weak_ptr<ICDefinition> live;
    std::unique_ptr<ICDefinition> released;
    quint64 stamp = 0;
};

/// The cache's state, on the main thread only.
struct Cache {
    QHash<QByteArray, quint64> ids;   ///< Key (path, NUL, bytes) to id.
    std::unordered_map<quint64, Entry> entries; ///< Id to entry; a value holds its definition, so it is not copied.
    QMap<quint64, quint64> lru;       ///< Release stamp to id, for the kept definitions: the first goes first.
    qsizetype keptBytes = 0;          ///< Bytes charged to the kept definitions.
    qsizetype budget = kDefaultBudget;
    quint64 nextId = 1;
    quint64 clock = 0;
    int parses = 0;
};

/// Returns the cache. It is never destroyed: a definition's last holder can outlive the statics.
Cache &cache()
{
    static Cache *instance = new Cache;
    return *instance;
}

/// Forgets the kept definition \a id, and frees it.
void drop(Cache &c, const quint64 id)
{
    const auto found = c.entries.find(id);
    if (found == c.entries.end()) {
        return;
    }
    Entry &entry = found->second;
    c.keptBytes -= entry.released->charge();
    c.lru.remove(entry.stamp);
    c.ids.remove(entry.key);
    c.entries.erase(found);
}

/// Drops the least recently released definitions until the kept ones fit the budget.
void trim(Cache &c)
{
    while (c.keptBytes > c.budget && !c.lru.isEmpty()) {
        drop(c, c.lru.begin().value());
    }
}

} // namespace

ICDefinition::ICDefinition(const QString &path, const QByteArray &bytes)
{
    QByteArray data(bytes);
    QDataStream stream(&data, QIODevice::ReadOnly);
    auto preamble = Serialization::readPreamble(stream);
    m_version = preamble.version;
    m_metadata = preamble.metadata;
    m_blobRegistry = Serialization::deserializeBlobRegistry(m_metadata, m_version);
    // The blobs are held once, in m_blobRegistry: the metadata's copies of them go, with any copies of the files they nest.
    for (const char *key : {"embeddedICs", "blobRegistry", "fileCopies"}) {
        m_metadata.remove(QString::fromLatin1(key));
    }
    m_records = preamble.remainingPayload;
    // The memory the definition holds, counted once: the records, the blobs, and the cache key's copy of the path and the
    // bytes. The scalars of the metadata are not counted.
    qsizetype blobBytes = 0;
    for (const QByteArray &blob : std::as_const(m_blobRegistry)) {
        blobBytes += blob.size();
    }
    m_charge = m_records.size() + blobBytes + path.toUtf8().size() + 1 + bytes.size();
}

std::shared_ptr<const ICDefinition> ICDefinition::acquire(const QString &path, const QByteArray &bytes)
{
    Q_ASSERT(QCoreApplication::instance()->thread() == QThread::currentThread());
    auto &c = cache();

    QByteArray key = path.toUtf8();
    key.append('\0');
    key.append(bytes);

    if (const quint64 id = c.ids.value(key, 0); id != 0) {
        Entry &entry = c.entries[id];
        if (auto live = entry.live.lock()) {
            return live;
        }
        if (entry.released) {
            // Kept after its last use, and in use again: it leaves the kept set
            ICDefinition *definition = entry.released.release();
            c.lru.remove(entry.stamp);
            c.keptBytes -= definition->m_charge;
            return adopt(id, definition);
        }
    }

    // Parsed before anything is recorded, so bytes that do not parse keep nothing
    auto *definition = new ICDefinition(path, bytes);
    ++c.parses;
    const quint64 id = c.nextId++;
    Entry entry;
    entry.key = key;
    c.entries.emplace(id, std::move(entry));
    c.ids.insert(key, id);
    return adopt(id, definition);
}

std::shared_ptr<const ICDefinition> ICDefinition::adopt(const quint64 id, ICDefinition *definition)
{
    std::shared_ptr<ICDefinition> shared(definition, [id](ICDefinition *done) { onReleased(id, done); });
    cache().entries[id].live = shared;
    return shared;
}

void ICDefinition::onReleased(const quint64 id, ICDefinition *definition)
{
    auto &c = cache();
    const auto found = c.entries.find(id);
    if (found == c.entries.end()) {
        delete definition;
        return;
    }
    Entry &entry = found->second;
    entry.live.reset();

    // One larger than the budget is not kept
    if (definition->m_charge > c.budget) {
        c.ids.remove(entry.key);
        c.entries.erase(found);
        delete definition;
        return;
    }

    entry.stamp = ++c.clock;
    entry.released.reset(definition);
    c.lru.insert(entry.stamp, id);
    c.keptBytes += definition->m_charge;
    trim(c);
}

void ICDefinition::setBudget(const qsizetype bytes)
{
    auto &c = cache();
    c.budget = bytes;
    trim(c);
}

qsizetype ICDefinition::budget()
{
    return cache().budget;
}

int ICDefinition::parseCount()
{
    return cache().parses;
}

void ICDefinition::dropReleased()
{
    auto &c = cache();
    while (!c.lru.isEmpty()) {
        drop(c, c.lru.begin().value());
    }
}
