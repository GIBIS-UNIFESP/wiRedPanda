// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementAppearance.h"

#include <algorithm>

#include <QFileInfo>
#include <QHash>
#include <QSet>

#include "App/Core/Common.h"
#include "App/Element/Element.h"

namespace ElementAppearanceDetail {

/// The appearances whose active list has each file path in a slot, as their last loads left them; GUI-thread only.
QHash<QString, QSet<ElementAppearance *>> &showing()
{
    static QHash<QString, QSet<ElementAppearance *>> appearances;
    return appearances;
}

/// The version each file path's last load gave, nothing for one that did not load; GUI-thread only.
QHash<QString, std::optional<PictureId>> &versions()
{
    static QHash<QString, std::optional<PictureId>> loaded;
    return loaded;
}

/// Whether \a path names a file, which can change on disk: a resource's picture is the one it was built with, so a
/// load of it never gives another version and the registry leaves it out.
bool isFile(const QString &path)
{
    return !path.isEmpty() && !path.startsWith(":/");
}

/// Takes \a appearance out of the registry of each file path in \a paths.
void leave(ElementAppearance *appearance, const QStringList &paths)
{
    auto &appearances = showing();
    for (const QString &path : paths) {
        if (!isFile(path)) {
            continue;
        }
        const auto entry = appearances.find(path);
        if (entry == appearances.end()) {
            continue;
        }
        entry->remove(appearance);
        if (entry->isEmpty()) {
            appearances.erase(entry);
        }
    }
}

} // namespace ElementAppearanceDetail

ElementAppearance::ElementAppearance(Element *owner)
    : m_owner(owner)
{
}

ElementAppearance::~ElementAppearance()
{
    ElementAppearanceDetail::leave(this, m_loadedPaths);
}

void ElementAppearance::seed(const QStringList &defaultAppearances)
{
    m_defaultAppearances = defaultAppearances;
    m_alternativeAppearances = m_defaultAppearances;
}

void ElementAppearance::appearanceChanged()
{
    const QStringList &active = m_usingDefaultAppearance ? m_defaultAppearances : m_alternativeAppearances;
    if (m_loadedPaths == active) {
        return;
    }

    // The new pictures, the body they give and the pivot it moves are reported as one change.
    const Element::ChangeBatch batch(*m_owner);

    ElementAppearanceDetail::leave(this, m_loadedPaths);
    QStringList loaded; // The file paths this change loaded, each a version every appearance showing it takes

    const qsizetype count = active.size();
    m_pictures.resize(count);
    m_picturePaths.resize(count);
    m_loadedPaths.resize(count);
    m_failedPaths.resize(count);

    bool replaced = false;
    for (qsizetype slot = 0; slot < count; ++slot) {
        const QString &asked = active.at(slot);
        if (asked == m_loadedPaths.at(slot)) {
            continue;
        }
        m_loadedPaths[slot] = asked;
        if (asked.isEmpty()) {
            continue;
        }

        // Resolution against contextDir already happened once, at load time (see
        // ExternalFilePath::forReading(), called from ElementSerializer) — by the
        // time a path reaches here it's either a resource reference or already directly
        // usable (a fresh absolute path from a file picker, or an already-resolved one
        // read back from a saved project). Nothing left to do here but load it.
        QString path = asked;
        m_failedPaths[slot].clear();

        // The library loads each version of a file once, for every element that shows it.
        std::optional<PictureId> picture = PictureLibrary::installed().load(path);
        if (!picture) {
            // Nothing here may throw: a state change runs inside Qt's event handlers and an undo inside QUndoStack,
            // which an exception cannot pass on macOS. The slot's default is shown; the guard above keeps a later
            // change of the lists from loading the same broken path again.
            qCDebug(zero) << "Problem loading pixmapPath: " << path;
            m_failedPaths[slot] = path;
            path = m_defaultAppearances.value(slot);
            picture = path.isEmpty() ? std::nullopt : PictureLibrary::installed().load(path);
        }

        m_pictures[slot] = picture;
        m_picturePaths[slot] = path;
        replaced = true;
        if (ElementAppearanceDetail::isFile(asked)) {
            loaded.append(asked);
        }
    }

    auto &showing = ElementAppearanceDetail::showing();
    for (const QString &path : std::as_const(m_loadedPaths)) {
        if (ElementAppearanceDetail::isFile(path)) {
            showing[path].insert(this);
        }
    }

    if (!replaced) {
        return;
    }

    // The rotation pivots on the body's centre, so the pivot follows the picture whenever it changes.
    m_owner->pictureReplaced();

    // A new or failed version of a file is the picture of every element showing its path, which each of them
    // measures; a load giving the version the path last gave changes nothing
    auto &versions = ElementAppearanceDetail::versions();
    for (const QString &path : std::as_const(loaded)) {
        const qsizetype slot = m_loadedPaths.indexOf(path);
        const std::optional<PictureId> version = m_failedPaths.at(slot).isEmpty() ? m_pictures.at(slot) : std::nullopt;
        const auto last = versions.constFind(path);
        const bool changed = (last != versions.constEnd()) && (last.value() != version);
        versions.insert(path, version);
        if (!changed) {
            continue;
        }
        const QSet<ElementAppearance *> others = showing.value(path);
        for (ElementAppearance *other : others) {
            if (other != this) {
                other->showVersion(path, version);
            }
        }
    }
}

