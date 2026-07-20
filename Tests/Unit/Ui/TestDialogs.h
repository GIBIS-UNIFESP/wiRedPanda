// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestDialogs : public QObject
{
    Q_OBJECT

private slots:

    // ============================================================
    // ClockDialog Tests (5 tests)
    // ============================================================
    void testClockDialogConstructor();
    void testClockDialogMinPeriod();
    void testClockDialogMaxPeriod();
    void testClockDialogRange();
    void testClockDialogPeriodReturnValue();

    // Regression seam: every production caller stack-allocates this modal dialog (run via
    // exec() then destructed at scope exit), which only ever invokes the base-object
    // destructor ABI entry point -- the separate "deleting destructor" entry point (used by
    // `delete` through a pointer) is a distinct compiled function that stays uncovered
    // without a heap-allocated instance.
    void testClockDialogHeapAllocationDeletesCleanly();

    // ============================================================
    // LengthDialog Tests (5 tests)
    // ============================================================
    void testLengthDialogConstructor();
    void testLengthDialogMinLength();
    void testLengthDialogMaxLength();
    void testLengthDialogRange();
    void testLengthDialogReturnValue();

    // See testClockDialogHeapAllocationDeletesCleanly() -- same reasoning, for LengthDialog.
    void testLengthDialogHeapAllocationDeletesCleanly();

    // Regression: Qt Designer's "Dialog" placeholder title and bare, hand-typed range
    // numbers no longer ship — the title is real and the range labels are derived from
    // the slider's actual bounds so they can't drift from it.
    void testLengthDialogTitleAndRangeLabels();

    // ============================================================
    // ElementEditor — Wireless Tx duplicate-label rejection (5 tests)
    // ============================================================
    void testElementEditorRejectsDuplicateTxLabel();
    void testElementEditorAllowsUniqueTxLabel();
    void testElementEditorRejectsModeChangeToDuplicateTx();
    void testElementEditorAllowsSelfRelabelTx();
    void testElementEditorAllowsRxWithDuplicateLabel();
};
