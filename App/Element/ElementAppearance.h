// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Owns an Element's appearance lists and the picture it shows.
 */

#pragma once

#include <optional>

#include <QCoreApplication>
#include <QPixmap>
#include <QStringList>

#include "App/Element/PictureLibrary.h"

class Element;

/**
 * \class ElementAppearance
 * \brief Owns the pictures an Element can show: the default/alternative appearance lists, the picture each slot of
 * them loaded, and the procedural render pixmap of a body drawn as vectors.
 *
 * \details The pictures are loaded, oriented, sized and drawn by presentation, through the installed PictureLibrary:
 * this keeps only the id each slot's picture is loaded as. Which slot an element shows is its presenter's to pick
 * from the element's state. Orientation state (rotation/flip) stays on the owning element;
 * this type reaches it through a narrow owner reference, which it also uses to report the picture's changes.
 */
class ElementAppearance
{
    Q_DECLARE_TR_FUNCTIONS(ElementAppearance)

public:
    /// Constructs the appearance bound to its owning \a owner element.
    explicit ElementAppearance(Element *owner);

    /// Seeds the appearance lists from the type's descriptor (lazily deriving defaults from
    /// \a pixmapPath when \a defaultAppearances is empty; alternatives default to the defaults).
    void seed(const QStringList &defaultAppearances,
              const QStringList &alternativeAppearances,
              const QString &pixmapPath);

    // --- The picture shown ---

    /// Returns the picture of slot \a slot, as the installed PictureLibrary loaded it; nothing for a slot out of range
    /// or one with no picture.
    std::optional<PictureId> picture(int slot) const
    {
        return ((slot >= 0) && (slot < m_pictures.size())) ? m_pictures.at(slot) : std::nullopt;
    }

    /// Returns the path of the picture of slot \a slot: the one asked for, or the slot's default when that did not
    /// load; empty for a slot out of range or one with no picture.
    QString picturePath(int slot) const { return m_picturePaths.value(slot); }

    /// Returns the procedural render pixmap (setRenderPixmap()); null for an element that shows a picture.
    QPixmap renderPixmap() const { return m_hasCustomRenderPixmap ? m_renderPixmap : QPixmap(); }

    /// Returns true while the element draws a procedural body instead of its picture (setRenderPixmap()).
    bool hasRenderPixmap() const { return m_hasCustomRenderPixmap; }

    /// Returns true if at least one default appearance is available.
    bool hasDefaultAppearances() const { return !m_defaultAppearances.isEmpty(); }

    /**
     * \brief Loads the picture of each slot of the active list whose path changed, and reports the pictures replaced.
     * \details Called whenever the lists change. A picture that does not load (its file gone, or not an image) gives
     * its slot the slot's default and the slot keeps its path: a state change or an undo goes on. Opening a file
     * reports it (reportFailedPicture()).
     */
    void appearanceChanged();

    /// Throws a Pandaception naming the first slot's picture that did not load, and why, while its default is shown.
    void reportFailedPicture() const;

    /// Replaces the picture with a procedurally-built body pixmap (Mux/Demux/TruthTable/IC).
    /// From then on the owner draws its own body from this pixmap's footprint: orientation
    /// changes leave it untouched, and the owner turns about its body's centre.
    void setRenderPixmap(const QPixmap &pixmap);

    // --- Appearance list ---

    /// Switches the appearance: restores defaults or applies \a fileName at slot 0.
    void setAppearance(const bool defaultAppearance, const QString &fileName);

    /// Sets a custom appearance at \a index (empty \a fileName restores that slot's default).
    void setAppearanceAt(const int index, const QString &fileName);

    /// Returns the built-in default appearance paths.
    const QStringList &defaultAppearances() const { return m_defaultAppearances; }

    /// Returns the active (possibly user-customised) appearance paths.
    const QStringList &alternativeAppearances() const { return m_alternativeAppearances; }

    /**
     * \brief Makes \a pixmapPath the default appearance of a type with no built-in appearances, which shows its pixmap
     * path's picture: a path that names the theme's picture changes with the theme.
     * \details The slot follows it unless it holds a custom appearance, and the picture reloads. Nothing changes when
     * \a pixmapPath is empty or already the default.
     */
    void setDefaultPixmapPath(const QString &pixmapPath);

    /// Copies the default appearances over the alternative list (restore-to-default).
    void resetAlternativeToDefault() { m_alternativeAppearances = m_defaultAppearances; }

    /// Sets one alternative-appearance slot without reloading the picture (serializer / subclass use).
    void setAlternativeAppearanceAt(const int index, const QString &path);

    /// Sets the "using built-in default appearance" flag.
    void setUsingDefaultAppearance(const bool value) { m_usingDefaultAppearance = value; }

    /// Returns true when the active appearance still matches the built-in default.
    bool usingDefaultAppearance() const { return m_usingDefaultAppearance; }

    /// Recomputes the using-default flag by comparing the alternative and default lists.
    void recomputeUsingDefault();

    /// Returns the external (non-resource) appearance file paths the element depends on.
    QStringList externalFiles() const;

private:
    Element *m_owner = nullptr;

    QList<std::optional<PictureId>> m_pictures; ///< Each slot's picture, as the PictureLibrary loaded it.
    QStringList m_picturePaths;                 ///< Each slot's picture's path: the one asked for, or the slot's default.
    QStringList m_loadedPaths;                  ///< Each slot's path as appearanceChanged() last loaded it (change guard).
    QStringList m_failedPaths;                  ///< Each slot's path when it did not load, else empty.
    QPixmap m_renderPixmap;               ///< Procedural body pixmap; only its size is used (setRenderPixmap()).
    bool m_hasCustomRenderPixmap = false; ///< True while the owner draws m_renderPixmap's footprint instead of the picture.

    QString m_pixmapPath;         ///< Resource/file path of the default pixmap (from the descriptor).

    QStringList m_defaultAppearances;     ///< Built-in appearance paths (resource files).
    QStringList m_alternativeAppearances; ///< Active appearance paths (user paths override defaults).
    bool m_usingDefaultAppearance = true; ///< True while the active appearance matches the default.
};
