// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

class TruthTable : public GraphicElement
{
    Q_OBJECT

public:
    explicit TruthTable(QGraphicsItem *parent = nullptr);

    void updatePortsProperties() override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void update();

private:
    void generatePixmap();

};

Q_DECLARE_METATYPE(TruthTable)
