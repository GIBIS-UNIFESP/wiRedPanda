// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestConnectionManager : public QObject
{
    Q_OBJECT

private slots:

    void testConnectionCreation();
    void testConnectionValidation();
    void testConnectionDeletion();
    void testMultiPortConnection();
    void testHoverShowsConnectedPortLabels();
    void testHoverSkipsUnnamedPortLabels();
    void testHoverFanOutShowsAllPeerLabels();
    void testHoverLabelExcludedFromHitTesting();

    // PortHoverLabel's remaining geometry/paint surface, not exercised via ConnectionManager
    void testHoverLabelConstructsForTopAndBottomSides();
    void testHoverLabelBoundingRectInflatesBoundsByOnePixel();
    void testHoverLabelPaintDrawsVisibleChip();
    void testHoverReleaseClearsPeerHighlightAfterWireDeleted();

    // sideFor()'s vertical-bias branch for a rotated element's port
    void testShowHoverLabelsBiasesLabelForRotatedElementPort();
    // hoverPort()/decodePort()'s stale-index guard
    void testHoverPortReturnsNullForOutOfRangeStaleIndex();

    // startFromInput()/tryComplete()/updateEditedEnd()'s input-anchored wire path -- only the
    // output-anchored direction was exercised before
    void testStartFromInputBeginsWireAnchoredAtEnd();
    void testTryCompleteFromInputCompletesAtOutputPort();
    void testUpdateEditedEndForInputAnchoredWireMovesStart();

    void testTryCompleteWithNoPortUnderCursorIsNoOp();
    void testTryCompleteAtWrongPortTypeIsNoOp();
    void testTryCompleteRejectedConnectionDeletesWireAndShowsMessage();
    void testCancelDeletesInProgressWire();

    // detach(): re-routing an existing wire, previously untested at all
    void testDetachRewiresFromSameOutput();
    void testDetachWithNoWireIsNoOp();

    void testUpdateEditedEndWithNoConnectionIsNoOp();
    void testUpdateEditedEndWithBothPortsLostCleansUp();

    void testUpdateHoverShowsForbiddenCursorForSamePolarityPort();
    void testClearHoverReleasesHoveredPort();

    void testConnectionRejectionReasonForAlreadyConnectedPorts();

    void testShowHoverLabelsWithNullPortIsNoOp();
    void testConnectedPeersWithNullPortReturnsEmpty();
};
