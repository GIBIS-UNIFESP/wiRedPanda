/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INPUTVCC_H
#define INPUTVCC_H

#include "graphicelement.h"

class InputVcc : public GraphicElement
{
public:
    explicit InputVcc(QGraphicsItem *parent = nullptr);
    ~InputVcc() override = default;
    void setSkin(bool defaultSkin, const QString &filename) override;
};

#endif /* INPUTVCC_H */
