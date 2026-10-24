// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/ItemWithId.h"

#include "App/Scene/SceneItemRegistry.h"

// Virtual destructor entry/exit is exercised constantly (every GraphicElement/
// Connection destruction across the suite) but never separately attributed by
// gcov — same ABI-variant gap as App/Wiring/Port.h's destructor: no object whose
// dynamic type is ItemWithId itself ever exists (every item is a GraphicElement
// or a Connection), so this class's own D0 (deleting destructor) never runs. A
// delete, even through an ItemWithId*, dispatches virtually to the concrete
// type's D0, which invokes this D2 (base-object destructor) as part of that
// unwind instead.
ItemWithId::~ItemWithId() // LCOV_EXCL_LINE
{
    if (m_registry) {
        m_registry->forget(this);
    }
} // LCOV_EXCL_LINE
