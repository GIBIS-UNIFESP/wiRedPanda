// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief The body an element takes, and the interface through which an element has it measured.
 */

#pragma once

#include <QPointF>
#include <QRectF>

class Element;

/// The rectangle an element's body takes in the element's own coordinates, the centre the element turns about, and
/// what is drawn beyond the body.
struct BodyShape {
    QRectF body;
    QPointF centre;

    /// What the body draws beyond itself, which is not part of the element: it is neither selected, hit nor aligned (a
    /// wireless node's indicator). Null for most types.
    QRectF decoration = {};

    friend bool operator==(const BodyShape &, const BodyShape &) = default;
};

/**
 * \class BodyMeasure
 * \brief Measures an element's body.
 * \details Declared by the model, whose elements keep their body's shape, and implemented by presentation, which knows
 * how each type looks: the model never names presentation. Presentation installs its implementation as the program
 * starts (installPresenters()), before any element exists, in every program that has elements.
 */
class BodyMeasure
{
public:
    virtual ~BodyMeasure() = default; // LCOV_EXCL_LINE -- the installed implementation lives as long as the program

    /// Returns the shape of \a element's body as its state gives it now.
    virtual BodyShape measure(const Element &element) const = 0;

    /// Returns the installed implementation. A program without one is built wrong: a debug build stops here, a release
    /// build measures every body empty.
    static const BodyMeasure &installed();

    /// Installs \a measure, which outlives every element.
    static void install(const BodyMeasure *measure);
};
