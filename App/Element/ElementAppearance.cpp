// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementAppearance.h"

#include <algorithm>

#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Element/Element.h"

ElementAppearance::ElementAppearance(Element *owner)
    : m_owner(owner)
{
}

void ElementAppearance::seed(const QStringList &defaultAppearances,
                                         const QStringList &alternativeAppearances,
                                         const QString &pixmapPath)
{
    m_defaultAppearances = defaultAppearances;
    m_pixmapPath = pixmapPath;

    // For elements whose pixmapPath is theme-dependent (e.g. memory elements),
    // defaultAppearances may be left empty in the descriptor to avoid evaluating the
    // theme path during static initialisation (before QApplication exists).
    // Populate it lazily here from the now-correct pixmapPath.
    if (m_defaultAppearances.isEmpty() && !m_pixmapPath.isEmpty()) {
        m_defaultAppearances = QStringList({m_pixmapPath});
    }

    m_alternativeAppearances = alternativeAppearances.isEmpty() ? m_defaultAppearances : alternativeAppearances;
}

void ElementAppearance::appearanceChanged()
{
    const QStringList &active = m_usingDefaultAppearance ? m_defaultAppearances : m_alternativeAppearances;
    if (m_loadedPaths == active) {
        return;
    }

    // The new pictures, the body they give and the pivot it moves are reported as one change.
    const Element::ChangeBatch batch(*m_owner);

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
    }

    if (!replaced) {
        return;
    }

    // Loading a picture hands display ownership back from a procedural render pixmap.
    m_hasCustomRenderPixmap = false;

    // The rotation pivots on the body's centre, so the pivot follows the picture whenever it changes.
    m_owner->pictureReplaced();
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

void ElementAppearance::setRenderPixmap(const QPixmap &pixmap)
{
    // As in appearanceChanged(): the new picture, its body and the pivot it moves are one change.
    const Element::ChangeBatch batch(*m_owner);
    m_renderPixmap = pixmap;
    m_hasCustomRenderPixmap = true;
    // The owner's body is drawn from this footprint, so rotation must pivot on its centre —
    // the picture flow gets the same guarantee from appearanceChanged().
    m_owner->pictureReplaced();
}

void ElementAppearance::setDefaultPixmapPath(const QString &pixmapPath)
{
    if (pixmapPath.isEmpty() || (m_defaultAppearances == QStringList{pixmapPath})) {
        return;
    }

    const QString previous = m_defaultAppearances.value(0);
    m_pixmapPath = pixmapPath;
    m_defaultAppearances = QStringList{pixmapPath};
    if (m_alternativeAppearances.value(0) == previous) {
        m_alternativeAppearances = m_defaultAppearances;
    }
    appearanceChanged();
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
    // InputRotary (hardcoded 0), Led (colorIndex(), always in its own valid range), and
    // ElementSerializer's two appearance-name loaders (which already throw on an
    // out-of-range index *before* ever reaching this call).
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
