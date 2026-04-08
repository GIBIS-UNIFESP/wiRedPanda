// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestDisplay7 : public QObject
{
    Q_OBJECT

private slots:

    void testDisplay7ColorSetting();
    void testDisplay7SegmentRendering();
    void testDisplay7SaveLoad();
    void testDisplay7AllColors();
    void testDisplay7Paint();
};

