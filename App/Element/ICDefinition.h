// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ICDefinition: an IC's .panda bytes, parsed once and shared by the instances that load them.
 */

#pragma once

#include <memory>

#include <QByteArray>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QVersionNumber>

/**
 * \class ICDefinition
 * \brief An IC's .panda bytes, parsed once: the format revision, the metadata, the embedded blob registry, and the
 *        element records, already decompressed.
 *
 * \details Each instance builds its own elements from the records, so the parse the instances share is the part that
 * does not depend on the instance. acquire() returns the definition of a file (keyed by its absolute path and its bytes)
 * or of a blob (keyed by its bytes), so a definition is reused only for the bytes it was parsed from.
 *
 * While an instance holds a definition it is found through a weak reference: it is never parsed twice and never evicted.
 * After its last holder lets go it is kept, charged its size in bytes, under a budget. The least recently released go
 * first, and a definition larger than the budget is not kept at all. The cache is used on the main thread only.
 */
class ICDefinition
{
public:
    /// Returns the definition of \a bytes, read from the file at \a path (empty for a blob). It is parsed only when no
    /// instance holds or keeps one for the same path and bytes. Throws, and keeps nothing, when \a bytes do not parse.
    static std::shared_ptr<const ICDefinition> acquire(const QString &path, const QByteArray &bytes);

    /// Returns the format revision the bytes were written in.
    QVersionNumber version() const { return m_version; }

    /// Returns the metadata map of the file.
    const QMap<QString, QVariant> &metadata() const { return m_metadata; }

    /// Returns the embedded IC blobs the file holds, by name.
    const QMap<QString, QByteArray> &blobRegistry() const { return m_blobRegistry; }

    /// Returns the element records (elements and connections), already decompressed.
    const QByteArray &records() const { return m_records; }

    /// Returns the size, in bytes, this definition is charged in the cache.
    qsizetype charge() const { return m_charge; }

    /// Sets the budget, in bytes, of the released definitions kept (a test seam).
    static void setBudget(qsizetype bytes);

    /// Returns the budget, in bytes, of the released definitions kept.
    static qsizetype budget();

    /// Returns how many definitions have been parsed so far (a test seam).
    static int parseCount();

    /// Drops every released definition kept (a test seam).
    static void dropReleased();

private:
    ICDefinition(const QString &path, const QByteArray &bytes);

    static std::shared_ptr<const ICDefinition> adopt(quint64 id, ICDefinition *definition);
    static void onReleased(quint64 id, ICDefinition *definition);

    QVersionNumber m_version;
    QMap<QString, QVariant> m_metadata;
    QMap<QString, QByteArray> m_blobRegistry;
    QByteArray m_records;
    qsizetype m_charge = 0;
};
