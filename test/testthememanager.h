// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include "thememanager.h"

class TestThemeManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Basic theme manager functionality
    void testThemeManagerConstruction();
    void testThemeManagerInstance();
    void testThemeManagerSingleton();
    
    // Theme operations
    void testCurrentTheme();
    void testSetTheme();
    void testAvailableThemes();
    void testThemeExists();
    void testDefaultTheme();
    
    // Theme switching
    void testThemeSwitch();
    void testThemeSwitchNotification();
    void testThemeSwitchRollback();
    void testInvalidThemeSwitch();
    void testRapidThemeSwitching();
    
    // Pixmap and resource management
    void testPixmapLoading();
    void testPixmapCaching();
    void testPixmapScaling();
    void testPixmapThemes();
    void testMissingPixmaps();
    void testCorruptedPixmaps();
    
    // Theme file operations
    void testThemeFileLoading();
    void testThemeFileSaving();
    void testThemeFileValidation();
    void testThemeFileFormat();
    void testCustomThemeFiles();
    void testThemeFilePermissions();
    
    // Theme configuration
    void testThemeSettings();
    void testThemeProperties();
    void testThemeColors();
    void testThemeFonts();
    void testThemeStyles();
    void testThemeMetadata();
    
    // Resource path management
    void testResourcePaths();
    void testThemeResourcePath();
    void testPixmapResourcePath();
    void testRelativeResourcePaths();
    void testAbsoluteResourcePaths();
    void testResourcePathResolution();
    
    // Theme inheritance and hierarchy
    void testThemeInheritance();
    void testThemeOverrides();
    void testThemeHierarchy();
    void testParentThemes();
    void testThemeChaining();
    
    // Dynamic theme updates
    void testDynamicThemeUpdate();
    void testLiveThemeChange();
    void testThemeUpdateNotifications();
    void testElementThemeUpdate();
    void testGlobalThemeUpdate();
    
    // Error handling and validation
    void testThemeValidation();
    void testInvalidThemes();
    void testMissingThemeFiles();
    void testCorruptedThemes();
    void testThemeLoadFailures();
    void testThemeErrorRecovery();
    
    // Performance and optimization
    void testThemeLoadingPerformance();
    void testThemeCachingPerformance();
    void testMemoryUsage();
    void testPixmapMemoryManagement();
    void testResourceCleanup();
    
    // Integration with elements
    void testElementThemeIntegration();
    void testElementPixmapUpdate();
    void testElementThemeChange();
    void testThemeConsistency();
    void testThemeApplicationOrder();
    
    // Custom themes and user themes
    void testCustomThemeCreation();
    void testCustomThemeInstallation();
    void testUserThemePreferences();
    void testThemeImportExport();
    void testThemePackaging();
    
    // Platform and system integration
    void testSystemThemeDetection();
    void testPlatformThemes();
    void testHighDPIThemes();
    void testDarkModeDetection();
    void testSystemColorScheme();
    
    // Edge cases and stress testing
    void testManyThemes();
    void testLargeThemes();
    void testFrequentThemeSwitching();
    void testConcurrentThemeAccess();
    void testThemeUnloading();
    
private:
    void validateThemeManagerState();
    void createTestTheme(const QString& name);
    void cleanupTestThemes();
    QString createTestPixmapPath();
    void testThemeProperty(const QString& property, const QVariant& value);
};