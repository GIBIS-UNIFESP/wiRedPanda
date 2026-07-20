// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestConnectionHandler : public QObject
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
