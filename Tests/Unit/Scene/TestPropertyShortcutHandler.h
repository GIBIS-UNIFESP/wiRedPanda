// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestPropertyShortcutHandler : public QObject
{
    Q_OBJECT

private slots:

    void testGateInputCountNext();
    void testGateInputCountPrev();
    void testClockFrequencyNext();
    void testClockFrequencyPrev();
    void testBuzzerFrequency();
    void testLedColorSecondary();
    void testDisplayColorPrev();
    void testInputRotaryOutputSize();
    void testMorphNextElement();
    void testMorphPrevElement();
    void testMultiElementShortcutIsSingleUndoStep();

    // Display16's previous-color-only main-property branch
    void testDisplay16ColorPrevMainProperty();
    // A "not implemented" main-property type (e.g. AudioBox) is a genuine no-op
    void testAudioBoxMainPropertyIsNoOp();
    // TruthTable's output-size secondary property
    void testTruthTableOutputSizeSecondaryProperty();
    // Multi-selection macro grouping for adjustSecondaryProperty()/nextElement()/prevElement()
    void testMultiElementSecondaryPropertyIsSingleUndoStep();
    void testMultiElementMorphNextIsSingleUndoStep();
    void testMultiElementMorphPrevIsSingleUndoStep();
};
