// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestSettings : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    // Basic Operations
    /// The whole test process must resolve the Settings singleton to the temp-dir
    /// redirect installed by TestUtils::setupTestEnvironment() — never to the user's
    /// real config file, which tests would otherwise overwrite (language, theme,
    /// geometry, recent files).
    void testSettingsRedirectedAwayFromUserConfig();
    void testSettingsFileName();

    // Typed Accessors
    void testTypedMainWindowGeometry();
    void testTypedMainWindowState();
    void testTypedSplitterGeometry();
    void testTypedSplitterState();
    void testTypedDolphinGeometry();
    void testTypedMinimapGeometry();
    void testTypedFastMode();
    void testTypedLabelsUnderIcons();
    void testTypedUpdateChecksDisabled();
    void testTypedLanguage();
    void testTypedTheme();
    void testTypedRecentFiles();
    void testTypedAutosaveFiles();
};
