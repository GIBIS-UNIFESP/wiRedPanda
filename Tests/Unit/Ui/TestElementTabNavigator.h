// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestElementTabNavigator : public QObject
{
    Q_OBJECT

private slots:
    // Tab/Backtab cycle elements in reading order; these lock that ordering (the B2 bug was a
    // column-major ordering that contradicted the documented row-major reading order).
    void testReadingOrderIsRowMajor();
    void testReadingOrderTieBreaksLeftToRight();
    void testReadingOrderEmptyAndSingle();

    // Integration-level coverage of the same ordering through the real eventFilter() path
    // (focus movement across widgets), not just the static readingOrder() helper above.
    void testTabNavigation();
    void testBacktabNavigation();
    void testTabWrapAround();

    void testEventFilterIgnoresNonSingleSelection();
    void testEventFilterIgnoresKeyPressWithNoScene();
    void testEventFilterIgnoresFocusedElementNotInVisibleElements();
    void testTabRevertsToOriginalElementWhenNoOtherFieldIsEnabled();
};
