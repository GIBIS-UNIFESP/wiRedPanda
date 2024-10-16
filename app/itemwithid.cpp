
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemwithid.h"
#include "elementfactory.h"

ItemWithId::ItemWithId()
{
    ElementFactory::addItem(this);
}

ItemWithId::~ItemWithId()
{
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
