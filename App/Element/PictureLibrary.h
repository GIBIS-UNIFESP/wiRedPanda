// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief The interface through which an element loads the picture it shows.
 */

#pragma once

#include <optional>

#include <QtGlobal>

class QString;

/// Identifies one version of one picture file, as the installed PictureLibrary loaded it.
enum class PictureId : quint32 {};

/**
 * \class PictureLibrary
 * \brief Loads the pictures elements show.
 * \details Declared by the model, whose elements choose which picture they show, and implemented by presentation,
 * which loads, orients, sizes and draws them: the model keeps only the id it is given. Presentation installs its
 * implementation as the program starts (installPresenters()), before any element exists, in every program that has
 * elements.
 */
class PictureLibrary
{
public:
    virtual ~PictureLibrary() = default; // LCOV_EXCL_LINE -- the installed implementation lives as long as the program

    /// Loads the picture at \a path. \returns its id, the same for every element that shows this version of the file,
    /// or nothing if it does not load.
    virtual std::optional<PictureId> load(const QString &path) const = 0;

    /// Returns the installed implementation. A program without one is built wrong: a debug build stops here, a release
    /// build loads nothing.
    static const PictureLibrary &installed();

    /// Installs \a library, which outlives every element.
    static void install(const PictureLibrary *library);
};
