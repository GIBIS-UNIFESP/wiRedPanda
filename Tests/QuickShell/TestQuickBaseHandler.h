// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Exercises QuickBaseHandler (MCP/Server/Handlers/QuickBaseHandler.h, a thin BaseHandler
/// subclass constructed with QuickAppController* instead of MainWindow*) via a local
/// StubHandler, mirroring Tests/Unit/MCP/TestBaseHandler.h/.cpp's technique. Everything except
/// currentScene()/validateElementId()/validatedElement() is inherited unmodified from
/// BaseHandler, so most of these tests are really exercising BaseHandler's own shared logic,
/// just through a QuickBaseHandler instance instead of a raw BaseHandler one.
/// `MainWindow window;` (auto-creates a tab; "no scene" tests close it) becomes
/// `QuickAppController controller;` with `newTab()` simply never called for those same cases --
/// QuickAppController starts with zero tabs.
class TestQuickBaseHandler : public QObject
{
    Q_OBJECT

private slots:
    void testCreateSuccessResponseIncludesResultAndId();
    void testCreateSuccessResponseOmitsIdWhenNull();
    void testCreateErrorResponseIncludesCodeAndMessage();
    void testCreateErrorResponseOmitsIdWhenNull();

    void testValidateParametersAcceptsWhenAllPresent();
    void testValidateParametersRejectsWhenMissing();

    void testCurrentCanvasNullWithNoAppController();
    void testCurrentCanvasNullWithNoCurrentTab();
    void testCurrentCanvasReturnsRealCanvas();

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
