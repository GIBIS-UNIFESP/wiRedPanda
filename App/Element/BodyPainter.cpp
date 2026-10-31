// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/BodyPainter.h"

#include <QtGlobal>

namespace BodyPainterDetail {

const BodyPainter *g_installed = nullptr;

/// What a release build draws with when no implementation is installed: nothing.
class NoPainter final : public BodyPainter
{
public:
    void paint(QPainter &painter, const Element &element) const override
    {
        Q_UNUSED(painter)
        Q_UNUSED(element)
    }
};

} // namespace BodyPainterDetail

const BodyPainter &BodyPainter::installed()
{
    Q_ASSERT_X(BodyPainterDetail::g_installed, "BodyPainter::installed()",
               "no BodyPainter is installed: the program does not call installPresenters()");
    if (!BodyPainterDetail::g_installed) {
        static const BodyPainterDetail::NoPainter none;
        return none; // LCOV_EXCL_LINE -- a broken invariant, unreachable in a program that installs the presenters
    }
    return *BodyPainterDetail::g_installed;
}

void BodyPainter::install(const BodyPainter *painter)
{
    BodyPainterDetail::g_installed = painter;
}
