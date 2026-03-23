// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/ItemWithId.h"

#include "App/Element/ElementFactory.h"

ItemWithId::ItemWithId()
{
    // Self-register with ElementFactory so every item gets a unique integer ID
    // and can be looked up globally by that ID (e.g. after undo/redo commands).
    ElementFactory::addItem(this);
}

ItemWithId::~ItemWithId()
{
    // Deregister on destruction to prevent dangling ID→pointer entries in the
    // factory map; safe to call even during exception unwinding.
    ElementFactory::removeItem(this);
}

int ItemWithId::id() const
{
    return m_id;
}

void ItemWithId::setId(const int id)
{
    m_id = id;
}

