// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/ItemWithId.h"

ItemWithId::ItemWithId()
{
}

ItemWithId::~ItemWithId()
{
}

int ItemWithId::id() const
{
    return m_id;
}

void ItemWithId::setId(const int id)
{
    m_id = id;
}
