// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestMCPValidator : public QObject
{
    Q_OBJECT

private slots:
    void testConstructorLoadsRealSchema();
    void testConstructorFailsOnMissingFile();
    void testConstructorFailsOnMalformedJson();
    void testConstructorFailsOnUnresolvableSchemaRef();

    void testValidateCommandFailsWhenSchemaNotLoaded();
    void testValidateCommandRejectsMissingMethodField();
    void testValidateCommandRejectsUnknownMethod();
    void testValidateCommandAcceptsValidCommand();
    void testValidateCommandRejectsSchemaViolation();
    void testValidateCommandQJsonOverloadDelegates();
    void testValidateCommandQJsonOverloadCatchesMalformedInput();
    void testValidateCommandCachesCompiledValidatorAcrossCalls();

    void testValidateResponseFailsWhenSchemaNotLoaded();
    void testValidateResponseRejectsBaseSchemaViolation();
    void testValidateResponseAcceptsBaseOnlyWhenNoExpectedCommand();
    void testValidateResponseUsesSpecificSchemaWhenAvailable();
    void testValidateResponseQJsonOverloadCatchesMalformedInput();
    void testValidateResponseRejectsMissingCommandResponseDefinition();

    void testFindCommandSchemaReturnsNullForUnknownCommand();
    void testFindResponseSchemaReturnsNullForUnknownCommand();
    void testFindCommandSchemaResolvesRef();
    void testFindResponseSchemaResolvesRef();

    void testQjsonToNlohmannRoundTrip();
    void testNlohmannToQJsonRoundTrip();
    void testNlohmannToQJsonReturnsEmptyOnUnparsableJson();
};
