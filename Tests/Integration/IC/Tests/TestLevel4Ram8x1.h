// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel4RAM8X1 : public QObject
{
    Q_OBJECT

private slots:
    void testRamArray_data();
    void testRamArray();
    void testRamBasic_data();
    void testRamBasic();
    void testRamEdgeCases_data();
    void testRamEdgeCases();
};
