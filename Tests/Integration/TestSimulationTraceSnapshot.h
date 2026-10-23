// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/**
 * \brief Pins what master's simulation computes, step by step, with its clocks frozen.
 * \details Every traced logic type at its fewest and its most ports goes through every combination of up to eleven
 * wired inputs and then a seeded walk, wired and with its last input open; the six small fixtures, the 73 IC
 * components, the four-gate perf IC and the IC-heavy circuit go through a seeded walk of their inputs. Each step is
 * one update() of the simulation, whose timer never starts, so a clock keeps the level the first update gives it.
 */
class TestSimulationTraceSnapshot : public QObject
{
    Q_OBJECT

private slots:
    void testElementTracesMatchGolden();
    void testCircuitTracesMatchGolden_data();
    void testCircuitTracesMatchGolden();
    void testClocksHoldWhileStepped();
    void testEveryGoldenHasACircuit();
};
