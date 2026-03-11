// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "App/Element/GraphicElement.h"

class Led : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit Led(QGraphicsItem *parent = nullptr);

    // --- Color State ---

    QString color() const override;
    QString genericProperties() override;
    void setColor(const QString &color) override;

    // --- Visual ---

    void refresh() override;
    void setSkin(const bool useDefaultSkin, const QString &fileName) override;
    void updatePortsProperties() override;

    // --- Serialization ---

    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void save(QDataStream &stream) const override;

private:
    // --- Internal methods ---

    int colorIndex();

    // --- Members ---

    QString m_color = "White";
    int m_colorIndex = 0; /* white = 0, red = 2, green = 4, blue = 6, purple = 8 */
};

Q_DECLARE_METATYPE(Led)

