// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestSettings : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Basic settings functionality
    void testSettingsConstruction();
    void testSettingsInstance();
    void testSettingsSingleton();
    
    // Settings persistence
    void testSettingsSave();
    void testSettingsLoad();
    void testSettingsFile();
    void testSettingsPath();
    
    // Individual setting operations
    void testGetSetting();
    void testSetSetting();
    void testSettingExists();
    void testRemoveSetting();
    
    // Setting types and values
    void testBooleanSettings();
    void testIntegerSettings();
    void testStringSettings();
    void testFloatSettings();
    void testComplexSettings();
    
    // Default values and fallbacks
    void testDefaultValues();
    void testSettingFallbacks();
    void testMissingSettings();
    void testInvalidSettings();
    
    // Settings validation
    void testSettingValidation();
    void testSettingConstraints();
    void testInvalidValues();
    void testOutOfRangeValues();
    
    // Settings groups and organization
    void testSettingsGroups();
    void testNestedSettings();
    void testSettingsHierarchy();
    void testGroupOperations();
    
    // Settings synchronization
    void testSettingsSync();
    void testConcurrentAccess();
    void testMultipleInstances();
    void testSettingsLocking();
    
    // File operations and error handling
    void testCorruptedSettingsFile();
    void testMissingSettingsFile();
    void testReadOnlySettingsFile();
    void testSettingsFilePermissions();
    
    // Settings migration and versioning
    void testSettingsVersion();
    void testSettingsMigration();
    void testLegacySettings();
    void testVersionCompatibility();
    
    // Performance and optimization
    void testSettingsPerformance();
    void testLargeSettingsFile();
    void testFrequentAccess();
    void testMemoryUsage();
    
    // Integration and dependencies
    void testApplicationSettings();
    void testGlobalSettings();
    void testUserSettings();
    void testSystemSettings();
    
    // Edge cases and error conditions
    void testEmptySettings();
    void testSpecialCharacters();
    void testUnicodeSettings();
    void testBinaryData();
    
private:
    void validateSettingsState();
    void createTestSettings();
    void cleanupTestSettings();
    QString getTestSettingsPath();
};