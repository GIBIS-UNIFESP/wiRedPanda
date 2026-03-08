// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class IC;

class TestComponents : public QObject
{
    Q_OBJECT

private slots:
    void testNode();
    void testVCC();
    void testGND();
    void testIC();
    void testICs();

private:
    void testICData(IC *ic);
};

