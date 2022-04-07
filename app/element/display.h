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
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)

public:
    explicit Display(QGraphicsItem *parent = nullptr);

    static int current_id_number; // Number used to create distinct labels for each instance of this element.

    QString getColor() const override;
    static QPixmap convertColor(const QImage &src, bool red, bool green, bool blue);
    static void convertAllColors(QVector<QPixmap> &maps);
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void refresh() override;
    void save(QDataStream &ds) const override;
    void setColor(const QString &getColor) override;
    void setSkin(bool defaultSkin, const QString &filename) override;
    void updatePorts() override;

private:
    const QString m_titleText = tr("<b>7-SEGMENT DISPLAY</b>");
    const QString m_translatedName = tr("Display");
    const QString m_pixmap = ":/output/counter/counter_on.png";

    QVector<QPixmap> bkg, a, b, c, d, e, f, g, dp;
    QString m_color;
    int m_color_number;
};

Q_DECLARE_METATYPE(Display)
