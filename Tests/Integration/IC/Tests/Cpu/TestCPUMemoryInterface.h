// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestCPUMemoryInterface : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testMemoryInterface();
    void testMemoryInterfaceBoundary();
    void testMemoryInterfaceBoundary_data();
    void testMemoryInterfaceControl();
    void testMemoryInterfaceControl_data();
    void testMemoryInterfacePatterns();
    void testMemoryInterfacePatterns_data();
    void testMemoryInterfaceWrite();
    void testMemoryInterfaceWrite_data();
    void testMemoryInterface_data();
};

