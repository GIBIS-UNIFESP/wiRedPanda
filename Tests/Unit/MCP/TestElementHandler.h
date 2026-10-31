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
    void testSetElementPropertiesAnswersAPushThatDropsWithAnError();
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
    void testHandleListElementsReportsAWirelessNodesOutline();

    void testHandleMoveElementRejectsMissingParams();
    void testHandleMoveElementRejectsInvalidCoordinates();
    void testHandleMoveElementRejectsNonNumericY();
    void testHandleMoveElementRejectsUnknownElement();
    void testHandleMoveElementMovesRealElementSnappedToGrid();

    void testHandleSetElementPropertiesRejectsMissingElementId();
    void testHandleSetElementPropertiesRejectsUnknownElement();
    void testHandleSetElementPropertiesRejectsPortSizeParams();
    /// get_output_value's `value` is a bool, so unknown and error are indistinguishable from
    /// logic low -- while create_waveform reports the same state as a raw -1, leaving one server
    /// describing the same signal two different ways. An explicit `status` field resolves it.
    void testGetOutputValueReportsFourStateStatus();
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
    void testHandleSetElementPropertiesRxModeSeversInputConnection();
    void testHandleSetElementPropertiesAcceptsWirelessModeNoneForRealNode();
    void testHandleSetElementPropertiesRejectsInvalidWirelessMode();
    void testHandleSetElementPropertiesRefusesWirelessModeForNonNode();
    void testHandleSetElementPropertiesPushesUndoableCommand();

    // A request changes nothing unless every value passes; then it is one undo step, its values read back
    void testSetElementPropertiesRefusesAPropertyTheTypeLacks();
    void testSetElementPropertiesRefusesAValueOfTheWrongKind();
    void testSetElementPropertiesRefusesAnInvalidValueBeforeChangingAnything();
    void testSetElementPropertiesRefusesAFrequencyTheClockCannotTake();
    void testSetElementPropertiesRefusesAColourTheEditorDoesNotOffer();
    void testSetElementPropertiesRefusesATriggerTheEditorDoesNotTake();
    void testSetElementPropertiesRefusesAnAppearanceIndexAlone();
    void testSetElementPropertiesRefusesAnAppearanceIndexOutOfRange();
    void testSetElementPropertiesRefusesAPictureThatDoesNotLoad();
    void testSetElementPropertiesRefusesASecondTxOnALabel();
    void testSetElementPropertiesPushesNothingWhenNothingChanges();
    void testSetElementPropertiesIsOneUndoStep();
    void testSetElementPropertiesReadsBackWhatTheElementTook();

    void testHandleSetInputValueRejectsMissingParams();
    void testHandleSetInputValueRejectsUnknownElement();
    void testHandleSetInputValueRejectsNonInputElement();
    void testHandleSetInputValueSetsRealValue();

    void testHandleGetOutputValueRejectsMissingParams();
    void testHandleGetOutputValueRejectsUnknownElement();
    void testHandleGetOutputValueReturnsInputElementValue();
    void testHandleGetOutputValueOfAnInputReportsEachPortsStatus();
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
    void testHandleMorphElementRejectsNonElementItem();
    void testHandleMorphElementMorphsRealElement();

    void testHandleCommandRejectsUnknownCommand();

    void testResponsesMatchGolden();
};
