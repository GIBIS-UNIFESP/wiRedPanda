/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef OR_H
#define OR_H

#include "graphicelement.h"

class Or : public GraphicElement
{
public:
    explicit Or(QGraphicsItem *parent = nullptr);
    ~Or() override = default;

    void setSkin(bool defaultSkin, const QString &filename) override;
};

#endif /* OR_H */
