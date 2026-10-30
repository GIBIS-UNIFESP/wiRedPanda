// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/// An element's parameters: each type's schema, the store that holds them and how a new value is taken.
class TestParameters : public QObject
{
    Q_OBJECT

private slots:
    // --- The schemas ---
    void testEachTypeHasTheParametersOfItsSchema_data();
    void testEachTypeHasTheParametersOfItsSchema();

    // --- The store ---
    void testTheStoreHoldsItsSchemasDefaults();
    void testTheStoreRefusesWhatItsTypeDoesNotHave();

    // --- Setting a parameter ---
    void testSetParamTakesAValueAsItsSpecDoes();
    void testParameterChangedFollowsEachChangeOnly();
};
