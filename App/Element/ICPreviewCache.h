// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ICPreviewCache: the hover previews of IC definitions, one per definition and directory.
 */

#pragma once

#include <cstddef>
#include <memory>

#include <QPixmap>
#include <QString>

class ICDefinition;

/**
 * \class ICPreviewCache
 * \brief The hover previews of IC definitions, rendered once and kept for every instance of a definition.
 *
 * \details An instance asks for its preview through preview(), with the definition it holds and the directory that
 * definition's references resolve against. A preview is found again while the definition lives: a definition is
 * found through a weak reference, so a reload, which replaces it, leaves its preview behind to go. The most recently
 * used kCapacity previews are kept; the least recently used goes first. A null preview is kept too, so a circuit that
 * does not render is not built again. The cache is used on the main thread only.
 *
 * A theme change drops every preview, since wires, ports and labels are drawn in the theme's colours.
 */
class ICPreviewCache
{
public:
    /// The most previews kept at once.
    static constexpr std::size_t kCapacity = 16;

    /// Returns the preview of \a definition read from \a contextDir, rendered on the first ask and kept after.
    static QPixmap preview(const std::shared_ptr<const ICDefinition> &definition, const QString &contextDir);

    /// Drops every preview kept.
    static void clear();

    /// Returns how many previews have been rendered so far (a test seam).
    static int renderCount();

    /// Returns how many previews are kept (a test seam).
    static int entryCount();
};
