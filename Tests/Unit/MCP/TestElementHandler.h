// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Regression coverage for MCP/Server/Handlers/ElementHandler.cpp's set_element_properties
/// "delay" validation, found stale against Clock::setDelay()'s real [-1, 1] range during a
/// deep review of commits since 5.1.3.
class TestElementHandler : public QObject
{
    Q_OBJECT

private slots:
    void testSetElementPropertiesAcceptsNegativeClockDelay();
    void testSetElementPropertiesRejectsClockDelayOutOfRange();

    void testHandleCreateElementRejectsMissingParams();
    void testHandleCreateElementRejectsEmptyType();
    void testHandleCreateElementRejectsNonNumericX();
    void testHandleCreateElementRejectsNonNumericY();
    void testHandleCreateElementRejectsInvalidType();
    void testHandleCreateElementRejectsNoScene();
    void testHandleCreateElementCreatesRealElement();
    void testHandleCreateElementAppliesLabel();

    void testHandleDeleteElementRejectsMissingParams();
    void testHandleDeleteElementRejectsUnknownElement();
    void testHandleDeleteElementDeletesRealElement();

    void testHandleListElementsRejectsNoScene();
    void testHandleListElementsReturnsRealElements();

    void testHandleMoveElementRejectsMissingParams();
    void testHandleMoveElementRejectsInvalidCoordinates();
    void testHandleMoveElementRejectsNonNumericY();
    void testHandleMoveElementRejectsUnknownElement();
    void testHandleMoveElementMovesRealElementSnappedToGrid();

    void testHandleSetElementPropertiesRejectsMissingElementId();
    void testHandleSetElementPropertiesRejectsUnknownElement();
    void testHandleSetElementPropertiesRejectsPortSizeParams();
    void testHandleSetElementPropertiesChangesLabel();
    void testHandleSetElementPropertiesChangesColorWhenSupported();
    void testHandleSetElementPropertiesChangesFrequencyWhenSupported();
    void testHandleSetElementPropertiesRejectsInvalidFrequency();
    void testHandleSetElementPropertiesRejectsNonNumericDelay();
    void testHandleSetElementPropertiesChangesRotation();
    void testHandleSetElementPropertiesChangesTriggerWhenSupported();
    void testHandleSetElementPropertiesChangesLockedState();
    void testHandleSetElementPropertiesChangesVolumeWhenSupported();
    void testHandleSetElementPropertiesRejectsInvalidVolume();
    void testHandleSetElementPropertiesChangesAppearance();
    void testHandleSetElementPropertiesChangesAppearanceAtIndex();
    void testHandleSetElementPropertiesChangesWirelessModeAndSeversConnections();
    void testHandleSetElementPropertiesAcceptsWirelessModeNoneForRealNode();
    void testHandleSetElementPropertiesRejectsInvalidWirelessMode();
    void testHandleSetElementPropertiesIgnoresWirelessModeForNonNode();
    void testHandleSetElementPropertiesPushesUndoableCommand();

    void testHandleSetInputValueRejectsMissingParams();
    void testHandleSetInputValueRejectsUnknownElement();
    void testHandleSetInputValueRejectsNonInputElement();
    void testHandleSetInputValueSetsRealValue();

    void testHandleGetOutputValueRejectsMissingParams();
    void testHandleGetOutputValueRejectsUnknownElement();
    void testHandleGetOutputValueReturnsInputElementValue();
    void testHandleGetOutputValueReturnsGenericElementValue();
    void testHandleGetOutputValueReturnsOutputGroupValue();
    void testHandleGetOutputValueRejectsInvalidPort();
    void testHandleGetOutputValueRejectsOutOfRangePort();

    void testHandleRotateElementRejectsMissingParams();
    void testHandleRotateElementRejectsInvalidAngle();
    void testHandleRotateElementRejectsUnknownElement();
    void testHandleRotateElementNormalizesAngle();

    void testHandleFlipElementRejectsMissingParams();
    void testHandleFlipElementRejectsInvalidAxis();
    void testHandleFlipElementRejectsUnknownElement();
    void testHandleFlipElementFlipsRealElement();

    void testHandleUpdateElementDelegatesAndSimplifiesResponse();
    void testHandleUpdateElementPropagatesErrors();

    void testHandleChangeInputSizeRejectsMissingParams();
    void testHandleChangeInputSizeRejectsInvalidSize();
    void testHandleChangeInputSizeRejectsOutOfRangeSize();
    void testHandleChangeInputSizeRejectsUnknownElement();
    void testHandleChangeInputSizeChangesRealSize();

    void testHandleChangeOutputSizeRejectsMissingParams();
    void testHandleChangeOutputSizeRejectsInvalidSize();
    void testHandleChangeOutputSizeRejectsOutOfRangeSize();
    void testHandleChangeOutputSizeRejectsUnknownElement();
    void testHandleChangeOutputSizeChangesRealSize();

    void testHandleToggleTruthTableOutputRejectsMissingParams();
    void testHandleToggleTruthTableOutputRejectsInvalidPosition();
    void testHandleToggleTruthTableOutputRejectsUnknownElement();
    void testHandleToggleTruthTableOutputRejectsNonTruthTable();
    void testHandleToggleTruthTableOutputRejectsOutOfRangePosition();
    void testHandleToggleTruthTableOutputTogglesRealCell();

    void testHandleMorphElementRejectsMissingParams();
    void testHandleMorphElementRejectsNonArrayElementIds();
    void testHandleMorphElementRejectsEmptyElementIds();
    void testHandleMorphElementRejectsEmptyTargetType();
    void testHandleMorphElementRejectsInvalidTargetType();
    void testHandleMorphElementRejectsNonIntegerElementId();
    void testHandleMorphElementRejectsUnknownElementId();
    void testHandleMorphElementRejectsNonGraphicElementItem();
    void testHandleMorphElementMorphsRealElement();

    void testHandleCommandRejectsUnknownCommand();
};
