// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include "App/Core/ThemeManager.h"

/**
 * \brief Loads every example, IC component, fixture and perf fixture and compares a dump of its scene with a golden.
 * \details The dump lists the elements by id (type, geometry, label, port counts and the picture shown), each port with
 * its position in the element and in the scene, and the wires reached through their ports, never through the scene's
 * items. The goldens are in Tests/Integration/SceneSnapshot, one subdirectory per input directory. Each fixture is also
 * edited once by each edit and its wires' paths dumped, in Tests/Integration/WireGeometry. The pictures' paths name the
 * theme, so the class runs on the Light theme rather than on whatever the platform's System theme is.
 */
class TestSceneSnapshot : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testSnapshotMatchesGolden_data();
    void testSnapshotMatchesGolden();
    void testEveryGoldenHasACircuit();
    void testWireGeometryAfterEdit_data();
    void testWireGeometryAfterEdit();

private:
    Theme m_previousTheme = Theme::System;
};
