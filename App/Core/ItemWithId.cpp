// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/ItemWithId.h"

#include "App/Scene/SceneItemRegistry.h"

// Virtual destructor entry/exit is exercised constantly (every GraphicElement/
// Connection destruction across the suite) but never separately attributed by
// gcov — same ABI-variant gap as App/Wiring/Port.h's destructor: no code in
// the repo ever `delete`s through a bare ItemWithId*, so this class's own D0
// (deleting destructor) never runs — every real deletion goes through the
// concrete GraphicElement/Connection type's own D0, which invokes this D2
// (base-object destructor) as part of that unwind instead.
ItemWithId::~ItemWithId() // LCOV_EXCL_LINE
{
    if (m_registry) {
        m_registry->forget(this);
    }
} // LCOV_EXCL_LINE
