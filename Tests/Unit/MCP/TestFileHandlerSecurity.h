// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Regression coverage for MCP/Server/Handlers/FileHandler.cpp hardening found in a
/// ninth-pass audit: export_image's PNG branch previously re-implemented (and never
/// bounded) CircuitExporter's now-fixed unbounded-buffer allocation, and its "padding"
/// parameter was accepted with no upper bound.
class TestFileHandlerSecurity : public QObject
{
    Q_OBJECT

private slots:
    void testExportImagePngClampsExtremeSceneDimensions();
    void testExportImageClampsExcessivePadding();
};
