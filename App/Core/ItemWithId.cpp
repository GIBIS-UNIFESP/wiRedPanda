// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/ItemWithId.h"

#include "App/Scene/SceneItemRegistry.h"

ItemWithId::~ItemWithId()
{
    if (m_registry) {
        m_registry->forget(this);
    }
}
