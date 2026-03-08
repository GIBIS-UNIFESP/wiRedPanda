// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel2PriorityEncoder8To3 : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void test8to3PriorityEncoder_data();
    void test8to3PriorityEncoder();
};
