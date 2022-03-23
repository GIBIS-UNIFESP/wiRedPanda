/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef NODE_H
#define NODE_H

#include "graphicelement.h"

class Node : public GraphicElement
{
public:
    explicit Node(QGraphicsItem *parent = nullptr);
    ~Node() override = default;

    void updatePorts() override;
    void setSkin(bool defaultSkin, const QString &filename) override;
};

#endif /* NODE_H */
