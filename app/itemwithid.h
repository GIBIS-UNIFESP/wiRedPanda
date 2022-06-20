/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

class ItemWithId
{
public:
    ItemWithId();
    virtual ~ItemWithId();

    int id() const;
    void setId(const int id);

private:
    int m_id = 0;
};
