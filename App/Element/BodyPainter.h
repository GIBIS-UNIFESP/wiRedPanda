// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief The interface through which an element, while it is a graphics item, has itself drawn.
 */

#pragma once

class Element;
class QPainter;

/**
 * \class BodyPainter
 * \brief Draws an element as the scene shows it: its selection, then its body.
 * \details Declared by the model, whose elements are drawn by the graphics view as items, and implemented by
 * presentation, which knows how each type looks: the model never names presentation. Presentation installs its
 * implementation as the program starts (installPresenters()), before any element exists, in every program that has
 * elements.
 */
class BodyPainter
{
public:
    virtual ~BodyPainter() = default; // LCOV_EXCL_LINE -- the installed implementation lives as long as the program

    /// Draws \a element with \a painter, in the element's own coordinates.
    virtual void paint(QPainter &painter, const Element &element) const = 0;

    /// Returns the installed implementation. A program without one is built wrong: a debug build stops here, a release
    /// build draws nothing.
    static const BodyPainter &installed();

    /// Installs \a painter, which outlives every element.
    static void install(const BodyPainter *painter);
};
