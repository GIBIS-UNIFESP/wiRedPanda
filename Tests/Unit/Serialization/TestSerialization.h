// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QByteArray>
#include <QObject>
#include <QTemporaryDir>
#include <QVersionNumber>

class WorkSpace;
class GraphicElement;

class TestSerialization : public QObject
{
    Q_OBJECT

private slots:
    // Lifecycle
    void init();

    // Basic round-trip tests
    void testSingleElementRoundTrip();
    void testMultipleElementsRoundTrip();
    void testConnectionPersistence();
    void testElementPropertiesPreserved_data();
    void testElementPropertiesPreserved();

    // Complex circuits
    void testComplexCircuitSaveLoad();
    void testICSerializationBasic();

    // File format
    void testFileFormatHeader();
    void testDolphinFilenamePreserved();
    void testSceneRectPreserved();

    // Error handling
    void testInvalidMagicHeader();
    void testCorruptedStream();

    // Property persistence
    void testRotationPreserved_data();
    void testRotationPreserved();
    void testLabelPreserved_data();
    void testLabelPreserved();
    void testInputSizePreserved();
    void testOutputSizePreserved();

    // Special cases
    void testEmptyCircuit();

    void testLargeCircuitWith100Elements();
    void testLargeCircuitWith500Elements();

    void testUnicodeInLabels();
    void testSpecialCharactersInLabels();
    void testExtendedAsciiCharacters();

    void testMissingRequiredAttributes();
    void testInvalidElementTypeInStream();
    void testNegativePositionValues();
    void testExtremelyLargeValues();
    void testNullElementHandling();

    void testOldVersionHeaderFormat();
    void testMissingVersionHeader();

    void testTruncatedFileRecovery();
    void testInvalidUTF8InLabels();
    void testMismatchedElementCount();
    void testMultiGateChainSerialization();
    void testExtremelyLongLabels();
    void testStreamPositionValidation();
    void testConnectionWithDeletedPorts();
    void testMalformedConnectionData();
    void testMalformedEmbeddedICsRegistryRejected();
    void testMalformedWaypointsDataRejected();
    void testConnectionWireModeAndWaypointsRoundTrip();

    // Wireless node serialization (4 tests)
    void testWirelessTxNodePreservedInScene();
    void testWirelessRxNodePreservedInScene();
    void testWirelessPairCircuit();
    void testWirelessNoneNodeInScene();

    // Versioned backup utility (Serialization::createVersionedBackup)
    void testVersionedBackupNamingPattern();    // backup name: basename.vX.Y.Z.ext
    void testVersionedBackupIsIdempotent();     // second call does not overwrite existing backup
    void testVersionedBackupPreservesOriginal();// original file content unchanged after backup
    void testVersionedBackupMultiVersions();    // different versions produce different backup files

    // File-copy utility (Serialization::copyPandaFile)
    void testCopyPandaFileCopiesNonPandaContent();          // non-panda content still gets copied, no recursion
    void testCopyPandaFileCopiesFileBackedDependency();     // a real fileBackedICs entry is also copied
    // Regression: D16 — circular fileBackedICs metadata must not infinite-recurse
    void testCopyPandaFileTerminatesOnCircularMetadata();
    // Regression: a long, non-cyclic chain of distinct fileBackedICs dependencies must be
    // rejected past a fixed nesting depth, not recurse until the call stack is exhausted.
    void testCopyPandaFileRejectsDeepDependencyChain();

    // libFuzzer regression fixtures (Tests/Fuzz/regressions/) — malformed
    // .panda inputs that must throw cleanly under ASan, never crash.
    void testFuzzRegressionCleanupUAF();
    void testFuzzRegressionICBlobShrink();
    void testFuzzRegressionUnboundedPortList();
    void testFuzzRegressionConnectionStreamMapOOM();   // bugD
    void testFuzzRegressionICBoundaryOrphanConns();     // bugE
    void testFuzzRegressionUnknownTypeIdMetadataOOM();  // bugF
    void testFuzzRegressionElementStreamMapOOM();       // bugG
    void testFuzzRegressionOldFormatV11Elements();      // bugH
    void testFuzzRegressionOldFormatICSkinRef();        // bugI
    void testFuzzRegressionDolphinFilenameOOM();        // oom_dolphin_filename_large
    void testFuzzRegressionPandaHeaderAppNameOOM();     // oom_panda_header_large_appname
    void testFuzzRegressionNonFinitePosition();         // ic_nonfinite_position (fuzz_ic_file)
    void testFuzzRegressionNonFiniteRotation();         // ic_nonfinite_rotation (fuzz_ic_file)

    // Regression: GraphicElementSerializer::loadTrigger() (old-format, version 1.9-4.0)
    // used raw, unbounded QKeySequence deserialization instead of the bounded reader
    // already used by the new-format path.
    void testReadBoundedKeySequenceRoundTrip();
    void testReadBoundedKeySequenceAcceptsMaxFourKeys();
    void testReadBoundedKeySequenceRejectsImplausibleCount();

private:
    // Helper methods
    QByteArray saveToMemory(WorkSpace &workspace);
    void loadFromMemory(WorkSpace &workspace, const QByteArray &data);
    void verifyElementEquality(GraphicElement *original, GraphicElement *loaded);

    QTemporaryDir m_tempDir;
};
