// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "physicalconnection.h"

PhysicalConnection::PhysicalConnection(QGraphicsItem *parent)
    : QNEConnection(parent)
{
    // All behavior inherited from QNEConnection base class
    // - Normal visible rendering via QNEConnection::paint()
    // - Normal selection via QNEConnection::shape()  
    // - Normal user interaction via QNEConnection::sceneEvent()
}