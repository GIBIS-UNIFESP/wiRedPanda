// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestCircuitExporter : public QObject
{
    Q_OBJECT

private slots:

    void testExporterCreation();

    // Regression: D19 — renderToImage must throw on save failure
    void testRenderToImageThrowsOnInvalidPath();

    // Regression: QPixmap(size) is uninitialized until painted over; Scene only paints its
    // background opaquely once Scene::updateTheme() has run. A WorkSpace that never called
    // it (this test, deliberately) must still produce a fully-painted (not garbage) image.
    void testRenderToImagePaddingIsTransparentNotGarbage();

    // Regression: an element at an extreme-but-finite position (the only check on load
    // rejects non-finite, not large, coordinates) must not allocate a proportionally-sized
    // pixel buffer — output must stay within CircuitExporter::kMaxImageDimension.
    void testRenderToImageClampsExtremeSceneDimensions();

    // Regression: Scene::drawBackground()'s grid-dot loop had no bound on point count and
    // operates on the *unscaled* exposed rect — an extreme-but-finite element position made
    // it try to build a point list with billions of entries (confirmed via debugger: ~1e10
    // iterations in progress at the 300s watchdog timeout), hanging well before either
    // export function's own size cap ever mattered. renderToPdf shares the exact same
    // vulnerable code path (unlike the pixel-buffer issue, which is renderToImage-only,
    // since renderToPdf's page size is fixed) and must complete promptly too.
    void testRenderToPdfHandlesExtremeSceneDimensionsPromptly();

    // Regression: renderToImage() was refactored into a thin wrapper around
    // CircuitExporter::renderScaledImage() so the MCP export_image handler could reuse the
    // same bounding logic instead of re-implementing (and never bounding) it. Pin the
    // extracted function's own contract directly, independent of the file round-trip above.
    void testRenderScaledImageClampsExtremeDimensionsDirectly();

    void testRenderToPdfThrowsOnInvalidPath();
    void testRenderScaledImageThrowsOnEmptyPaddedRect();
};
