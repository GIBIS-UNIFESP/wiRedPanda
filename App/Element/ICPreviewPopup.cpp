// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICPreviewPopup.h"

#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>

#include "App/Element/IC.h"

ICPreviewPopup::ICPreviewPopup(QWidget *parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);   // Prevent solid black fill on Windows
    setAttribute(Qt::WA_ShowWithoutActivating);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    m_imageLabel = new QLabel(this);
    m_imageLabel->setObjectName(QStringLiteral("preview"));
    m_imageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_imageLabel);

    // Outer popup chrome + an inset frame around the preview pixmap so the
    // rendered circuit reads as a framed image rather than floating content.
    // The QLabel selector is scoped by objectName so future labels added to
    // the popup don't silently inherit the inset frame styling.
    setStyleSheet(
        "ICPreviewPopup {"
        "  background-color: rgba(30, 30, 30, 230);"
        "  border: 1px solid rgba(120, 120, 120, 180);"
        "  border-radius: 6px;"
        "}"
        "QLabel#preview {"
        "  border: 1px solid rgba(170, 170, 170, 200);"
        "  border-radius: 3px;"
        "  padding: 6px;"
        "  background-color: rgba(15, 15, 15, 200);"
        "}"
    );

    m_hideTimer.setSingleShot(true);
    m_hideTimer.setInterval(300);
    connect(&m_hideTimer, &QTimer::timeout, this, &QWidget::hide);

    m_showTimer.setSingleShot(true);
    m_showTimer.setInterval(1000);
    connect(&m_showTimer, &QTimer::timeout, this, &ICPreviewPopup::executeShow);
}

void ICPreviewPopup::showForIC(IC *ic, const QPoint &screenPos)
{
    cancelHide();

    if (!ic) {
        return;
    }

    m_pendingIC = ic;
    m_pendingPos = screenPos;

    if (isVisible()) {
        // If the popup is already visible (e.g., moved quickly from another IC),
        // update it immediately without a delay.
        executeShow();
    } else {
        m_showTimer.start();
    }
}

void ICPreviewPopup::executeShow()
{
    if (!m_pendingIC) {
        return;
    }

    // The pixmap is the single source of truth: it's null when the IC was
    // empty or oversized at load time, or when generation otherwise failed.
    const QPixmap &preview = m_pendingIC->previewPixmap();
    if (preview.isNull()) {
        hide();
        return;
    }

    m_imageLabel->setPixmap(preview);
    adjustSize();

    // Position slightly offset from the cursor, then clamp to the available
    // screen geometry so the popup never extends off-screen.
    QPoint pos = m_pendingPos + QPoint(16, 16);
    if (const auto *screen = QGuiApplication::screenAt(pos)) {
        const QRect avail = screen->availableGeometry();
        pos.setX(qMin(pos.x(), avail.right()  - width()));
        pos.setY(qMin(pos.y(), avail.bottom() - height()));
    }
    move(pos);
    show();
}

void ICPreviewPopup::scheduleHide()
{
    m_showTimer.stop();
    m_hideTimer.start();
}

void ICPreviewPopup::cancelHide()
{
    m_hideTimer.stop();
    m_showTimer.stop();
}

void ICPreviewPopup::updatePendingPos(const QPoint &screenPos)
{
    if (!isVisible()) {
        m_pendingPos = screenPos;
    }
}

void ICPreviewPopup::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event)
    cancelHide();
}

void ICPreviewPopup::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    scheduleHide();
}
