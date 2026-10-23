// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/**
 * \brief Loads a circuit and runs one measured phase per test function.
 * \details Each phase's work is a never-inlined function in TestPerfProbe.cpp, so
 * `valgrind --tool=callgrind --toggle-collect='*::phaseX(*'` counts that phase alone;
 * Scripts/profile_probe.sh runs each phase that way and prints the instruction counts. The class is skipped
 * unless WIREDPANDA_PERF_PROBE is set, since its run takes about 20 seconds; with it set, it is a smoke test on
 * Tests/Fixtures/Perf/clocked_2000.panda (or the circuit in PROBE_FILE). The probe is always compiled, so it
 * cannot rot.
 *
 * Wires get their geometry, as in the application. The elements an edit uses are picked in ascending id,
 * so the same edit is measured whatever order the scene keeps its elements in. Frames are painted through
 * the workspace's own view, shown off screen at 1280x800 with fast mode off, as the application shows it.
 */
class TestPerfProbe : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void load();
    void loadThenFrames();
    void idleTick();
    void addWire();
    void addWireThenUndo();
    void moveElement();
    void deleteElement();
    void frames();
    void pan();
    void wireExtents();
};
