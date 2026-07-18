// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickICPreview.h"

#include <QBuffer>

#include "App/Core/Settings.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/QuickShell/Canvas/CanvasItem.h"

QuickICPreview::QuickICPreview(QObject *parent)
    : QObject(parent)
{
    m_hideTimer.setSingleShot(true);
    m_hideTimer.setInterval(300);
    connect(&m_hideTimer, &QTimer::timeout, this, &QuickICPreview::hidePopup);

    m_showTimer.setSingleShot(true);
    m_showTimer.setInterval(1000);
    connect(&m_showTimer, &QTimer::timeout, this, &QuickICPreview::executeShow);
}

void QuickICPreview::setCanvas(CanvasItem *canvas)
{
    if (m_canvas) {
        disconnect(m_canvas, nullptr, this, nullptr);
    }
    m_canvas = canvas;
    cancelHide();
    m_pendingIc = nullptr;
    hidePopup();
    if (m_canvas) {
        connect(m_canvas, &CanvasItem::icPreviewRequested, this, &QuickICPreview::onPreviewRequested);
        connect(m_canvas, &CanvasItem::icPreviewMoved, this, &QuickICPreview::onPreviewMoved);
        connect(m_canvas, &CanvasItem::icPreviewHideRequested, this, &QuickICPreview::onPreviewHideRequested);
        connect(m_canvas, &CanvasItem::icPreviewCancelRequested, this, &QuickICPreview::onPreviewCancelRequested);
    }
}

void QuickICPreview::onPreviewRequested(GraphicElement *ic, QPoint screenPos)
{
    showForIC(ic, screenPos);
}

void QuickICPreview::onPreviewMoved(GraphicElement *ic, QPoint screenPos)
{
    if (isShowActiveFor(ic)) {
        updatePendingPos(screenPos);
    } else {
        showForIC(ic, screenPos);
    }
}

void QuickICPreview::onPreviewHideRequested()
{
    scheduleHide();
}

void QuickICPreview::onPreviewCancelRequested(GraphicElement *ic)
{
    Q_UNUSED(ic)
    cancelHide();
    hidePopup();
}

void QuickICPreview::showForIC(GraphicElement *ic, const QPoint &screenPos)
{
    if (Settings::icPreviewDisabled()) {
        return;
    }

    cancelHide();

    if (!ic) {
        return;
    }

    m_pendingIc = ic;
    m_pendingScreenPos = screenPos;

    if (m_visible) {
        // Already visible (e.g. moved quickly from another IC): update immediately, no delay.
        executeShow();
    } else {
        m_showTimer.start();
    }
}

void QuickICPreview::executeShow()
{
    GraphicElement *ic = m_pendingIc;
    if (!ic || !m_canvas) {
        return;
    }

    auto *icElement = qobject_cast<IC *>(ic);
    const QString title = icElement ? icElement->displayName() : QString();
    const QImage image = m_canvas->renderICPreviewImage(ic);

    // The image is null when the IC was empty/oversized, or generation failed -- still show
    // the title in that case, matching ICPreviewPopup::executeShow()'s own fallback.
    if (title.isEmpty() && image.isNull()) {
        hidePopup();
        return;
    }

    m_title = title;
    m_imageUrl.clear();
    if (!image.isNull()) {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        m_imageUrl = QStringLiteral("data:image/png;base64,") + QString::fromLatin1(bytes.toBase64());
    }
    m_screenPos = m_pendingScreenPos;
    m_visible = true;
    emit refreshed();
}

void QuickICPreview::scheduleHide()
{
    m_showTimer.stop();
    m_hideTimer.start();
}

void QuickICPreview::cancelHide()
{
    m_hideTimer.stop();
    m_showTimer.stop();
}

void QuickICPreview::updatePendingPos(const QPoint &screenPos)
{
    if (!m_visible) {
        m_pendingScreenPos = screenPos;
    }
}

bool QuickICPreview::isShowActiveFor(const GraphicElement *ic) const
{
    return m_pendingIc == ic && (m_visible || m_showTimer.isActive());
}

void QuickICPreview::hidePopup()
{
    if (m_visible) {
        m_visible = false;
        emit refreshed();
    }
}
