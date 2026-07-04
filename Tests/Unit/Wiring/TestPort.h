// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestPort : public QObject
{
    Q_OBJECT

private slots:

    void testPortTooltip();
    void testPortHoverState();

    // Regression: F18 — setInputs must keep index() == vector position
    void testSetInputsReindexesPorts();
};
