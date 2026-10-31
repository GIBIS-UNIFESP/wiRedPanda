// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief The presenters of the types whose body is drawn as vectors to fit their ports.
 */

#pragma once

#include "App/Presentation/ElementPresenter.h"

class QRectF;

/**
 * \class FootprintPresenter
 * \brief The presenter of a type whose body is drawn as vectors to fit its ports (Mux, Demux, TruthTable, IC).
 * \details The body is the footprint, the nominal 64x64 square grown to wherever the ports land
 * (Element::renderBodyBounds()), and turns about its centre. The element reports a replaced picture after laying its
 * ports out, so the body and the pivot follow them.
 */
class FootprintPresenter : public ElementPresenter
{
public:
    BodyShape measure(const Element &element) const override;

    /// Draws \a element's footprint (paintFootprint()) with the painter's origin at the footprint's top-left.
    void paintBody(QPainter &painter, const Element &element, const BodyContext &context) const override;

protected:
    /// Draws \a element's body in \a footprint, which lies at the origin, with antialiasing on.
    virtual void paintFootprint(QPainter &painter, const Element &element, const QRectF &footprint) const = 0;
};

/// The presenter of Mux: a trapezoid wider on the input side.
class MuxPresenter final : public FootprintPresenter
{
protected:
    void paintFootprint(QPainter &painter, const Element &element, const QRectF &footprint) const override;
};

/// The presenter of Demux: a trapezoid wider on the output side.
class DemuxPresenter final : public FootprintPresenter
{
protected:
    void paintFootprint(QPainter &painter, const Element &element, const QRectF &footprint) const override;
};

/// The presenter of TruthTable: a DIP-like grey body with the table's icon, upright at any orientation, and a shadow.
class TruthTablePresenter final : public FootprintPresenter
{
protected:
    void paintFootprint(QPainter &painter, const Element &element, const QRectF &footprint) const override;
};

/// The presenter of IC: a DIP package, tinted while its definition is embedded, with the mascot upright at any
/// orientation, a shadow and the pin-1 notch. An IC whose definition has not loaded has the bare 64x64 square for its
/// footprint.
class ICPresenter final : public FootprintPresenter
{
protected:
    void paintFootprint(QPainter &painter, const Element &element, const QRectF &footprint) const override;
};
