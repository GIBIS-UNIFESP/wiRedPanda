// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestTourEngine : public QObject
{
    Q_OBJECT

private slots:
    void testRetranslateBeforeLoadIsNoOp();
    void testRetranslateWhileInactiveIsNoOp();
    void testRetranslatePreservesProgressAndData();
    void testRetranslateEmitsRetranslatedOnly();

    // TourOverlay::scaledFontPx() (#14 accessibility: font-scale-hostile hardcoded px sizes)
    void testOverlayFontScalesWithApplicationFont();
};
