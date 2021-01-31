/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ITEMWITHID_H
#define ITEMWITHID_H

class ItemWithId
{
    int m_id;

public:
    ItemWithId();
    int id() const;
    void setId(int id);
    virtual ~ItemWithId();
};

#endif /* ITEMWITHID_H */