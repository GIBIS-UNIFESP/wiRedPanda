// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestSettings : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();

    // Basic Operations
    void testSettingsFileName();
    void testSettingsSetAndGet();
    void testSettingsContains();
    void testSettingsRemove();
    void testSettingsGetNonExistent();
    void testSettingsOverwrite();

    // Data Types
    void testSettingsString();
    void testSettingsInteger();
    void testSettingsBoolean();
    void testSettingsByteArray();
    void testSettingsStringList();
    void testSettingsEmptyStringList();
    void testSettingsNullVariant();

    // Real-World Settings
    void testThemePersistence();
    void testLanguagePersistence();
    void testRecentFilesPersistence();
    void testAutosaveFilesPersistence();
    void testWindowGeometryPersistence();
    void testBooleanSettings();
    void testHierarchicalKeys();
    void testWarningFlagsPersistence();

    // Edge Cases
    void testMultipleReads();
    void testClearAllSettings();
    void testVeryLongKey();
    void testVeryLargeValue();

private:
    QString m_testOrganization = "TestWiredPanda";
    QString m_testApplication = "TestSettings";
};