qsizetype ElementAppearance::showingCount(const QString &path)
{
    return ElementAppearanceDetail::showing().value(path).size();
}

void ElementAppearance::showVersion(const QString &path, const std::optional<PictureId> picture)
{
    const Element::ChangeBatch batch(*m_owner);
    bool replaced = false;
    for (qsizetype slot = 0; slot < m_loadedPaths.size(); ++slot) {
        if (m_loadedPaths.at(slot) != path) {
            continue;
        }
        // A version that does not load gives the slot its default, as a load of it would
        const bool fails = !picture.has_value();
        const QString shownPath = fails ? m_defaultAppearances.value(slot) : path;
        const std::optional<PictureId> shown = !fails ? picture
            : (shownPath.isEmpty() ? std::nullopt : PictureLibrary::installed().load(shownPath));
        if ((m_pictures.at(slot) == shown) && (m_failedPaths.at(slot).isEmpty() != fails)) {
            continue;
        }
        m_pictures[slot] = shown;
        m_picturePaths[slot] = shownPath;
        m_failedPaths[slot] = fails ? path : QString();
        replaced = true;
    }
    if (replaced) {
        m_owner->pictureReplaced();
    }
}

void ElementAppearance::reportFailedPicture() const
{
    const auto failed = std::find_if(m_failedPaths.cbegin(), m_failedPaths.cend(),
                                     [](const QString &path) { return !path.isEmpty(); });
    if (failed == m_failedPaths.cend()) {
        return;
    }

    const QFileInfo info(*failed);
    const QString reason = !info.exists()
                               ? tr("File does not exist")
                               : !info.isReadable()
                                     ? tr("File is not readable")
                                     : tr("Unknown reason");
    throw PANDACEPTION("Couldn't load pixmap: %1 (%2)", *failed, reason);
}

void ElementAppearance::setAppearance(const bool defaultAppearance, const QString &fileName)
{
    if (defaultAppearance) {
        m_alternativeAppearances = m_defaultAppearances;
    } else {
        m_alternativeAppearances[0] = fileName;
    }

    m_usingDefaultAppearance = defaultAppearance;
    appearanceChanged();
}

void ElementAppearance::setAppearanceAt(const int index, const QString &fileName)
{
    if (index < 0 || index >= m_alternativeAppearances.size()) {
        return;
    }

    if (fileName.isEmpty()) {
        m_alternativeAppearances[index] = m_defaultAppearances.at(index);
    } else {
        m_alternativeAppearances[index] = fileName;
    }

    m_usingDefaultAppearance = (m_alternativeAppearances == m_defaultAppearances);
    appearanceChanged();
}

void ElementAppearance::setAlternativeAppearanceAt(const int index, const QString &path)
{
    // Unreachable: every call site passes a provably in-range index -- ElementInput
    // (static_cast<int>(isOn()), always 0/1 for its always-2-slot appearance list),
    // InputRotary (hardcoded 0), Element::applyAppearance() (the descriptor's shownSlot, an
    // LED's within its 26 slots), and ElementSerializer's two appearance-name loaders (which
    // already throw on an out-of-range index *before* ever reaching this call).
    if (index < 0 || index >= m_alternativeAppearances.size()) { // LCOV_EXCL_LINE
        return; // LCOV_EXCL_LINE
    }
    m_alternativeAppearances[index] = path;
}

void ElementAppearance::recomputeUsingDefault()
{
    m_usingDefaultAppearance = std::equal(
        m_defaultAppearances.begin(), m_defaultAppearances.end(),
        m_alternativeAppearances.begin(), m_alternativeAppearances.end()
        );
}

QStringList ElementAppearance::externalFiles() const
{
    QStringList result;
    for (int i = 0; i < m_alternativeAppearances.size() && i < m_defaultAppearances.size(); ++i) {
        const QString &appearance = m_alternativeAppearances.at(i);
        if (appearance != m_defaultAppearances.at(i) && !appearance.startsWith(":/")) {
            result.append(appearance);
        }
    }
    return result;
} // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned QStringList, never reached after the return above.
