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
 * variants are built from the SVG's source, read as the picture loaded, as each orientation is first shown; a picture
 * without labels, the same in every orientation, has one variant. Versions
 * are not dropped: their number is bounded by the pictures the program shows, not by circuit size. Variants are, since
 * an angle can be any number (MCP): those of every picture stay within variantBudget, each charged the bytes it holds,
 * the one shown least recently going first; a variant dropped is built again when its orientation is shown. GUI-thread
 * only.
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

    /// Returns a preview of the picture at \a path as \a theme shows it (themed()), scaled to fit \a size; null if it
    /// does not load.
    static QPixmap preview(const QString &path, Theme theme, const QSize &size);

    /**
     * \brief Returns the picture at \a path as \a theme shows it.
     * \details A built-in picture in a Light folder is shown in the Dark theme as its sibling in the Dark folder beside
     * it, when there is one; any other path, and any path in another theme, is returned as it is.
     */
    static QString themed(const QString &path, Theme theme);

    /// Returns the picture the palette and the menus show for \a type, in the current theme; null for
    /// ElementType::Unknown and for a type with no picture (IC).
    static QPixmap typePicture(ElementType type);

    /// The bytes the variants of every picture may hold: their rasters and vector renderers. The bundled pictures take
    /// 5.0 MiB in the editor's sixteen orientations in both themes (measured, Qt 6.12), which it holds, with room for
    /// the angles MCP can ask for and custom pictures.
    static constexpr qsizetype variantBudget = 16 * 1024 * 1024;

private:
    friend class TestElementPictures;

    ElementPictures() = default;

    /// Returns how many orientations of the picture \a id have a variant kept.
    static qsizetype variantCount(PictureId id);

    /// Returns the bytes the variants of every picture are charged against variantBudget.
    static qsizetype variantBytes();
    /// Returns the bytes the variants of the picture \a id are charged.
    static qsizetype variantBytes(PictureId id);
};
