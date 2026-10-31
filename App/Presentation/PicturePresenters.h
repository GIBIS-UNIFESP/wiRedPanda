// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief The presenters of the types whose body is a picture, with what some draw over it.
 */

#pragma once

#include <QVector>

#include "App/Presentation/ElementPresenter.h"

class QRectF;

/// The presenter of most types: the body is the picture the element shows, and turns about its centre.
class PicturePresenter : public ElementPresenter
{
public:
    BodyShape measure(const Element &element) const override;

    /// Draws the picture \a element shows, then what its type draws over it and beyond (paintOverlay()).
    void paintBody(QPainter &painter, const Element &element, const BodyContext &context) const override;

    /// Returns the rectangle the picture \a element shows takes, at the origin, the same in every theme; empty while it
    /// shows none.
    static QRectF pictureRect(const Element &element);

protected:
    /// Draws over \a element's picture, and beyond it, what its type shows of its state, in the element's own
    /// coordinates and \a context's theme; nothing by default.
    virtual void paintOverlay(QPainter &painter, const Element &element, const BodyContext &context) const;
};

/// The presenter of the inputs that show their level (InputButton, InputSwitch, Clock): off, then on.
class LevelPresenter final : public PicturePresenter
{
public:
    int pictureSlot(const Element &element) const override;
};

/// The presenter of the audio outputs (Buzzer, AudioBox): silent, then playing; the label beside the body.
class AudioPresenter final : public PicturePresenter
{
public:
    int pictureSlot(const Element &element) const override;
    LabelPlacement label(const Element &element) const override;
};

/// The presenter of the LED: the slot its colour and inputs lit at its last refresh.
class LedPresenter final : public PicturePresenter
{
public:
    int pictureSlot(const Element &element) const override;
};

/// The presenter of InputRotary: a mark for every position around the dial, and the arrow at the selected one.
class RotaryPresenter final : public PicturePresenter
{
protected:
    void paintOverlay(QPainter &painter, const Element &element, const BodyContext &context) const override;
};

/**
 * \brief The presenter of Node: its picture and, while it is wireless, the indicator over the port the channel
 * replaces, in the colour of the level it carries (ParamId::Carried).
 * \details The indicator is the body's decoration (BodyShape::decoration): it is neither selected, hit nor aligned.
 */
class NodePresenter final : public PicturePresenter
{
public:
    BodyShape measure(const Element &element) const override;

    /// Below the node's small body.
    LabelPlacement label(const Element &element) const override;

protected:
    void paintOverlay(QPainter &painter, const Element &element, const BodyContext &context) const override;
};

/**
 * \brief The presenter of the segment displays (Display7, Display14, Display16): the lit segments over the picture, in
 * the display's colour.
 * \details The body is the picture and the 64x64 box the segments are drawn in, which a custom picture need not cover;
 * it turns about the picture's centre.
 */
class SegmentDisplayPresenter final : public PicturePresenter
{
public:
    /// \a segments holds, for each input in order, the slot of the default appearance list of the segment it lights.
    explicit SegmentDisplayPresenter(QVector<int> segments);

    BodyShape measure(const Element &element) const override;

protected:
    void paintOverlay(QPainter &painter, const Element &element, const BodyContext &context) const override;

private:
    QVector<int> m_segments;
};

/**
 * \brief The presenter of Text: its picture and, while its label is empty, a faint hint to add text in its place.
 * \details The body holds the picture, the label's box and the hint's place, whether the hint shows or not, so a click
 * on the text hits the element; it turns about the picture's centre.
 */
class TextPresenter final : public PicturePresenter
{
public:
    BodyShape measure(const Element &element) const override;

protected:
    void paintOverlay(QPainter &painter, const Element &element, const BodyContext &context) const override;
};
