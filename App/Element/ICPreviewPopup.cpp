// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICPreviewPopup.h"

#include <QFrame>
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

    // Inset frame holds both the filename caption and the preview image, so the
    // name reads against the same framed background as the rendered circuit
    // rather than floating on the popup chrome outside the frame.
    auto *frame = new QFrame(this);
    frame->setObjectName(QStringLiteral("previewFrame"));
    auto *frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(6, 6, 6, 6);
    frameLayout->setSpacing(4);

    m_titleLabel = new QLabel(frame);
    m_titleLabel->setObjectName(QStringLiteral("title"));
    m_titleLabel->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(m_titleLabel);

    m_imageLabel = new QLabel(frame);
    m_imageLabel->setObjectName(QStringLiteral("preview"));
    m_imageLabel->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(m_imageLabel);

    layout->addWidget(frame);

    // Outer popup chrome + an inset frame around the caption and pixmap so the
    // content reads as a single framed card rather than floating content.
    // Selectors are scoped by objectName so the inner labels don't inherit the
    // frame's border/background.
    setStyleSheet(
        "ICPreviewPopup {"
        "  background-color: rgba(30, 30, 30, 230);"
        "  border: 1px solid rgba(120, 120, 120, 180);"
        "  border-radius: 6px;"
        "}"
        "QFrame#previewFrame {"
        "  border: 1px solid rgba(170, 170, 170, 200);"
        "  border-radius: 3px;"
        "  background-color: rgba(15, 15, 15, 200);"
        "}"
        "QLabel#title {"
        "  color: rgba(230, 230, 230, 235);"
        "  font-weight: bold;"
        "  padding: 2px 4px;"
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

    // The popup carries the IC's name (formerly a separate Qt tooltip) plus the
    // rendered preview. The pixmap is null when the IC was empty or oversized at
    // load time, or when generation failed — in that case we still show the name.
    const QString title = m_pendingIC->displayName();
    const QPixmap &preview = m_pendingIC->previewPixmap();

    if (title.isEmpty() && preview.isNull()) {
        hide();
        return;
    }

    m_titleLabel->setText(title);
    m_titleLabel->setVisible(!title.isEmpty());

    if (preview.isNull()) {
        m_imageLabel->clear();
        m_imageLabel->hide();
    } else {
        m_imageLabel->setPixmap(preview);
        m_imageLabel->show();
    }

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
