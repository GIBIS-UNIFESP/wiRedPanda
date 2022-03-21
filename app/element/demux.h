/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef DEMUX_H
#define DEMUX_H

#include "graphicelement.h"

#include <QObject>

class Demux : public GraphicElement
{
public:
    explicit Demux(QGraphicsItem *parent = nullptr);
    ~Demux() override = default;

    void updatePorts() override;
    void setSkin(bool defaultSkin, const QString &filename) override;
};

#endif /* DEMUX_H */
