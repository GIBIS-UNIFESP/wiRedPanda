// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>

class TestFeatures : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // Element Features Tests (12 tests)
    void testAndGateFeatures();
    void testInputElementFeatures();
    void testDisplayElementFeatures();
    void testFrequencyFeature();
    void testColorFeature();
    void testAudioFeature();
    void testDelayFeature();
    void testLabelFeature();
    void testTriggerFeature();
    void testTruthTableFeature();
    void testMultipleElementsFeatures();
    void testFeatureConsistency();

private:
    QTemporaryDir m_tempDir;
};

