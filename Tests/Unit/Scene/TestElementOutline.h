// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/// What a body draws beyond itself (a wireless node's indicator) is drawn, but is no part of the element's outline,
/// which is what is hit, selected and aligned, and what the editing tools frame.
class TestElementOutline : public QObject
{
    Q_OBJECT

private slots:
    void testADecorationIsDrawnButIsNotTheOutline();
    void testAPressOnADecorationMissesTheElement();
    void testARubberBandOverADecorationAloneSelectsNothing();
    void testAlignLeftUsesTheOutline();
    void testDistributeHorizontallyUsesTheOutline();
    void testTheCloneDragPictureFramesTheOutline();
    void testTheLabelEditorStartsOnTheOutline();
    void testZoomToFitFramesTheOutline();
};
