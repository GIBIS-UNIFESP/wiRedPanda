// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestICUnit : public QObject
{
    Q_OBJECT

private slots:

    void testICLoadFromFile();
    void testICPortLabelResolution();
    void testICNestedSaveLoad();
    void testICInvalidFile();
    void testICPreviewPopupRespectsDisabledSetting();
    void testPreviewPopupClampsToScreen();

    // GraphicElement::mouseDoubleClickEvent() (added for #7/#38 inline rename) must never
    // shadow IC's own double-click override -- ordinary virtual dispatch should guarantee
    // this, but it's asserted directly rather than assumed.
    void testDoubleClickOpensSubCircuitNotInlineEditor();

    // ICSimulation's empty-IC and disconnected-input guards
    void testUnloadedIcSimulationMethodsAreNoOps();
    void testLoadedIcWithDisconnectedInputIsUnknown();

    // ICRenderer's non-finite/degenerate-geometry defense-in-depth guards
    void testGeneratePixmapWithNonFiniteBoundsIsNoOp();
    void testGeneratePreviewPixmapWithNonFiniteBoundsIsNoOp();
    void testGeneratePreviewPixmapWithDegenerateAspectRatioIsEmpty();

    // ICPreviewPopup::executeShow() and showForIC()'s remaining branches
    void testShowForIcNullIcIsNoOp();
    void testShowForIcImmediateWhenAlreadyVisible();
    void testExecuteShowWithNullPendingIcIsNoOp();
    void testExecuteShowWithEmptyTitleAndNullPreviewHides();
    void testExecuteShowWithTitleAndPreviewShowsBoth();
    void testExecuteShowWithTitleButNullPreviewHidesImageOnly();
    void testEnterEventCancelsHide();
    void testLeaveEventSchedulesHide();

    // IC::load()'s stale-portMap-key eviction (fuzz-hardening: reload shrinks port count)
    void testReloadWithFewerPortsEvictsStalePortMapKeys();

    // hoverEnterEvent()/hoverMoveEvent()'s "cursor over a port" branch
    void testHoverOverPortRequestsPreviewHide();

    // displayName() for an embedded (blob-backed) IC
    void testDisplayNameForEmbeddedIc();
};
