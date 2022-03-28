/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class Led : public GraphicElement
{
    Q_OBJECT

public:
    explicit Led(QGraphicsItem *parent = nullptr);
    ~Led() override = default;
    static int current_id_number; // Number used to create distinct labels for each instance of this element.

    void refresh() override;
    void setColor(const QString &getColor) override;
    QString getColor() const override;

    void save(QDataStream &ds) const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    QString genericProperties() override;

    void updatePorts() override;
    void setSkin(bool defaultSkin, const QString &filename) override;

private:
    QString m_color;
    int m_colorNumber; /* white = 0, red = 2, green = 4, blue = 6, purple = 8 */

    void resetLedPixmapName(int ledNumber);
};

