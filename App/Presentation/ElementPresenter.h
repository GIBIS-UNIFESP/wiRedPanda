// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief How an element of each type looks: the presenters, one per type, and their installation.
 */

#pragma once

#include "App/Core/Enums.h"
#include "App/Element/BodyMeasure.h"

class Element;
class QPainter;
class ThemeAttributes;

/// What a presenter draws a body with, besides the element.
struct BodyContext {
    const ThemeAttributes &theme;
};

/**
 * \class ElementPresenter
 * \brief How the elements of one type look: the shape of their body and what it draws.
 * \details A presenter reads the element and keeps nothing of it, so one serves every element of its type. Each type
 * has one, registered by installPresenters(); presentation measures and draws every element through them, as the
 * model's BodyMeasure and BodyPainter, which installPresenters() installs.
 */
class ElementPresenter
{
public:
    virtual ~ElementPresenter() = default; // LCOV_EXCL_LINE -- the presenters live as long as the program

    /// Returns the shape of \a element's body as its state gives it now.
    virtual BodyShape measure(const Element &element) const = 0;

    /// Draws \a element's body with \a painter, in the element's own coordinates, without its selection.
    virtual void paintBody(QPainter &painter, const Element &element, const BodyContext &context) const = 0;

    /// Returns the presenter registered for the elements of \a type, or null if there is none.
    static const ElementPresenter *find(ElementType type);

    /// Returns the presenter of the elements of \a type. Every type an element can have has one (installPresenters()):
    /// a debug build stops on a type without, a release build draws it as its picture.
    static const ElementPresenter &of(ElementType type);
};

/// Registers each type's presenter and installs them as the model's BodyMeasure and BodyPainter, and the pictures as its
/// PictureLibrary. Called once by every program that has elements, as it starts, before any element exists.
void installPresenters();
