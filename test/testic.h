// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class IC;

class TestIC : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Constructor and basic functionality
    void testICConstruction();
    void testICWithParent();
    void testICDefaults();
    void testICDestruction();
    
    // File operations
    void testLoadICFile();
    void testSaveICFile();
    void testInvalidICFile();
    void testCorruptedICFile();
    void testMissingICFile();
    void testEmptyICFile();
    
    // IC properties and metadata
    void testICName();
    void testSetICName();
    void testICDescription();
    void testICVersion();
    void testICAuthor();
    void testICMetadata();
    
    // Port management
    void testICInputPorts();
    void testICOutputPorts();
    void testPortLabels();
    void testPortMapping();
    void testDynamicPorts();
    void testPortValidation();
    
    // Circuit logic and behavior
    void testICLogicExecution();
    void testICPropagationDelay();
    void testICStateManagement();
    void testComplexICLogic();
    void testNestedICs();
    void testRecursiveICs();
    
    // Serialization and file format
    void testICFileSerialization();
    void testICFileDeserialization();
    void testICFileVersionCompatibility();
    void testICFileFormatValidation();
    void testLegacyICFiles();
    
    // Graphics and visualization
    void testICPixmap();
    void testICBoundingRect();
    void testICRendering();
    void testICCustomSkin();
    void testICTheme();
    
    // Element integration
    void testICAsElement();
    void testICInCircuit();
    void testICConnections();
    void testICSignalFlow();
    void testMultipleICInstances();
    
    // File path and resource management
    void testICFilePath();
    void testSetICFilePath();
    void testRelativeFilePaths();
    void testAbsoluteFilePaths();
    void testICFileResolution();
    void testICFileCache();
    
    // Validation and error handling
    void testICValidation();
    void testICErrors();
    void testICWarnings();
    void testICFileCorruption();
    void testICLoadFailure();
    void testICCircularDependency();
    
    // Performance and optimization
    void testICLoadPerformance();
    void testLargeICFile();
    void testICMemoryUsage();
    void testICCaching();
    void testICUnloading();
    
    // Configuration and settings
    void testICConfiguration();
    void testICParameters();
    void testICSettings();
    void testICPreferences();
    
    // Advanced features
    void testICLibraries();
    void testICTemplates();
    void testICInheritance();
    void testICPolymorphism();
    void testICFactoryPattern();
    
    // Integration and compatibility
    void testICWithSimulation();
    void testICWithWaveform();
    void testICWithArduino();
    void testICExportImport();
    
    // Edge cases and stress testing
    void testVeryLargeIC();
    void testManyICInstances();
    void testDeepICNesting();
    void testICMemoryLimits();
    void testConcurrentICAccess();
    
private:
    IC* createTestIC();
    IC* createICFromFile(const QString& filename);
    void validateICState(IC* ic);
    void createTestICFile(const QString& filename, const QString& content);
    bool compareICs(IC* ic1, IC* ic2);
    QString createValidICContent();
    QString createInvalidICContent();
};