// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Real coverage for QuickICPreview (App/QuickShell/Chrome/QuickICPreview), the Quick-side
/// port of ICPreviewPopup's show/hide-delay state machine -- see that class's own doc comment.
/// This closes the gap TestICUnit.h's own doc comment named ("ICPreviewPopup's Quick equivalent
/// ... doesn't exist yet"): it now does, and this file covers it.
///
/// Scoped to the hover-hold behavior added alongside ICPreviewPopup.qml's HoverHandler
/// (cancelPendingHide()/resumePendingHide()) plus the minimal show/hide-delay plumbing needed to
/// exercise it meaningfully -- not a full line-for-line port of every ICPreviewPopup unit test
/// (showForIC()/executeShow()'s own null/degenerate-input guards are exercised indirectly via
/// CanvasItem::renderICPreviewImage()'s own tests already).
class TestQuickICPreview : public QObject
{
    Q_OBJECT

private slots:
    void testCancelPendingHideKeepsPopupVisibleAcrossHideDelay();
    void testResumePendingHideEventuallyHidesPopup();
};
