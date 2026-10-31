// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ElementPainter: draws an element, its selection then its body, in the theme it is handed.
 */

#pragma once

class BodyPainter;
class Element;
class QPainter;
class ThemeAttributes;

/**
 * \namespace ElementPainter
 * \brief Draws an element: its selection while it is selected, the same for every type, then its body, drawn by its
 * type's presenter (ElementPresenter::paintBody()).
 *
 * \details The selection is drawn under the body, so the body is drawn the same whether or not the element is
 * selected. Everything is drawn in the theme handed. While elements are graphics items, an element draws itself
 * through the model's BodyPainter, which installPresenters() installs from here, in the current theme.
 */
namespace ElementPainter {

/// Draws \a element with \a painter, in the element's own coordinates: a rounded rectangle over its outline in
/// \a theme's selection colours while it is selected, then its body.
void paint(QPainter &painter, const Element &element, const ThemeAttributes &theme);

/// Returns the model's BodyPainter: paint() in the current theme.
const BodyPainter &inCurrentTheme();

} // namespace ElementPainter
