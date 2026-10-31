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
 * \brief Owns which picture an Element shows: the default/alternative appearance lists, the picture shown from them,
 * and the procedural render pixmap of a body drawn as vectors.
 *
 * \details The pictures are loaded, oriented, sized and drawn by presentation, through the installed PictureLibrary:
 * this keeps only the id each picture is loaded as. Orientation state (rotation/flip) stays on the owning element;
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

    /// Returns the picture shown, as the installed PictureLibrary loaded it; nothing before one loads.
    std::optional<PictureId> picture() const { return m_picture; }

    /// Returns the path of the picture shown; empty before one loads.
    const QString &picturePath() const { return m_picturePath; }

    /// Returns the procedural render pixmap (setRenderPixmap()); null for an element that shows a picture.
    QPixmap renderPixmap() const { return m_hasCustomRenderPixmap ? m_renderPixmap : QPixmap(); }

    /// Returns true while the element draws a procedural body instead of its picture (setRenderPixmap()).
    bool hasRenderPixmap() const { return m_hasCustomRenderPixmap; }

    /// Returns true if at least one default appearance is available.
    bool hasDefaultAppearances() const { return !m_defaultAppearances.isEmpty(); }

    /**
     * \brief Shows the appearance at position \a index in the active list.
     * \details A picture that does not load (its file gone, or not an image) shows the slot's default in its place and
     * the slot keeps its path: a state change or an undo goes on. Opening a file reports it (reportFailedPicture()).
     */
    void setPixmap(const int index);

    /// Throws a Pandaception naming the picture last applied and why, when it did not load and its default is shown.
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

    std::optional<PictureId> m_picture;   ///< The picture shown, as the PictureLibrary loaded it.
    QString m_picturePath;                ///< The path of the picture shown: the one asked for, or the slot's default.
    QPixmap m_renderPixmap;               ///< Procedural body pixmap; only its size is used (setRenderPixmap()).
    bool m_hasCustomRenderPixmap = false; ///< True while the owner draws m_renderPixmap's footprint instead of the picture.

    QString m_pixmapPath;         ///< Resource/file path of the default pixmap (from the descriptor).
    QString m_currentPixmapPath;  ///< Path last requested via setPixmap() (change guard).
    QString m_failedPixmapPath;   ///< The picture last applied when it did not load, else empty.

    QStringList m_defaultAppearances;     ///< Built-in appearance paths (resource files).
    QStringList m_alternativeAppearances; ///< Active appearance paths (user paths override defaults).
    bool m_usingDefaultAppearance = true; ///< True while the active appearance matches the default.
};
