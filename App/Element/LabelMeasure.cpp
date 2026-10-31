// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/LabelMeasure.h"

#include <QtGlobal>

namespace LabelMeasureDetail {

const LabelMeasure *g_installed = nullptr;

/// What a release build measures with when no implementation is installed: every label empty at the origin.
class EmptyMeasure final : public LabelMeasure
{
public:
    LabelShape measure(const Element &element) const override
    {
        Q_UNUSED(element)
        return {};
    }

    const QFont &font() const override
    {
        static const QFont font;
        return font;
    }
};

} // namespace LabelMeasureDetail

const LabelMeasure &LabelMeasure::installed()
{
    Q_ASSERT_X(LabelMeasureDetail::g_installed, "LabelMeasure::installed()",
               "no LabelMeasure is installed: the program does not call installPresenters()");
    if (!LabelMeasureDetail::g_installed) {
        static const LabelMeasureDetail::EmptyMeasure empty;
        return empty; // LCOV_EXCL_LINE -- a broken invariant, unreachable in a program that installs the presenters
    }
    return *LabelMeasureDetail::g_installed;
}

void LabelMeasure::install(const LabelMeasure *measure)
{
    LabelMeasureDetail::g_installed = measure;
}
