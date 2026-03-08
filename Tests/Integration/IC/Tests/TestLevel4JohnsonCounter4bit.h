// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel4JohnsonCounter4Bit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testJohnsonCounter_data();
    void testJohnsonCounter();
};

