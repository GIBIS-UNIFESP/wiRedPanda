// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestSequentialLogic : public QObject
{
    Q_OBJECT

private slots:
    /// All four flip-flops must sample their control inputs the same way: LIVE at the edge.
    /// Under the three-region drain, live IS the pre-edge value -- Memory elements are held out
    /// of the active region, so nothing they read has published yet. Latching the PREVIOUS
    /// evaluation's value in any one of them would make the family disagree with itself, and
    /// "the previous evaluation" is not a defined instant in an event-driven engine anyway; it
    /// is whenever the element last happened to be woken.
    void testFlipFlopFamilyAgreesOnControlInputSampling();

    // Port count and type verification
    void testDFlipFlop();
    void testDLatch();
    void testJKFlipFlop();
    void testSRFlipFlop();
    void testSRLatch();
    void testTFlipFlop();

    // Behavioral tests
    void testDFlipFlopBehavior();
    void testDLatchBehavior();
    void testJKFlipFlopBehavior();
    void testSRFlipFlopBehavior();
    void testSRLatchBehavior();
    void testTFlipFlopBehavior();

    // resetSimState() tests (only the 4 asynchronous-preset/clear flip-flops override it;
    // DLatch/SRLatch have no edge-detection state to restore)
    void testDFlipFlopResetSimState();
    void testJKFlipFlopResetSimState();
    void testSRFlipFlopResetSimState();
    void testTFlipFlopResetSimState();

    // updateTheme() reloads the theme-specific pixmap for every sequential element
    void testSequentialElementsUpdateTheme();
};
