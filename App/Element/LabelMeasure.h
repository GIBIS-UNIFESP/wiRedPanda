// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Where an element's label goes and the room its text takes, and the interface through which an element has
 * them measured.
 */

#pragma once

#include <QFont>
#include <QPointF>
#include <QSizeF>

class Element;

/// Where an element's label goes and the room its text takes.
struct LabelShape {
    /// The point the label's origin is kept at, in the element's un-rotated local frame: a rotatable element's label is
    /// counter-oriented about it, so it stays upright where the anchor says.
    QPointF anchor;
    /// How many degrees the label turns about its origin before it is placed: 0, or 90 for an IC, whose name stands
    /// along the side of the chip.
    int rotation = 0;
    /// The box the label's text takes in the label's own coordinates, whose top left corner is its origin; empty while
    /// it shows no text.
    QSizeF box;

    friend bool operator==(const LabelShape &, const LabelShape &) = default;
};

/**
 * \class LabelMeasure
 * \brief Measures an element's label, and gives the font it is drawn in.
 * \details Declared by the model, whose elements keep their label's shape, and implemented by presentation, which
 * places the label of each type and lays its text out: the model never names presentation. Presentation installs its
 * implementation as the program starts (installPresenters()), before any element exists, in every program that has
 * elements.
 */
class LabelMeasure
{
public:
    virtual ~LabelMeasure() = default; // LCOV_EXCL_LINE -- the installed implementation lives as long as the program

    /// Returns the shape of \a element's label as its state gives it now.
    virtual LabelShape measure(const Element &element) const = 0;

    /// Returns the font a label is drawn in.
    virtual const QFont &font() const = 0;

    /// Returns the installed implementation. A program without one is built wrong: a debug build stops here, a release
    /// build measures every label empty at the origin.
    static const LabelMeasure &installed();

    /// Installs \a measure, which outlives every element.
    static void install(const LabelMeasure *measure);
};
