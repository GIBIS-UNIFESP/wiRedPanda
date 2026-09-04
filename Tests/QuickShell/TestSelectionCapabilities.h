// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestSelectionCapabilities : public QObject
{
    Q_OBJECT

private slots:

    void testSelectionCapabilities();
    void testDemuxSelectionCannotChangeInputSize();

    void testSingleElementReflectsItsOwnFeatureFlags();
    void testMixedSelectionAndReducesFeatureFlags();
    void testHasSameLabelReflectsWhetherLabelsMatch();
    void testHasSameTypeFalseAndElementTypeUnknownForMixedTypes();
    void testCanChangeInputSizeFalseWhenInputSizeIsFixed();
    void testCanChangeOutputSizeFalseForTruthTableRegardlessOfRange();
    void testHasLatchedValueFalseWhenSelectionIncludesInputButton();
    void testCanMorphAcrossInputAndStaticInputGroups();
    void testCanMorphFalseAcrossUnrelatedGroups();
    void testSameCheckStateAndSameOutputValueForInputElements();
};
