/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LED_H
#define LED_H

#include "graphicelement.h"

class Led : public GraphicElement
{
public:
    explicit Led(QGraphicsItem *parent = nullptr);
    ~Led() override = default;
    static int current_id_number; // Number used to create distinct labels for each instance of this element.

    void refresh() override;
    void setColor(const QString &getColor) override;
    QString getColor() const override;

private:
    QString m_color;
    int m_colorNumber; /* white = 0, red = 2, green = 4, blue = 6, purple = 8 */

    /* GraphicElement interface */
    void resetLedPixmapName(int ledNumber);

public:
    void save(QDataStream &ds) const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    QString genericProperties() override;

    // GraphicElement interface
public:
    void updatePorts() override;
    void setSkin(bool defaultSkin, const QString &filename) override;
};

#endif /* LED_H */
