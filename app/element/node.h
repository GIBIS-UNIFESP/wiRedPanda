/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class Node : public GraphicElement
{
    Q_OBJECT
    Q_PROPERTY(QString pixmap MEMBER m_pixmap CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

public:
    explicit Node(QGraphicsItem *parent = nullptr);

    void updatePorts() override;

private:
    const QString m_pixmap = ":/basic/node.png";
    const QString m_titleText = tr("<b>NODE</b>");
    const QString m_translatedName = tr("Node");
};

Q_DECLARE_METATYPE(Node)
