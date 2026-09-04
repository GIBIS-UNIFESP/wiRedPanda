// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>
#include <QTest>

/// Exercises the IC blob registry (CanvasICRegistry, owned by CanvasItem): embed/extract,
/// serialization, the workspace inline tab, undo/redo, and registry CRUD.
///
/// DropArea.onDropped reads drop.source.modelData directly, with no serialization step, so
/// tests targeting Widgets' QMimeData drag-and-drop transport have no Quick-side equivalent
/// and are not present here.
class TestICInline : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // Core embed/extract
    void testEmbedFromBlob();
    void testEmbedPreservesSimulation();
    void testExtractToFile();
    void testRoundTripEmbedExtract();

    // Serialization
    void testSaveLoadWithEmbeddedIC();
    void testBlobRegistryPersistence();

    // Workspace inline tab
    void testInlineTabSaveLoad();
    void testChildBlobPropagation();
    void testInlineTabSaveThrowsWhenFileBackedIcMissingOnDisk();

    // Undo/redo
    void testUndoRedoEmbedExtract();

    // ICRegistry
    void testRegistryBlobCRUD();
    void testRegistryRename();
    void testRegistryFindByBlobName();
    void testRegistryEmbedICsByFile();
    void testRegistryClearBlobs();

    // --- Core load/embed/extract ---

    void testLoadFromBlob();
    void testFlattenBlobRecursive();
    void testFlattenBlobMissingFile();
    void testFlattenBlobNoConversion();
    void testFlattenBlobMixedInlineFileBacked();
    void testFlattenBlobCircularReference();
    void testFlattenBlobSelfReferenceName();
    void testEmbedSimulatesCorrectly();
    void testEmbedSimulatesMultiLevel();
    void testEmbedExtractAtSceneLevel();
    void testEmbedExtractWithActiveConnections();
    void testLoadFromBlobNested();

    // --- Blob name and serialization ---

    void testBlobNamePreservation();
    void testSetBlobNameAlone();
    void testSetBlobNameOnFileBacked();
    void testLabelIndependentFromBlobName();
    void testBlobNameRenamePropagation();
    void testBlobNameSpecialCharacters();
    void testBlobVersionMigrationPath();
    void testLoadFromBlobPreservesBlob();
    void testLoadFromBlobEmptyCircuit();
    void testLoadFromBlobNonExistentContextDir();
    void testLoadV41MapDirectConstruct();
    void testLoadMismatchNoFileName();
    void testReEmbedWithDifferentBlob();

    // --- Workspace and inline tab ---

    void testWorkspaceInlineTab();
    void testWorkspaceInlineSaveOrphanedParent();
    void testWorkspaceAutosaveSkipsInlineIC();
    void testWorkspaceLoadFromBlobBlobNameDerivation();
    void testInlineSaveContextDirSwitch();
    void testInlineTabSaveAfterModification();
    void testLoadFromBlobNullParent();
    void testLoadFromBlobInvalidParentICId();
    void testNestedWorkspaceChainPropagation();
    void testLoadFromBlobCleansUpConnections();

    // --- Child blob propagation ---

    void testOnChildICBlobSavedOrphan();
    void testOnChildICBlobSavedPropagation();
    void testOnChildICBlobSavedNonEmbeddedTarget();
    void testOnChildICBlobSavedCorruptBlob();
    void testOnChildICBlobSavedCorruptBlobMultiTarget();
    void testOnChildICBlobSavedPortCountWithConnections();
    void testOnChildICBlobSavedBlobNameMismatch();
    void testOnChildICBlobSavedAllOrNothing();

    // --- Multiple instances and remove ---

    void testMultipleInstancesBlobUpdate();
    void testMultipleInstancesBlobUpdateLabelsPreserved();
    void testMultipleDifferentEmbeddedTypes();
    void testRemoveEmbeddedIC();
    void testRemoveEmbeddedICWithConnections();
    void testRemoveEmbeddedICNonExistent();
    void testRemoveEmbeddedICMultipleInstances();
    void testRemoveEmbeddedICCrossConnections();
    void testMixedScene();
    void testPortCountChangeConnectionCleanup();

    // --- Serialization mismatch and rollback ---

    void testSerializationMismatchFallback();
    void testSerializationMismatchFallbackCase2State();
    void testSetInlineDataRollback();
    void testSetInlineDataRollbackSimulationWorks();
    void testSetInlineDataRollbackFromEmbedded();
    void testSetInlineDataEmptyBlobName();
    void testSetInlineDataEmptyBlobThrows();
    void testSetInlineDataEmptyBlobNameRoundTripFails();
    void testSimulationRestartTimingDuringSetInlineData();

    // --- Save/load round-trip and mixed ---

    void testSaveLoadRoundTrip();
    void testLoadFileClearsInlineData();
    void testCopyFileGuardDuringPaste();
    void testCopyPasteEmbeddedICRoundTrip();
    void testFlipRotateEmbeddedIC();

    // --- UI widgets ---

    void testSelectionCapabilitiesEmbedded();
    void testSelectionCapabilitiesSingleNonIC();
    void testSelectionCapabilitiesEmptyList();
    void testContextMenuICActionConditions();

    // --- SystemVerilog and drop-zone workflows ---

    void testSystemVerilogCodegenEmbeddedIC();
    void testSystemVerilogCodegenTwoEmbeddedSameBlobName();
    void testSVCodegenReservedWordBlobName();
    void testEmbedICByDropConversion();
    void testEmbedICByDropCollisionAutoSuffix();
    void testExtractICByDropConversion();
    void testExtractICByDropFileCollision();

    // --- Undo/redo and position ---

    void testMultiCycleUndoRedoPropagation();
    void testPositionRotationPreservedDuringPropagation();
    void testLoadFromBlobClearsFileState();

    // --- Blob registry and orphan cleanup ---

    void testRemoveEmbeddedICUndoRestoresBlob();

    // Regression: drop handler must not orphan element/mimeData on throw
    void testBlobRegistryMergeConflictSkipsExisting();
    void testUniqueNameCollision();
    void testOrphanBlobAfterDeleteAllInstances();
    void testImportNestedBlobsViaRegisterBlob();

    // --- Edge cases: ICRegistry ---

    void testRenameBlobSameNameNoOp();
    void testRenameBlobCollisionRejected();
    void testRenameBlobNonExistentNoOp();
    void testRemoveBlobNonExistent();
    void testInitEmbeddedICMissingBlob();
    void testInitEmbeddedICPreservesExistingLabel();
    void testEmbedICsByFileNoTargets();
    void testExtractToFileOverwritesExisting();
    void testBlobRegistryEmptyRoundTrip();
    void testRegisterBlobFlattensDeeplyNested();

    // --- Edge cases: IC load/save and undo ---

    void testLoadICWithMissingBlobFallsBackToFile();
    void testLoadICWithNullBlobRegistry();
    void testSaveLoadOrphanBlobPreserved();
    void testReconnectConnectionsSkipsDeletedElement();
    void testUpdateBlobCommandUndoRestoresOldBlob();
    // Regression: a cosmetic property edit (label) re-serializes/reloads an embedded IC via
    // UpdateCommand even when the blob reference is unchanged -- IC::load() must skip the
    // redundant resetInternalState()+loadFromBlob() in that case (large-IC label-edit lag).
    void testLabelUpdateSkipsRedundantSubcircuitReload();
    // Companion: if the blob's bytes DO change under an unchanged name (UpdateBlobCommand's
    // undo/redo), IC::load() must still reload -- the skip must key on content, not just name.
    void testBlobContentChangeUnderSameNameStillReloads();
    void testIsEmbeddedICWithStaleBlobName();
    void testRegisterBlobCommandUndoRedo();
    void testRegisterBlobCommandRedoAfterExternalRemove();
    void testRemoveBlobCommandUndoRedo();
    void testRenameBlobCommandUndoRedo();
    // Regression: a rename and an unrelated property edit must each undo independently — the
    // old apply()-triggered, untracked renameBlob() call let a later UpdateCommand undo try to
    // resolve the pre-rename blob name against a registry that no longer had it.
    void testRenameBlobCommandThenUnrelatedUpdateBothUndoIndependently();
    void testLoadICMissingAllNameFieldsThrows();
    void testUniqueBlobNameEmptyBase();
    void testBlobReturnsEmptyForMissingName();
    void testClearBlobsLeavesICsStale();
    void testLoadFileNotFoundPreservesEmbeddedState();
    void testExtractToFileUsesContextDir();

    // --- Edge cases: rollback, rename, and missing deps ---

    void testChildBlobSavedPartialFailureRollback();
    void testRenameBlobUpdatesNestedMetadata();
    void testRegisterBlobMissingFileDepWarns();

    // --- Inline IC save with file-based IC conversion ---

    void testInlineSaveConvertsFileBackedToEmbedded();
    void testNestedInlineSaveAndReopen();

    // Regression: inline-IC save must preserve wires touching file-backed sub-ICs
    void testInlineSavePreservesWiresA15();

    // Regression: onFileChanged must push UpdateBlobCommand for undo
    void testOnFileChangedPushesUndoCommandC5();

    // Regression: pasting an embedded IC previously registered its blob directly into the
    // registry, outside the undo system — undoing the paste left it orphaned forever.
    void testPasteEmbeddedICBlobUndoRemovesOrphan();

    // Regression: embedICsByFile()/extractToFile() mutated live, already-in-scene ICs with no
    // SimulationBlocker, unlike sibling onFileChanged() — a UAF risk if the IC contains a
    // sequential element while the simulation is running.
    void testEmbedICsByFileWithSequentialElementWhileSimulationRunning();

    // Regression: ICLoader::loadFileDirectly() only had cycle detection, not a depth cap
    // (unlike its sibling deserializeAndLoad()) — a long, non-cyclic chain of distinct file-
    // backed ICs recursed unbounded and could exhaust the call stack.
    void testLoadFileDirectlyEnforcesNestingDepthLimit();

private:
    QString fixturesSrcDir() const;
    bool copyFixture(const QString &name);

    QTemporaryDir m_tempDir;
    QString m_fixtureDir;
};
