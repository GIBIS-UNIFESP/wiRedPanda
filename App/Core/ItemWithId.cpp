// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/ItemWithId.h"

#include "App/Scene/SceneItemRegistry.h"

// Virtual destructor entry/exit is exercised constantly (every GraphicElement/
// Connection destruction across the suite) but never separately attributed by
// gcov — the same inlined-special-member-function false-miss as `= default`
// destructors elsewhere in this sweep, just with a real body here.
ItemWithId::~ItemWithId() // LCOV_EXCL_LINE
{
    if (m_registry) {
        m_registry->forget(this);
    }
} // LCOV_EXCL_LINE
