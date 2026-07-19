// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Phase 7c of the qtquick-rewrite plan: real coverage for QuickFileHandler's export_image
/// dimension/padding clamps, mirroring Tests/Unit/MCP/TestFileHandlerSecurity.cpp against the
/// Quick-side handler.
class TestQuickFileHandlerSecurity : public QObject
{
    Q_OBJECT

private slots:
    void testExportImagePngClampsExtremeSceneDimensions();
    void testExportImageClampsExcessivePadding();
};
