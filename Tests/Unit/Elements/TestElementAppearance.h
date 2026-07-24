// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestElementAppearance : public QObject
{
    Q_OBJECT

private slots:
    // setAppearanceAt() bounds and fallback behavior
    void testSetAppearanceAtOutOfBoundsIsNoOp();
    void testSetAppearanceAtEmptyFileNameFallsBackToDefault();
    void testSetAppearanceAtCustomFileName();

    // externalFiles()
    void testExternalFilesReflectsCustomAppearance();

    // Rendering a raster (non-SVG) appearance
    void testRenderRasterAppearance();

    // setPixmap() failure paths
    void testSetPixmapNonExistentFileThrows();
    void testSetPixmapUnreadableFileThrows();

    // orientSvgTextNodes()/orientedSvgPixmap()/sharedSvgRenderer() failure paths
    void testRotatedMalformedSvgFallsBackToBasePixmap();
    void testRotatedMalformedSvgFallsBackToBasePixmapContent();
};
