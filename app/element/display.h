/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class QNEPort;

class Display : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString pixmapPath MEMBER m_pixmapPath CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

public:
    explicit Display(QGraphicsItem *parent = nullptr);

    static QPixmap convertColor(const QImage &src, const bool red, const bool green, const bool blue);
    static void convertAllColors(QVector<QPixmap> &maps);

    QString color() const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void refresh() override;
    void save(QDataStream &stream) const override;
    void setColor(const QString &color) override;
    void updatePorts() override;

private:
    const QString m_pixmapPath = ":/output/counter/counter_on.svg";
    const QString m_titleText = tr("<b>7-SEGMENT DISPLAY</b>");
    const QString m_translatedName = tr("Display");

    QString m_color = "Red";
    QVector<QPixmap> bkg, a, b, c, d, e, f, g, dp;
    int m_colorNumber = 1;
};

Q_DECLARE_METATYPE(Display)
