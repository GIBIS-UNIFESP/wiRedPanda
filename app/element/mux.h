/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef MUX_H
#define MUX_H

#include "graphicelement.h"

#include <QObject>

class Mux : public GraphicElement
{
public:
    explicit Mux(QGraphicsItem *parent = nullptr);
    ~Mux() override = default;

    /* GraphicElement interface */
public:
    void updatePorts() override;
    void setSkin(bool defaultSkin, const QString &filename) override;
};

#endif /* MUX_H */
