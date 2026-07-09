// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTest>

class IC;
class Scene;

class TestIC : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // Port Mapping (2 tests)
    void testICPortMapping();
    void testICEmptyPorts();

    // Nested ICs (2 tests)
    void testICNestedSingleLevel();
    void testICNestedMultiLevel();

    // Logic Generation (1 test)
    void testICLogicGeneration();

    // Serialization (1 test)
    void testICSaveLoad();

    // Integration (4 tests)
    void testICInSimulation();
    void testICStatusPropagation();
    void testICRequiredPorts();
    void testICDefaultValues();

    // Error Handling (1 test)
    void testICMissingFile();

    void testICNestedCircuitPortMapping();
    void testICNestedCircuitSignalPropagation();
    void testICFileDependencyResolution();

    // Cross-platform path resolution
    void testICLoadWithRelativePath();
    void testICLoadWithRelativeSubfolderPath();
    void testICLoadWithSameOsAbsolutePath();
    void testICLoadWithForeignAbsolutePathForwardSlash();
    void testICLoadWithForeignAbsolutePathBackslash();
    void testICLoadWithForeignAbsolutePathMixedSlashes();
    void testICLoadWithNonExistentFileFallback();

    // Migration (4 tests)
    void testICMigrationPreservesConnections();
    void testICFileMigrationCreatesBackup();         // old sub-circuit file gets versioned backup
    void testICFileMigrationUpdatesSubcircuitVersion(); // sub-circuit re-saved at current version
    void testICFileMigrationDisabledSkips();         // no backup when migration disabled

    // Rendering under rotation (4 tests)
    void testICRotationKeepsSizingPixmap();          // drawBody() sizes the body from pixmap().rect()
    void testICRotationPivotsAtFootprintCenter();
    void testICRotatedBodyStillPaintsMascot();
    void testICRotatedMascotStaysUpright();

    // Rotation/flip pivot on a "big" IC (ports extend past the 64x64 body)
    void testICBigPivotsAtBoundingRectCenter();
    void testICBigRotationDoesNotDriftInScene();
    void testICBigFlipDoesNotDriftInScene();

private:
    /// Builds an IC with enough ports that its body outgrows the nominal 64x64 box, without
    /// needing a fixture file. Needs friend access to IC's protected port-size setters (this
    /// class is already `friend class TestIC` in IC.h), so it lives here rather than as a free
    /// helper function.
    IC *buildBigIC(Scene *scene);
};
