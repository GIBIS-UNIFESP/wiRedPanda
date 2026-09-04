// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickICPreview: QML-facing presenter driving the IC hover-preview popup.
 */

#pragma once

#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QQmlEngine>
#include <QString>
#include <QTimer>

class CanvasItem;
class GraphicElement;

/**
 * \class QuickICPreview
 * \brief Copy-and-adapted, Widgets-free port of App/Element/ICPreviewPopup's show/hide-delay
 * state machine for the Quick chrome.
 *
 * \details Drives ICPreviewPopup.qml's visible/title/imageUrl/screenPos bindings the way
 * ICPreviewPopup itself drives a QWidget's show()/move()/hide() calls. setCanvas() connects to
 * CanvasItem's icPreviewRequested()/icPreviewMoved()/icPreviewHideRequested()/
 * icPreviewCancelRequested() signals -- CanvasItem emits these directly from
 * hoverMoveEvent()/hoverLeaveEvent()/mouseDoubleClickEvent(), the same call sites that already
 * (redundantly, but harmlessly) call IC's own real previewRequested()/previewMoved()/
 * previewHideRequested() signals, which nothing in production's SceneUiBinder-based UI needs
 * this class to touch.
 *
 * \details The show/hide-delay/already-pending-for-this-IC state machine
 * (showForIC()/executeShow()/scheduleHide()/cancelHide()/updatePendingPos()/isShowActiveFor())
 * is a direct, line-for-line port of ICPreviewPopup's own logic, not a simplified
 * reinvention -- see each private method's doc comment for the production method it mirrors.
 * The one real difference: instead of rendering IC::previewPixmap() (always empty here --
 * ICRenderer::generatePreviewPixmap() guards on a real QApplication existing, which
 * wiredpanda never constructs), executeShow() calls CanvasItem::renderICPreviewImage()
 * and encodes the result as a data: URL for imageUrl -- see that method's own doc comment for
 * why (internalElements() instead of the pre-substitution designed circuit) and why a data:
 * URL (simplest way to hand a dynamically-generated image to a QML Image.source without a
 * QQuickImageProvider request-id/lookup scheme for what is otherwise a single popup).
 */
class QuickICPreview : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("QuickICPreview is only ever exposed via AppController.icPreview")

    Q_PROPERTY(bool visible READ isVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QString title READ title NOTIFY refreshed FINAL)
    Q_PROPERTY(QString imageUrl READ imageUrl NOTIFY refreshed FINAL)
    Q_PROPERTY(QPoint screenPos READ screenPos NOTIFY refreshed FINAL)

public:
    explicit QuickICPreview(QObject *parent = nullptr);

    /// Binds this presenter to \a canvas's icPreview* signals; passing nullptr unbinds. Hides
    /// any popup left pending/visible from the previously-bound canvas. Mirrors
    /// ICPreviewPopup's implicit per-Scene lifetime (production constructs one per MainWindow,
    /// this Quick chrome has exactly one shared instance rebound per tab switch instead).
    void setCanvas(CanvasItem *canvas);

    [[nodiscard]] bool isVisible() const { return m_visible; }
    [[nodiscard]] QString title() const { return m_title; }
    [[nodiscard]] QString imageUrl() const { return m_imageUrl; }
    [[nodiscard]] QPoint screenPos() const { return m_screenPos; }

    /// Cancels the pending auto-hide -- called from ICPreviewPopup.qml's own HoverHandler when
    /// the cursor moves from the IC onto the popup itself, mirroring ICPreviewPopup::
    /// enterEvent()'s identical cancelHide() call (so the popup can stay open indefinitely while
    /// the user reads it, instead of vanishing under the cursor on the original hide timer).
    Q_INVOKABLE void cancelPendingHide() { cancelHide(); }
    /// Restarts the auto-hide once the cursor leaves the popup again -- mirrors
    /// ICPreviewPopup::leaveEvent()'s identical scheduleHide() call.
    Q_INVOKABLE void resumePendingHide() { scheduleHide(); }

signals:
    void refreshed();

private:
    void onPreviewRequested(GraphicElement *ic, QPoint screenPos);
    void onPreviewMoved(GraphicElement *ic, QPoint screenPos);
    void onPreviewHideRequested();
    void onPreviewCancelRequested(GraphicElement *ic);

    /// Starts (or immediately executes, if already visible) the show-delay timer for \a ic at
    /// \a screenPos. Mirrors ICPreviewPopup::showForIC().
    void showForIC(GraphicElement *ic, const QPoint &screenPos);
    /// Renders and displays the popup for m_pendingIc. Mirrors ICPreviewPopup::executeShow().
    void executeShow();
    /// Starts the hide-delay timer, cancelling any pending show. Mirrors
    /// ICPreviewPopup::scheduleHide().
    void scheduleHide();
    /// Stops both timers without hiding anything already visible. Mirrors
    /// ICPreviewPopup::cancelHide().
    void cancelHide();
    /// Updates the pending screen position for a still-hidden (not yet shown) preview. Mirrors
    /// ICPreviewPopup::updatePendingPos().
    void updatePendingPos(const QPoint &screenPos);
    /// Returns true if \a ic is the one currently visible or about to become visible. Mirrors
    /// ICPreviewPopup::isShowActiveFor().
    [[nodiscard]] bool isShowActiveFor(const GraphicElement *ic) const;
    /// Hides the popup if visible. Mirrors ICPreviewPopup::hidePopup().
    void hidePopup();

    QPointer<CanvasItem> m_canvas;
    QTimer m_showTimer;
    QTimer m_hideTimer;
    /// Auto-nulled if the pending IC is deleted while a timer is running -- mirrors
    /// ICPreviewPopup::m_pendingIC's own QPointer<IC> guard.
    QPointer<GraphicElement> m_pendingIc;
    QPoint m_pendingScreenPos;

    bool m_visible = false;
    QString m_title;
    QString m_imageUrl;
    QPoint m_screenPos;
};
