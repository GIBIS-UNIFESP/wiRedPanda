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
    /// A property write that cannot take effect must not report success. Skipping one silently
    /// leaves success=true with the property simply absent from new_properties --
    /// indistinguishable from a real write unless the caller diffs the response against its own
    /// request.
    void testHandleSetElementPropertiesRejectsUnsupportedProperty();
    /// The sharpest case: ICs are flattened and their internal primitives carry the delays, so
    /// propagation_delay is meaningless on the container.
    void testHandleSetElementPropertiesRejectsPropagationDelayOnIC();
    /// A rejected call must leave the element completely untouched -- the capability check
    /// runs before any mutation, so a supported property in the same request is NOT applied.
    void testHandleSetElementPropertiesRejectionAppliesNothing();
    /// appearance_index needs the capability pre-check too: it is read only inside the
    /// "appearance" branch, so passing it alone can never take effect -- exactly the silent
    /// success the check exists to prevent.
    void testHandleSetElementPropertiesRejectsAppearanceIndexWithoutAppearance();
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
