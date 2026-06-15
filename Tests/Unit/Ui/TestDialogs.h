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

    // ============================================================
    // LengthDialog Tests (5 tests)
    // ============================================================
    void testLengthDialogConstructor();
    void testLengthDialogMinLength();
    void testLengthDialogMaxLength();
    void testLengthDialogRange();
    void testLengthDialogReturnValue();

    // ============================================================
    // ElementEditor — Wireless Tx duplicate-label rejection (5 tests)
    // ============================================================
    void testElementEditorRejectsDuplicateTxLabel();
    void testElementEditorAllowsUniqueTxLabel();
    void testElementEditorRejectsModeChangeToDuplicateTx();
    void testElementEditorAllowsSelfRelabelTx();
    void testElementEditorAllowsRxWithDuplicateLabel();
};
