// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QVector>

class LogicInput;

class TestLogicElementsEdgeCases : public QObject
{
    Q_OBJECT

private slots:
    void cleanup();
    void init();

    // Basic gate edge cases
    void testLogicAndEdgeCases();
    void testLogicOrEdgeCases();
    void testLogicNotEdgeCases();
    void testLogicNandEdgeCases();
    void testLogicNorEdgeCases();

    // XOR/XNOR edge cases
    void testLogicXorEdgeCases();
    void testLogicXnorEdgeCases();

    // Sequential element edge cases
    void testLogicDFlipFlopEdgeCases();
    void testLogicJKFlipFlopEdgeCases();
    void testLogicSRFlipFlopEdgeCases();
    void testLogicTFlipFlopEdgeCases();
    void testLogicDLatchEdgeCases();

    // Complex element edge cases
    void testLogicMuxEdgeCases();
    void testLogicDemuxEdgeCases();
    void testLogicTruthTableEdgeCases();

    // Infrastructure element edge cases
    void testLogicSRLatchEdgeCases();
    void testLogicInputEdgeCases();
    void testLogicOutputEdgeCases();
    void testLogicNodeEdgeCases();

    // Multi-input boundary tests
    void testMultiInputBoundaries();
    void testInvalidInputHandling();
    void testUpdateWithoutInputs();

private:
    void setupInputs(int count);
    void setInputValues(const QVector<bool> &values);

    QVector<LogicInput *> switches{10}; // More switches for edge cases
};
