// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// CanvasSplitCommand::redo() adds exactly one new Node element and one new Connection. The
/// split-connection tests assert `elements().size()`/`connections().size()` each growing by
/// exactly 1, rather than a single combined item-count figure -- unlike Widgets'
/// `scene->items().size()`, which flattens the whole QGraphicsItem tree (a Node contributes
/// itself + its label + its 2 ports as 4 separate items) and has no CanvasItem-side equivalent.
class TestQuickConnectionHandler : public QObject
{
    Q_OBJECT

private slots:
    void testConnectElementsRejectsMissingParams();
    void testConnectElementsRejectsUnknownSourceOrTarget();
    void testConnectElementsRejectsMissingPortSpec();
    void testConnectElementsRejectsInvalidPortLabel();
    void testConnectElementsRejectsEmptyPortLabel();
    void testConnectElementsRejectsNegativePortIndex();
    void testConnectElementsRejectsOutOfRangePortIndex();
    void testConnectElementsResolvesPortByLabel();
    void testConnectElementsRejectsDisallowedConnection();
    void testConnectElementsCreatesRealConnection();

    void testDisconnectElementsRejectsMissingParams();
    void testDisconnectElementsRejectsUnknownElements();
    void testDisconnectElementsRejectsUnknownSourceElement();
    void testDisconnectElementsRejectsWhenNoConnectionExists();
    void testDisconnectElementsSkipsInProgressConnection();
    void testDisconnectElementsRemovesRealConnection();
    void testDisconnectElementsMatchesReversedSourceTarget();

    void testListConnectionsRejectsNoScene();
    void testListConnectionsReturnsEmptyWhenNoneExist();
    void testListConnectionsReturnsRealConnections();
    void testListConnectionsSkipsInProgressConnection();

    void testSplitConnectionRejectsMissingParams();
    void testSplitConnectionRejectsInvalidPortsOrCoordinates();
    void testSplitConnectionRejectsUnknownSourceOrTarget();
    void testSplitConnectionRejectsWhenNotFound();
    void testSplitConnectionSkipsInProgressConnection();
    void testSplitConnectionPerformsRealSplit();

    void testHandleCommandRejectsUnknownCommand();
};
