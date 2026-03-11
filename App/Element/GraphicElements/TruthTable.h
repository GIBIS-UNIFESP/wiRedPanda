// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "App/Element/GraphicElement.h"

class TruthTable : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit TruthTable(QGraphicsItem *parent = nullptr);

    // --- Key Access ---

    QBitArray &key();
    void setkey(const QBitArray &key);

    // --- Visual ---

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void update();
    void updatePortsProperties() override;

    // --- Serialization ---

    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void save(QDataStream &stream) const override;

private:
    // --- Internal methods ---

    void generatePixmap();

    QBitArray m_key;
};

Q_DECLARE_METATYPE(TruthTable)
