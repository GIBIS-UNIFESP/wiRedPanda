// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestBaseHandler : public QObject
{
    Q_OBJECT

private slots:
    void testCreateSuccessResponseIncludesResultAndId();
    void testCreateSuccessResponseOmitsIdWhenNull();
    void testCreateErrorResponseIncludesCodeAndMessage();
    void testCreateErrorResponseOmitsIdWhenNull();

    void testValidateParametersAcceptsWhenAllPresent();
    void testValidateParametersRejectsWhenMissing();

    void testCurrentSceneNullWithNoMainWindow();
    void testCurrentSceneNullWithNoCurrentTab();
    void testCurrentSceneReturnsRealScene();

    void testValidatePositiveIntegerRejectsNonNumber();
    void testValidatePositiveIntegerRejectsZeroAndNegative();
    void testValidatePositiveIntegerAcceptsPositive();

    void testValidateNonNegativeIntegerRejectsNonNumber();
    void testValidateNonNegativeIntegerRejectsNegative();
    void testValidateNonNegativeIntegerAcceptsZeroAndPositive();

    void testValidateNonEmptyStringRejectsNonString();
    void testValidateNonEmptyStringRejectsEmpty();
    void testValidateNonEmptyStringAcceptsNonEmpty();

    void testValidateElementIdRejectsNonPositive();
    void testValidateElementIdRejectsNoScene();
    void testValidateElementIdRejectsNotFound();
    void testValidateElementIdAcceptsReal();

    void testValidateNumericRejectsNonNumber();
    void testValidateNumericRejectsNanAndInf();
    void testValidateNumericAcceptsFinite();

    void testValidatePortRangeRejectsNullElement();
    void testValidatePortRangeRejectsOutOfRange();
    void testValidatePortRangeAcceptsInRange();

    void testValidatedElementRejectsNonPositiveParam();
    void testValidatedElementRejectsMissingElement();
    void testValidatedElementRejectsNonGraphicElementItem();
    void testValidatedElementAcceptsRealElement();

    void testInputPortByLabelRejectsNullElement();
    void testInputPortByLabelRejectsNotFound();
    void testInputPortByLabelFindsRealPort();

    void testOutputPortByLabelRejectsNullElement();
    void testOutputPortByLabelRejectsNotFound();
    void testOutputPortByLabelFindsRealPort();

    void testAvailableInputPortsHandlesNullZeroAndNamed();
    void testAvailableOutputPortsHandlesNullZeroAndNamed();
};
