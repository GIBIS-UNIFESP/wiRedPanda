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

    // Typed Accessors
    void testTypedMainWindowGeometry();
    void testTypedMainWindowState();
    void testTypedSplitterGeometry();
    void testTypedSplitterState();
    void testTypedDolphinGeometry();
    void testTypedFastMode();
    void testTypedLabelsUnderIcons();
    void testTypedLanguage();
    void testTypedTheme();
    void testTypedRecentFiles();
    void testTypedAutosaveFiles();

private:
    QString m_testOrganization = "TestWiredPanda";
    QString m_testApplication = "TestSettings";
};

