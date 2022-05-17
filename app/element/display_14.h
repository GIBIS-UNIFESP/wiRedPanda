/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class Display14 : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

public:
    explicit Display14(QGraphicsItem *parent = nullptr);

    QString color() const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void refresh() override;
    void save(QDataStream &stream) const override;
    void setColor(const QString &color) override;
    void setSkin(const bool defaultSkin, const QString &fileName) override;
    void updatePorts() override;

private:
    const QString m_pixmap = ":/output/counter/counter_14_on.png";
    const QString m_titleText = tr("<b>14-SEGMENT DISPLAY</b>");
    const QString m_translatedName = tr("Display14");

    QString m_color;
    QVector<QPixmap> bkg, a, b, c, d, e, f, g1, g2, h, j, k, l, m, n, dp;
    int m_colorNumber;
};

Q_DECLARE_METATYPE(Display14)
