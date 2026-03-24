// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/// Tests for the StatusOps 4-state domination rules and gate-level integration.
class TestStatusOps : public QObject
{
    Q_OBJECT

private slots:
    // --- StatusOps binary function tests ---
    void testStatusAnd_data();
    void testStatusAnd();
    void testStatusOr_data();
    void testStatusOr();
    void testStatusNot_data();
    void testStatusNot();
    void testStatusXor_data();
    void testStatusXor();

    // --- StatusOps fold (multi-input) function tests ---
    void testStatusAndAll();
    void testStatusOrAll();
    void testStatusXorAll();

    // --- Gate-level 4-state integration tests ---
    void testAndGateUnknownDomination();
    void testOrGateUnknownDomination();
    void testNotGateUnknownPassthrough();
    void testNandGateUnknownDomination();
    void testNorGateUnknownDomination();
    void testXorGateUnknownPropagation();
    void testXnorGateUnknownPropagation();
    void testErrorPropagationThroughChain();
    void testMuxUnknownSelectLine();
    void testMuxUnknownDataInput();
    void testDemuxUnknownSelectLine();
    void testDemuxUnknownDataInput();
    void testTruthTableUnknownInput();
    void testNodePassesUnknownAndError();
    void test3InputAndWithUnknown();
    void test3InputOrWithUnknown();

    // --- Sequential element 4-state behavior ---
    void testDFlipFlopUnknownInputHolds();
    void testDFlipFlopUnknownClockNoEdge();
    void testJKFlipFlopUnknownPresetClearNoTrigger();
    void testTFlipFlopUnknownInput();
    void testSRFlipFlopUnknownPresetClear();
    void testDLatchUnknownInput();
    void testSRLatchUnknownInput();

    // --- updateInputs vs updateInputsAllowUnknown ---
    void testUpdateInputsBlocksUnknown();
    void testUpdateInputsBlocksError();
    void testUpdateInputsAllowUnknownPassesThrough();
    void testUpdateInputsAllowUnknownBlocksNullPredecessor();
};

