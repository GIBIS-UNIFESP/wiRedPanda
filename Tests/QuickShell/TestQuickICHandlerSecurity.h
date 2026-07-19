// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Phase 7c of the qtquick-rewrite plan: real coverage for QuickICHandler's path-traversal/
/// confinement guards, mirroring Tests/Unit/MCP/TestICHandlerSecurity.cpp against the
/// Quick-side handler. Security-relevant, prioritized early in Phase 7.
class TestQuickICHandlerSecurity : public QObject
{
    Q_OBJECT

private slots:
    void testEmbedIcRejectsPathTraversalBlobName();
    void testInstantiateIcInlineRejectsPathTraversalBlobName();
    void testExtractIcConfinesFileNameToProjectDirectory();
    void testInstantiateIcLeavesNoElementOnLoadFailure();
};
