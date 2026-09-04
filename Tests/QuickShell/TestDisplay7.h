// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Mirrors Tests/Unit/Elements/TestDisplay7.h/.cpp. testDisplay7Paint exercises
/// GraphicElement::paint(QPainter*) directly against a plain CanvasItem -- this is a paint
/// test, not a save/load one, so no QuickWorkSpace is needed.
class TestDisplay7 : public QObject
{
    Q_OBJECT

private slots:
    void testDisplay7ColorSetting();
    void testDisplay7SaveLoad();
    void testDisplay7AllColors();
    void testDisplay7Paint();
    void testDisplay7LoadPreV16RemapsPinsToCanonicalOrder();
};
