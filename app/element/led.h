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

    QString color() const override;
    QString genericProperties() override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version) override;
    void refresh() override;
    void save(QDataStream &stream) const override;
    void setColor(const QString &color) override;
    void setSkin(const bool defaultSkin, const QString &fileName) override;
    void updatePorts() override;

private:
    QString m_color = "White";
    int m_colorNumber = 0; /* white = 0, red = 2, green = 4, blue = 6, purple = 8 */
};

Q_DECLARE_METATYPE(Led)

// FIXME: connecting more than one source makes led stop working
