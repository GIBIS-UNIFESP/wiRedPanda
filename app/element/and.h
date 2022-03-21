/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef AND_H
#define AND_H

#include "graphicelement.h"

class And : public GraphicElement
{
public:
    explicit And(QGraphicsItem *parent = nullptr);
    ~And() override = default;

    void setSkin(bool defaultSkin, const QString &filename) override;
};

#endif /* AND_H */
