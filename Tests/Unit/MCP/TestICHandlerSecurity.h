// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Regression coverage for MCP/Server/Handlers/ICHandler.cpp hardening found in an
/// eighth-pass audit: path traversal via unsanitized blob_name/file_name in embed_ic/
/// extract_ic/instantiate_ic(inline), and a leaked IC on a failing instantiate_ic load.
class TestICHandlerSecurity : public QObject
{
    Q_OBJECT

private slots:
    void testEmbedIcRejectsPathTraversalBlobName();
    void testInstantiateIcInlineRejectsPathTraversalBlobName();
    void testExtractIcConfinesFileNameToProjectDirectory();
    void testInstantiateIcLeavesNoElementOnLoadFailure();
};
