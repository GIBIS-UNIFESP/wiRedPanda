// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDataStream>
#include <QObject>
#include <QPointF>
#include <QTemporaryDir>
#include <QTest>
#include <QVersionNumber>

#include "enums.h"

class WorkSpace;
class Scene;
class Simulation;
class GraphicElement;

class TestSerializationRegression : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Current example files regression
    void testCurrentExampleFiles();
    void testCurrentExampleFiles_data();

    // Version-specific tests
    void testLegacyFileLoading();
    void testLegacyFileLoading_data();

    // Error handling tests
    void testCorruptedFiles();
    void testUnsupportedVersions();

    // Cross-version compatibility
    void testSaveLoadRoundTrip();
    void testVersionUpgrade();

private:
    struct TestFileInfo {
        QString filename;
        QVersionNumber expectedVersion;
        QString description;
        bool shouldLoad;
        bool shouldShowWarning;
        int expectedElementCount;
        int expectedConnectionCount;
    };

    // Use real ElementType from enums.h instead of defining our own

    void setupTestFiles();
    void validateFileLoading(const TestFileInfo& info);
    bool verifyCircuitIntegrity(WorkSpace* workspace, const TestFileInfo& info);
    // Removed synthetic file creation methods - using real example files instead

    // Error testing helpers
    void testCorruptedHeader();
    void testTruncatedFile();
    void testInvalidElementData();

    QTemporaryDir* m_tempDir;
    QList<TestFileInfo> m_testFiles;
};
