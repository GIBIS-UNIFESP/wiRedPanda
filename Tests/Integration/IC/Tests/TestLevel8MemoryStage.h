// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel8MemoryStage : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testMemoryStage_data();
    void testMemoryStage();

    void testMemoryStageStructure();
};

