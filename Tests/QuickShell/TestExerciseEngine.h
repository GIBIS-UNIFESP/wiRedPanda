// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// testOverlayFontScalesWithApplicationFont is dropped, not ported: it tests
/// ExerciseOverlay::scaledFontPx(), a Widgets-only QWidget-based overlay confirmed dead for
/// Quick (QML's own font-scaling is handled declaratively, not through an equivalent C++ helper).
class TestExerciseEngine : public QObject
{
    Q_OBJECT

private slots:
    void testRetranslateBeforeLoadIsNoOp();
    void testRetranslateWhileInactiveIsNoOp();
    void testRetranslatePreservesProgressAndData();
    void testRetranslateEmitsRetranslatedOnly();
    void testNegativeMinCountClampsToZero();

    // Regression: an unknown element type in requiredElements silently made a step
    // permanently unsolvable with no diagnostic, unlike every other malformed-content path
    // in this feature.
    void testUnknownElementTypeWarnsAndNeverAdvances();
};
