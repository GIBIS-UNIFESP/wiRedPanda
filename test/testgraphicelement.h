// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class QGraphicsItem;

class TestGraphicElement : public QObject
{
    Q_OBJECT

private slots:
    // Constructor and basic properties tests
    void testGraphicElementConstruction();
    void testElementTypeAndGroup();
    void testElementProperties();
    void testPixmapHandling();
    void testBoundingRect();
    
    // Port management tests
    void testInputPortCreation();
    void testOutputPortCreation();
    void testPortSizeManagement();
    void testPortPropertiesUpdate();
    void testInputPortAccess();
    void testOutputPortAccess();
    void testPortValidation();
    
    // Element validation and state tests
    void testElementValidation();
    void testElementCapabilities();
    void testElementLimits();
    void testRotationHandling();
    void testPositionHandling();
    
    // Label and appearance tests
    void testLabelManagement();
    void testThemeUpdate();
    void testSkinManagement();
    void testColorManagement();
    void testAudioManagement();
    
    // Serialization tests
    void testSaveElement();
    void testLoadElement();
    void testSaveLoadRoundTrip();
    void testVersionCompatibility();
    void testPortMapSerialization();
    
    // Advanced features tests
    void testTriggerHandling();
    void testPriorityManagement();
    void testFrequencyAndDelay();
    void testTruthTableSupport();
    
    // Integration tests
    void testElementInScene();
    void testElementConnections();
    void testElementCloning();
    void testElementTransformations();
    
private:
    void validateElementState(class GraphicElement *element);
    void testSerializationRoundTrip(class GraphicElement *element);
};