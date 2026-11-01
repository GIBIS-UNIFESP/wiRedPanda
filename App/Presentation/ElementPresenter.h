// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief How an element of each type looks: the presenters, one per type, and their installation.
 */

#pragma once

#include <optional>

#include <QPointF>
#include <QRectF>
#include <QString>

#include "App/Core/Enums.h"
#include "App/Element/BodyMeasure.h"
#include "App/Element/ElementTypeDescriptor.h"
#include "App/Element/PictureLibrary.h"

class Element;
class QPainter;
class ThemeAttributes;

/// What a presenter draws a body with, besides the element.
struct BodyContext {
    const ThemeAttributes &theme;
};

/**
 * \brief What an element's body draws, exactly: in one theme, two elements with equal keys draw the same body.
 * \details Each presenter fills in the fields its body depends on and leaves the rest as they are by default. The
 * selection is not part of it, since ElementPainter draws it over no body, nor the position, nor the theme, which what
 * is drawn from a key holds beside it. Elements in one orientation whose bodies draw the same have equal keys; a body
 * that counter-orients its labels or an icon may draw alike in two equivalent orientations under two keys. A struct
 * compared exactly, not a hash, so that no collision hands an element another's drawing, and its coordinates compare
 * exactly, where QRectF's comparison is fuzzy.
 */
struct AppearanceKey {
    ElementType type = ElementType::Unknown; ///< The type whose own drawing the body has; Unknown for a picture alone.
    std::optional<PictureId> picture; ///< The picture shown, named in the Light theme (ElementPictures::themed()).
    quint32 state = 0;                ///< What the type draws over its picture, packed by its presenter.
    QRectF footprint;                 ///< The footprint a FootprintPresenter draws over; null for a picture.
    qreal rotation = 0; ///< The turn the body counter-orients labels or an icon for, in [0, 360); 0 when it has none.
    bool flipX = false; ///< The flips the body counter-orients labels or an icon for; false when it has none.
    bool flipY = false;
    QString text; ///< Text the body draws: an empty Text's hint, in the language shown.

    /// Sets the orientation to \a element's, its turn as ElementPictures::canonicalTurn() gives it, for a body that
    /// counter-orients labels or an icon so that they read upright.
    void setOrientation(const Element &element);

    /// Whether \a a and \a b are equal, every coordinate exactly.
    friend bool operator==(const AppearanceKey &a, const AppearanceKey &b);
};

/// Hashes \a key as operator==() compares it.
size_t qHash(const AppearanceKey &key, size_t seed = 0);

/// Where an element's label goes, in the element's un-rotated local frame, and how it turns about its origin.
struct LabelPlacement {
    QPointF anchor;
    int rotation = 0;
};

/**
 * \class ElementPresenter
 * \brief How the elements of one type look: the shape of their body and what it draws.
 * \details A presenter reads the element and keeps nothing of it, so one serves every element of its type. Each type
 * has one, registered by installPresenters(); presentation measures every element and places its label through them,
 * as the model's BodyMeasure and LabelMeasure, and ElementPainter draws its body with them; installPresenters()
 * installs the three.
 */
class ElementPresenter
{
public:
    virtual ~ElementPresenter() = default; // LCOV_EXCL_LINE -- the presenters live as long as the program

    /// Returns the shape of \a element's body as its state gives it now.
    virtual BodyShape measure(const Element &element) const = 0;

    /// Draws \a element's body with \a painter, in the element's own coordinates, without its selection.
    virtual void paintBody(QPainter &painter, const Element &element, const BodyContext &context) const = 0;

    /// Returns the key of what paintBody() draws of \a element in its state now (AppearanceKey).
    virtual AppearanceKey appearanceKey(const Element &element) const = 0;

    /// Returns the slot of \a element's appearance list whose picture it shows, as its state picks it; the first by
    /// default. It reads the element's parameters only: an element is measured from its base's constructor on.
    virtual int pictureSlot(const Element &element) const;

    /// Returns where \a element's label goes: below a 64x64 body, upright, by default.
    virtual LabelPlacement label(const Element &element) const;

    /// Returns the operation a click at \a point, in \a element's own coordinates, triggers: the press operation
    /// anywhere by default, none for a type without one.
    virtual std::optional<Operation> activationAt(const Element &element, const QPointF &point) const;

    /// Returns the presenter registered for the elements of \a type, or null if there is none.
    static const ElementPresenter *find(ElementType type);

    /// Returns the presenter of the elements of \a type. Every type an element can have has one (installPresenters()):
    /// a debug build stops on a type without, a release build draws it as its picture.
    static const ElementPresenter &of(ElementType type);
};

/// Registers each type's presenter and installs them as the model's BodyMeasure and LabelMeasure, ElementPainter as its
/// BodyPainter, and the pictures as its PictureLibrary. Called once by every program that has elements, as it starts, before any element exists.
void installPresenters();
