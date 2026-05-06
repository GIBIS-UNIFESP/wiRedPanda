// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Frameless popup widget that shows a preview of an IC's internal circuit.
 */

#pragma once

#include <QLabel>
#include <QPointer>
#include <QTimer>
#include <QWidget>

class IC;

/**
 * \class ICPreviewPopup
 * \brief Frameless tooltip-like widget showing a rendered preview of an IC's sub-circuit.
 *
 * \details Displayed on hover over an IC element.  The preview is rendered from
 * the IC's internal elements and connections into a QPixmap at reduced scale.
 * The popup is dismissed with a short delay (300ms) when the mouse leaves either
 * the IC or the popup itself.
 */
class ICPreviewPopup : public QWidget
{
    Q_OBJECT

public:
    /// Maximum preview dimensions in pixels.
    static constexpr int MaxWidth = 500;
    static constexpr int MaxHeight = 350;

    /// Maximum number of internal elements before we skip generating a preview.
    /// Empirically chosen: circuits above this size render in >16 ms on a
    /// mid-range laptop, making the 1-second hover delay feel unresponsive.
    static constexpr int MaxElementCount = 500;

    /// Constructs the popup as a frameless, non-activating child of \a parent.
    explicit ICPreviewPopup(QWidget *parent = nullptr);

    /// Schedules the popup to appear near \a screenPos with the preview of \a ic
    /// after a 1-second hover delay.  If the popup is already visible (e.g. the
    /// cursor moved quickly from another IC), the update is applied immediately.
    void showForIC(IC *ic, const QPoint &screenPos);

    /// Starts a delayed hide (300ms).  If showForIC() is called before the
    /// timer fires, the hide is cancelled.
    void scheduleHide();

    /// Cancels a pending show/hide.
    void cancelHide();

    /// Updates the position the popup will appear at when the show timer
    /// fires.  No effect if the popup is already visible.  Use this on
    /// hover-move so the popup tracks the cursor during the show delay.
    void updatePendingPos(const QPoint &screenPos);

    /// Returns the IC currently pending display (may be null).
    IC *pendingIC() const { return m_pendingIC; }

protected:
    /// \reimp Cancels a scheduled hide when the cursor enters the popup itself.
    void enterEvent(QEnterEvent *event) override;

    /// \reimp Schedules a hide when the cursor leaves the popup.
    void leaveEvent(QEvent *event) override;

private:
    void executeShow();

    QLabel *m_imageLabel;
    QTimer m_hideTimer;
    QTimer m_showTimer;
    QPointer<IC> m_pendingIC;   ///< Auto-nulled if the IC is deleted while a timer is pending.
    QPoint m_pendingPos;
};

