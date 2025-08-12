// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestQNEPortAdvanced : public QObject
{
    Q_OBJECT

private slots:
    void testIsConnectedMethod();
    void testCurrentBrushGetterSetter();
    void testHoverEnterLeaveVisualFeedback();
    void testPortConstructorEdgeCases();
    void testInputPortSetStatusBranches();
    void testOutputPortUpdateThemeBranches();
    void testPortFlagsAndPtr();
    void testConnectionManagement();
    void testPortValidationLogic();
    void testPortIndexAndNaming();
    void testDefaultStatusHandling();
    void testRequiredPortBehavior();
    void testPortWithNullGraphicElement();
    void testPortDestructorCleanup();
};