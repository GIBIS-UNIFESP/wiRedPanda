// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Regression coverage for MCP/Server/Handlers/ElementHandler.cpp's set_element_properties
/// "delay" validation, found stale against Clock::setDelay()'s real [-1, 1] range during a
/// deep review of commits since 5.1.3.
class TestElementHandler : public QObject
{
    Q_OBJECT

private slots:
    void testSetElementPropertiesAcceptsNegativeClockDelay();
    void testSetElementPropertiesRejectsClockDelayOutOfRange();
};
