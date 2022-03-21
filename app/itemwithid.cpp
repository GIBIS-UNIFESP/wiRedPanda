// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemwithid.h"
#include "elementfactory.h"

ItemWithId::ItemWithId()
{
    ElementFactory::addItem(this);
}

int ItemWithId::id() const
{
    return m_id;
}

void ItemWithId::setId(int id)
{
    m_id = id;
}

ItemWithId::~ItemWithId()
{
    ElementFactory::removeItem(this);
}