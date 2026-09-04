// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickICPreview.h"

#include "App/Element/IC.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickICPreview.h"

namespace {

/// A displayName() of "preview_ic" alone is enough for executeShow() to consider the popup
/// worth showing, even with no internal elements loaded to render an image from -- mirrors this
/// file's sibling test fixtures' minimal-IC pattern (e.g. TestQuickElementPalette.cpp's
/// placeEmbeddedPaletteTestIC()).
IC *placePreviewTestIC(CanvasItem &canvas, const QString &blobName)
{
    auto *ic = new IC();
    ic->setBlobName(blobName);
    canvas.receiveCommand(new CanvasAddItemsCommand({ic}, &canvas));
    return ic;
}

} // namespace

void TestQuickICPreview::testCancelPendingHideKeepsPopupVisibleAcrossHideDelay()
{
    // Mirrors ICPreviewPopup::enterEvent()'s cancelHide() call, wired to ICPreviewPopup.qml's
    // new HoverHandler: moving the cursor from the IC onto the popup itself must keep it open
    // past the point the hide timer would otherwise have fired.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *ic = placePreviewTestIC(canvas, "preview_ic");

    QuickICPreview presenter;
    presenter.setCanvas(&canvas);

    emit canvas.icPreviewRequested(ic, QPoint(10, 10));
    QVERIFY2(QTest::qWaitFor([&] { return presenter.isVisible(); }, 2000), "popup must become visible after the show delay");

    emit canvas.icPreviewHideRequested();
    presenter.cancelPendingHide();

    // Longer than the real 300ms hide delay -- if cancelPendingHide() didn't actually stop the
    // timer, the popup would have hidden itself well within this wait.
    QTest::qWait(500);
    QVERIFY2(presenter.isVisible(), "cancelPendingHide() must keep the popup open past its hide delay");
}

void TestQuickICPreview::testResumePendingHideEventuallyHidesPopup()
{
    // Mirrors ICPreviewPopup::leaveEvent()'s scheduleHide() call: once the cursor leaves the
    // popup again, the auto-hide must actually resume rather than staying cancelled forever.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *ic = placePreviewTestIC(canvas, "preview_ic");

    QuickICPreview presenter;
    presenter.setCanvas(&canvas);

    emit canvas.icPreviewRequested(ic, QPoint(10, 10));
    QVERIFY2(QTest::qWaitFor([&] { return presenter.isVisible(); }, 2000), "popup must become visible after the show delay");

    emit canvas.icPreviewHideRequested();
    presenter.cancelPendingHide();
    QVERIFY2(presenter.isVisible(), "sanity check: still visible right after cancelling");

    presenter.resumePendingHide();
    QVERIFY2(QTest::qWaitFor([&] { return !presenter.isVisible(); }, 2000), "resumePendingHide() must let the popup hide again");
}
