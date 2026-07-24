// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Interface for resolving a scene's associated .panda file directory.
 */

#pragma once

#include <QString>

/**
 * \class ContextDirProvider
 * \brief Narrow interface exposing a scene's context directory.
 *
 * \details Lets Scene::deserializationContext() populate SerializationContext::contextDir
 * without the Element layer needing to name the concrete Scene class.
 */
class ContextDirProvider
{
public:
    virtual ~ContextDirProvider() = default; // LCOV_EXCL_LINE -- inlined at every derived-class destruction site, not separately attributed

    /// Returns the directory of the .panda file associated with the scene.
    virtual QString contextDir() const = 0;
};
