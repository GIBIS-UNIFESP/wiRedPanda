// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtGlobal>

class ItemWithId
{
public:
    ItemWithId();
    virtual ~ItemWithId();

    int id() const;
    void setId(const int id);

private:
    Q_DISABLE_COPY(ItemWithId)

    int m_id = 0;
};
