// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Phase 7c of the qtquick-rewrite plan: real coverage for QuickElementHandler, mirroring
/// Tests/Unit/MCP/TestElementHandler.cpp's two regression tests against the Quick-side handler
/// (which had zero permanent test of its own until now, despite being a mechanical port of
/// ElementHandler.cpp -- see the plan's Phase 7 section).
class TestQuickElementHandler : public QObject
{
    Q_OBJECT

private slots:
    void testSetElementPropertiesAcceptsNegativeClockDelay();
    void testSetElementPropertiesRejectsClockDelayOutOfRange();
};
