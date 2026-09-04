// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Coverage for QuickICHandler's path-traversal/confinement guards, mirroring
/// Tests/Unit/MCP/TestICHandlerSecurity.cpp against the Quick-side handler.
class TestQuickICHandlerSecurity : public QObject
{
    Q_OBJECT

private slots:
    void testEmbedIcRejectsPathTraversalBlobName();
    void testInstantiateIcInlineRejectsPathTraversalBlobName();
    void testExtractIcConfinesFileNameToProjectDirectory();
    void testInstantiateIcLeavesNoElementOnLoadFailure();
};
