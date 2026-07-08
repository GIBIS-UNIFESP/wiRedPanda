// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Owns a GraphicElement's pixmap / SVG appearance, appearance list, and selection paint.
 */

#pragma once

#include <memory>

#include <QColor>
#include <QCoreApplication>
#include <QPixmap>
#include <QSize>
#include <QStringList>

class GraphicElement;
class QPainter;
class QSvgRenderer;
class ThemeAttributes;

/**
 * \class ElementAppearance
 * \brief Owns the visual appearance of a GraphicElement: the displayed pixmap, its SVG
 * renderer, the default/alternative appearance lists, and the selection-highlight colors.
 *
 * \details Extracted from GraphicElement so the rendering bulk (pixmap loading, SVG text
 * counter-orientation, the per-appearance caches, and the paint body) lives in one cohesive
 * unit.  Orientation state (rotation/flip) stays on the owning element; this type reads it
 * back through a narrow owner reference, which it also uses to trigger repaints and resolve
 * the element's context directory.
 */
class ElementAppearance
{
    Q_DECLARE_TR_FUNCTIONS(ElementAppearance)

public:
    /// Constructs the appearance bound to its owning \a owner element.
    explicit ElementAppearance(GraphicElement *owner);

    /// Out-of-line so the unique_ptr to the forward-declared QSvgRenderer can be destroyed.
    ~ElementAppearance();

    /// Seeds the appearance lists from element metadata (lazily deriving defaults from
    /// \a pixmapPath when \a defaultAppearances is empty; alternatives default to the defaults).
    void seedFromMetadata(const QStringList &defaultAppearances,
                          const QStringList &alternativeAppearances,
                          const QString &pixmapPath);

    // --- Pixmap access ---

    /// Returns the pixmap currently displayed.
    QPixmap pixmap() const { return m_pixmap; }

    /// Returns the centre point of the displayed pixmap in local coordinates.
    QPointF pixmapCenter() const;

    /// Returns true if at least one default appearance is available.
    bool hasDefaultAppearances() const { return !m_defaultAppearances.isEmpty(); }

    /// Loads and applies the appearance at position \a index in the active list.
    void setPixmap(const int index);

    /// Loads and applies the pixmap located at \a pixmapPath.
    void setPixmap(const QString &pixmapPath);

    /// Returns a scaled-down preview of the appearance at \a index, without mutating the
    /// element's live displayed pixmap. Returns a null pixmap for an out-of-range index.
    QPixmap previewPixmapAt(const int index, const QSize &size) const;

    /// Replaces the displayed pixmap with a procedurally-built one (Mux/Demux/TruthTable/IC).
    /// From then on the owner draws its own body from this pixmap's footprint: orientation
    /// changes leave it untouched, and the owner's transform origin is kept at its centre.
    void setRenderPixmap(const QPixmap &pixmap);

    /// Re-derives the displayed pixmap (and SVG renderer) from the owner's current orientation.
    void applyOrientation();

    // --- Appearance list ---

    /// Switches the appearance: restores defaults or applies \a fileName at slot 0.
    void setAppearance(const bool defaultAppearance, const QString &fileName);

    /// Sets a custom appearance at \a index (empty \a fileName restores that slot's default).
    void setAppearanceAt(const int index, const QString &fileName);

    /// Returns the built-in default appearance paths.
    const QStringList &defaultAppearances() const { return m_defaultAppearances; }

    /// Returns the active (possibly user-customised) appearance paths.
    const QStringList &alternativeAppearances() const { return m_alternativeAppearances; }

    /// Copies the default appearances over the alternative list (restore-to-default).
    void resetAlternativeToDefault() { m_alternativeAppearances = m_defaultAppearances; }

    /// Sets one alternative-appearance slot without reloading the pixmap (serializer / subclass use).
    void setAlternativeAppearanceAt(const int index, const QString &path);

    /// Sets the "using built-in default appearance" flag.
    void setUsingDefaultAppearance(const bool value) { m_usingDefaultAppearance = value; }

    /// Returns true when the active appearance still matches the built-in default.
    bool usingDefaultAppearance() const { return m_usingDefaultAppearance; }

    /// Recomputes the using-default flag by comparing the alternative and default lists.
    void recomputeUsingDefault();

    /// Returns the external (non-resource) appearance file paths the element depends on.
    QStringList externalFiles() const;

    // --- Painting ---

    /// Draws the selection highlight (when \a selected) and the SVG/pixmap body onto \a painter.
    void render(QPainter *painter, const QRectF &boundingRect, const bool selected) const;

    /// Applies the selection-highlight colors from \a theme.
    void applyTheme(const ThemeAttributes &theme);

    /// Fill color of the selection highlight rectangle.
    QColor selectionBrush() const { return m_selectionBrush; }

    /// Border color of the selection highlight rectangle.
    QColor selectionPen() const { return m_selectionPen; }

private:
    /// Rebuilds m_svgRenderer from the resolved SVG path and the owner's orientation, or clears
    /// it for non-SVG (raster) appearances.
    void rebuildSvgRenderer();

    GraphicElement *m_owner = nullptr;

    QPixmap m_pixmap;     ///< Currently displayed pixmap.
    QPixmap m_basePixmap; ///< Upright, unflipped pixmap; m_pixmap is derived from this.
    bool m_hasCustomRenderPixmap = false; ///< True while m_pixmap is a procedural render pixmap, not derived from m_basePixmap.
    std::unique_ptr<QSvgRenderer> m_svgRenderer; ///< Vector renderer for the current SVG appearance (null for raster).

    QString m_pixmapPath;         ///< Resource/file path of the default pixmap (from metadata).
    QString m_currentPixmapPath;  ///< Path last requested via setPixmap() (change guard).
    QString m_resolvedPixmapPath; ///< Resolved path of m_basePixmap; used to build orientation variants.

    QStringList m_defaultAppearances;     ///< Built-in appearance paths (resource files).
    QStringList m_alternativeAppearances; ///< Active appearance paths (user paths override defaults).
    bool m_usingDefaultAppearance = true; ///< True while the active appearance matches the default.

    QColor m_selectionBrush; ///< Fill color for the selection highlight rectangle.
    QColor m_selectionPen;   ///< Border color for the selection highlight rectangle.
};
