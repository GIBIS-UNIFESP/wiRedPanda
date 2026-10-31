// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/PictureLibrary.h"

#include <QString>

namespace PictureLibraryDetail {

const PictureLibrary *g_installed = nullptr;

/// What a release build loads with when no implementation is installed: nothing.
class NoPictures final : public PictureLibrary
{
public:
    std::optional<PictureId> load(const QString &path) const override
    {
        Q_UNUSED(path)
        return std::nullopt;
    }
};

} // namespace PictureLibraryDetail

const PictureLibrary &PictureLibrary::installed()
{
    Q_ASSERT_X(PictureLibraryDetail::g_installed, "PictureLibrary::installed()",
               "no PictureLibrary is installed: the program does not call installPresenters()");
    if (!PictureLibraryDetail::g_installed) {
        static const PictureLibraryDetail::NoPictures none;
        return none; // LCOV_EXCL_LINE -- a broken invariant, unreachable in a program that installs the presenters
    }
    return *PictureLibraryDetail::g_installed;
}

void PictureLibrary::install(const PictureLibrary *library)
{
    PictureLibraryDetail::g_installed = library;
}
