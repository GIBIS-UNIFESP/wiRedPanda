// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/BodyMeasure.h"

#include <QtGlobal>

namespace BodyMeasureDetail {

const BodyMeasure *g_installed = nullptr;

/// What a release build measures with when no implementation is installed: every body empty.
class EmptyMeasure final : public BodyMeasure
{
public:
    BodyShape measure(const Element &element) const override
    {
        Q_UNUSED(element)
        return {};
    }
};

} // namespace BodyMeasureDetail

const BodyMeasure &BodyMeasure::installed()
{
    Q_ASSERT_X(BodyMeasureDetail::g_installed, "BodyMeasure::installed()",
               "no BodyMeasure is installed: the program does not call installPresenters()");
    if (!BodyMeasureDetail::g_installed) {
        static const BodyMeasureDetail::EmptyMeasure empty;
        return empty; // LCOV_EXCL_LINE -- a broken invariant, unreachable in a program that installs the presenters
    }
    return *BodyMeasureDetail::g_installed;
}

void BodyMeasure::install(const BodyMeasure *measure)
{
    BodyMeasureDetail::g_installed = measure;
}
