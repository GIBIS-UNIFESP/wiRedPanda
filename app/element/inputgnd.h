/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INPUTGND_H
#define INPUTGND_H

#include "graphicelement.h"

class InputGnd : public GraphicElement
{
public:
    explicit InputGnd(QGraphicsItem *parent = nullptr);
    ~InputGnd() override = default;
    void setSkin(bool defaultSkin, const QString &filename) override;
};

#endif /* INPUTGND_H */
