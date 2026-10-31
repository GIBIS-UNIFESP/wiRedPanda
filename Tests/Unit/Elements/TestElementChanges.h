// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/// The change funnel: every change of an element is reported once, with what it changed, and the bounds Qt has follow.
class TestElementChanges : public QObject
{
    Q_OBJECT

private slots:
    // --- Coverage ---
    void testEveryChangeOfEveryTypeIsReported();

    // --- Reporting ---
    void testEachChangeIsReportedOnce();
    void testBoundsThatMoveAreReportedAsBody();
    void testThePictureAloneIsNotReported();
    void testAChangeThatThrowsIsStillReported();

    // --- Bounds ---
    void testElementsOutOfASceneMeasureTheirBoundsLive();
};
