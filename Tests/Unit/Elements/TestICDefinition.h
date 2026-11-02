// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestICDefinition : public QObject
{
    Q_OBJECT

private slots:

    void init();
    void cleanup();

    void testOneFileAndItsBytesShareOneDefinition();
    void testSameBytesAtAnotherPathParseAgain();
    void testOtherBytesAtOnePathParseAgain();
    void testDefinitionHoldsTheBytesItParsed();
    void testChargeCountsTheBlobsItHolds();
    void testReleasedDefinitionIsKeptAndReused();
    void testInUseDefinitionIsNeverEvicted();
    void testReleasedDefinitionIsChargedItsSize();
    void testLeastRecentlyReleasedGoesFirst();
    void testFailedParseKeepsNothing();
    void testLoadedICHoldsItsDefinition();
    void testFileCacheSuppliesTheBytes();
    void testSceneRemovalDropsTheFileCache();

private:
    /// The budget the cache had before a test, restored after it.
    qsizetype m_budget = 0;
};
