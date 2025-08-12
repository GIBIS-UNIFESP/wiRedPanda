// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestQNEConnectionSafe : public QObject
{
    Q_OBJECT

private slots:
    void testBasicConstruction();
    void testPositionsAndPath();
    void testStatusManagement();
    void testVisualProperties();
    void testSerializationBasics();
    void testGraphicsItemProperties();
};