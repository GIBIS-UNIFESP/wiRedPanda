// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief The pictures elements show: loaded once per file version, oriented, sized and drawn.
 */

#pragma once

#include <memory>

#include <QPixmap>

#include "App/Core/Enums.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/PictureLibrary.h"

class Element;
class QPainter;
class QSize;
class QSvgRenderer;

/// A picture as an element shows it in its orientation.
struct PictureVariant {
    QPixmap pixmap;                         ///< The raster: the picture as loaded, or an SVG drawn with its labels counter-oriented.
    std::shared_ptr<QSvgRenderer> renderer; ///< An SVG's vector renderer, its labels counter-oriented as the raster's; null for a raster file.
    bool oriented = false;                  ///< Whether the raster is a counter-oriented variant rather than the picture as loaded.
};

/**
 * \class ElementPictures
 * \brief Loads, orients, sizes and draws the pictures elements show; the model's PictureLibrary.
 * \details Each version of a file (its path, modification time and size) is loaded once and shared by every element
 * that shows it. A rotatable element whose SVG picture has labels (a `<text>`, or an element marked `class="label"`)
 * is shown with them counter-oriented, so the glyphs read upright after the element's own rotation and flip; the
 * variants are built from the SVG's source, read as the picture loaded, as each orientation is first shown. Neither
 * versions nor variants are dropped: their number is bounded by the pictures the program shows and the orientations it
 * shows them in, not by circuit size. GUI-thread only.
 *
 * A built-in picture drawn for each theme (the latches' and flip-flops') is named by the Light theme's, in a Light
 * folder with a Dark one beside it (themed()); an element shows the sibling of the theme it is drawn in, and is
 * measured on the picture it names, the Light one, which is the size of the Dark one.
 */
class ElementPictures final : public PictureLibrary
{
public:
    /// Returns the one instance, installed as the model's PictureLibrary by installPresenters().
    static const ElementPictures &instance();

    std::optional<PictureId> load(const QString &path) const override;

    /// Returns \a element's picture as it shows it in \a theme, in its orientation now; null while it shows none.
    /// System is drawn as Light.
    const PictureVariant *shown(const Element &element, Theme theme) const;

    /// Draws \a variant at the origin with \a painter: an SVG from its vector renderer (crisp at any zoom), a raster
    /// file from its pixmap.
    static void draw(QPainter &painter, const PictureVariant &variant);

    /// Returns a preview of the picture at \a path scaled to fit \a size; null if it does not load.
    static QPixmap preview(const QString &path, const QSize &size);

    /**
     * \brief Returns the picture at \a path as \a theme shows it.
     * \details A built-in picture in a Light folder is shown in the Dark theme as its sibling in the Dark folder beside
     * it, when there is one; any other path, and any path in another theme, is returned as it is.
     */
    static QString themed(const QString &path, Theme theme);

    /// Returns the picture the palette and the menus show for \a type, in the current theme; null for
    /// ElementType::Unknown and for a type with no picture (IC).
    static QPixmap typePicture(ElementType type);

private:
    ElementPictures() = default;
};
