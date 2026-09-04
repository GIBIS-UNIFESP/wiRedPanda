// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/**
 * \class TestICUnit
 * \brief Domain-layer coverage for IC::load/save, ICLoader, ICSimulation, and ICRenderer's
 * pixmap guards.
 *
 * \details ICPreviewPopup's on-screen preview/hover behavior is covered separately by
 * TestQuickICPreview.h (QuickICPreview) and TestCanvasItemInteraction.cpp (CanvasItem's
 * hover/double-click routing); this class covers only the domain layer.
 */
class TestICUnit : public QObject
{
    Q_OBJECT

private slots:

    void testICLoadFromFile();
    void testICPortLabelResolution();
    void testICNestedSaveLoad();
    void testICInvalidFile();

    // ICSimulation's empty-IC and disconnected-input guards
    void testUnloadedIcSimulationMethodsAreNoOps();
    void testLoadedIcWithDisconnectedInputIsUnknown();

    // ICRenderer's non-finite/degenerate-geometry defense-in-depth guards
    void testGeneratePixmapWithNonFiniteBoundsIsNoOp();
    void testGeneratePreviewPixmapWithNonFiniteBoundsIsNoOp();
    void testGeneratePreviewPixmapWithDegenerateAspectRatioIsEmpty();

    // IC::load()'s stale-portMap-key eviction (fuzz-hardening: reload shrinks port count)
    void testReloadWithFewerPortsEvictsStalePortMapKeys();

    // displayName() for an embedded (blob-backed) IC
    void testDisplayNameForEmbeddedIc();

    // ICLoader::loadFileDirectly()'s file-open and circular-reference guards
    void testLoadFileDirectlyOpenFailureThrows();
    void testLoadFileDetectsCircularSelfReference();

    // ICLoader::migrateFile()'s write-open failure, and loadFileDirectly()'s
    // itemsGuard cleanup of a still-live Connection when migration throws
    void testMigrateFileOpenForWriteFailureThrowsAndCleansUpItems();
    void testMigrateFileCommitFailureThrows();

    // loadBoundaryElement()'s port-name proxying for a boundary input with
    // more than one port (e.g. InputRotary)
    void testLoadBoundaryElementProxiesMultiOutputInputPortNames();

    // loadBoundaryPorts()'s summary qCDebug, only emitted at verbosity >= 4
    void testLoadBoundaryPortsLogsSummaryAtVerbosity();

    // ICLoader::deserializeAndLoad()'s own nesting-depth guard (the blob-cache
    // path's counterpart to loadFileDirectly()'s identical-purpose check)
    void testDeserializeAndLoadEnforcesNestingDepthLimit();
};
