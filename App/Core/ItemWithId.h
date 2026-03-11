// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtGlobal>

class ItemWithId
{
public:
    // --- Lifecycle ---

    ItemWithId();
    virtual ~ItemWithId();

    // --- Identity ---

    int id() const;
    void setId(const int id);

private:
    Q_DISABLE_COPY(ItemWithId)

    int m_id = 0;
};
