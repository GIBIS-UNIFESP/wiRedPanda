// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelement.h"

class GraphicElementInput : public GraphicElement
{
    Q_OBJECT

public:
    explicit GraphicElementInput(ElementType type, ElementGroup group, const QString &pixmapPath, const QString &titleText, const QString &translatedName,
                                 const int minInputSize, const int maxInputSize, const int minOutputSize, const int maxOutputSize, QGraphicsItem *parent = nullptr)
        : GraphicElement(type, group, pixmapPath, titleText, translatedName, minInputSize, maxInputSize, minOutputSize, maxOutputSize, parent) {};

    bool isLocked() const { return m_locked; }
    virtual bool isOn(const int port = 0) const = 0;
    virtual int outputSize() const { return 1; }
    virtual int outputValue() const { return static_cast<int>(isOn()); }
    virtual void setOff() = 0;
    virtual void setOn() = 0;
    virtual void setOn(const bool value, const int port = 0) = 0;
    void setLocked(const bool locked) { m_locked = locked; }
    void updateOutputs();

protected:
    bool m_locked = false;
};

inline void GraphicElementInput::updateOutputs()
{
    for (int portIndex = 0; portIndex < outputSize(); ++portIndex) {
        logic()->setOutputValue(portIndex, isOn(portIndex));
    }
}
