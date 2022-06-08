/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

class GraphicElementInput : public GraphicElement
{
    Q_OBJECT

public:
    explicit GraphicElementInput(ElementType type, ElementGroup group, const int minInputSize, const int maxInputSize, const int minOutputSize, const int maxOutputSize, QGraphicsItem *parent = nullptr)
        : GraphicElement(type, group, minInputSize, maxInputSize, minOutputSize, maxOutputSize, parent){};

    bool isLocked() const { return m_locked; }
    virtual bool on(const int port = 0) const = 0;
    virtual int outputSize() const { return 1; }
    virtual int outputValue() const { return on(); }
    virtual void setOff() = 0;
    virtual void setOn() = 0;
    virtual void setOn(const bool value, const int port = 0) = 0;
    void setLocked(const bool value) { m_locked = value; }

protected:
    bool m_locked;
};
