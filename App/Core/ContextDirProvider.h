// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ContextDirProvider: abstract source of a circuit's context directory.
 */

#pragma once

#include <QString>

/**
 * \class ContextDirProvider
 * \brief Interface for objects that can supply the directory of the .panda file
 * the circuit they host was loaded from.
 *
 * \details Implemented by Scene. It lets lower layers (Element) resolve an item's
 * context directory via a dynamic_cast on QGraphicsItem::scene() without taking a
 * dependency on the concrete Scene type. See GraphicElement::resolveContextDir().
 */
class ContextDirProvider
{
public:
    virtual ~ContextDirProvider() = default;

    /// Returns the directory of the .panda file associated with this context.
    virtual QString contextDir() const = 0;
};
