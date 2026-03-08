// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>

class TestWorkspaceFileops : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    // Save Operations Tests (4 tests)
    void testSaveToNewFile();
    void testSaveToExistingFile();
    void testSaveWithSpecialCharactersInFilename();
    void testSavePreservesElementData();

    // Load Operations Tests (4 tests)
    void testLoadFromValidFile();
    void testLoadNonExistentFileThrowsException();
    void testLoadCorruptedFileHandling();
    void testLoadEmptyCircuit();

    // File State Management Tests (4 tests)
    void testModifiedFlagAfterAddElement();
    void testModifiedFlagClearedAfterSave();
    void testFilePathUpdatedAfterSave();
    void testFilePathPreservedAfterLoad();

    void testFileExtensionPandaAppend();
    void testFileExtensionNoDuplicate();
    void testDolphinFileNameStorage();
    void testLastIdInitializationValue();
    void testLastIdGetterSetter();
    void testLastIdPersistenceOnLoad();
    void testFileInfoAfterCreation();

private:
    QTemporaryDir m_tempDir;
};

