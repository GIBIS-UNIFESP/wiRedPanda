/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class Led : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)

public:
    explicit Led(QGraphicsItem *parent = nullptr);

    static int current_id_number; // Number used to create distinct labels for each instance of this element.

    QString genericProperties() override;
    QString getColor() const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    void refresh() override;
    void save(QDataStream &ds) const override;
    void setColor(const QString &getColor) override;
    void setSkin(bool defaultSkin, const QString &filename) override;
    void updatePorts() override;

private:
    void resetLedPixmapName(int ledNumber);

    const QString m_titleText = tr("<b>LED</b>");
    const QString m_translatedName = tr("Led");
    const QString m_pixmap = ":/output/WhiteLedOff.png";

    QString m_color;
    int m_colorNumber; /* white = 0, red = 2, green = 4, blue = 6, purple = 8 */
};

Q_DECLARE_METATYPE(Led)

