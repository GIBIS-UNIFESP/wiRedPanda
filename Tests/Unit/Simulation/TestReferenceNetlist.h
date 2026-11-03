// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/**
 * \brief Checks the reference's reading of the wiring (plan 5, §4.4): junctions, transmitters and receivers followed to
 * their drivers, defaults, nets with two drivers, alias loops, and an answer that does not depend on element order.
 */
class TestReferenceNetlist : public QObject
{
    Q_OBJECT

private slots:
    void testJunctionChainReadsTheSource();
    void testUnconnectedInputReadsTheDefault();
    void testTwoWiresIntoAnInputReadError();
    void testReceiverReadsItsTransmitter();
    void testDuplicatedChannelReadsError();
    void testWiredReceiverReadsError();
    void testAliasLoopReadsUnknown();
    void testOrderDoesNotChangeTheDrivers();
};
