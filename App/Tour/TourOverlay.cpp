// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Tour/TourOverlay.h"

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPushButton>
#include <QVBoxLayout>

#include "App/Core/ThemeManager.h"
#include "App/Tour/TourEngine.h"

static constexpr int kCalloutWidth   = 360;
static constexpr int kSpotlightPad   = 6;
static constexpr int kCalloutMargin  = 12;

TourOverlay::TourOverlay(TourEngine *engine, QWidget *parent)
    : QWidget(parent)
    , m_engine(engine)
{
    // Cover the entire parent from the start
    if (parent) {
        resize(parent->size());
        parent->installEventFilter(this);
    }

    setupUi();

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &TourOverlay::applyTheme);
}

void TourOverlay::setTargetResolver(TargetResolver resolver)
{
    m_resolver = std::move(resolver);
}

void TourOverlay::setParentWindow(QWidget *newParent)
{
    if (parentWidget()) {
        parentWidget()->removeEventFilter(this);
    }
    setParent(newParent);
    if (newParent) {
        resize(newParent->size());
        newParent->installEventFilter(this);
    }
}

void TourOverlay::setupUi()
{
    // --- Callout panel ---
    m_callout = new QFrame(this);
    m_callout->setFixedWidth(kCalloutWidth);

    auto *layout = new QVBoxLayout(m_callout);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(8);

    // Top row: step counter
    auto *topRow = new QHBoxLayout;
    m_stepCounter = new QLabel(m_callout);
    topRow->addWidget(m_stepCounter);
    topRow->addStretch();
    layout->addLayout(topRow);

    m_titleLabel = new QLabel(m_callout);
    m_titleLabel->setWordWrap(true);
    layout->addWidget(m_titleLabel);

    m_bodyLabel = new QLabel(m_callout);
    m_bodyLabel->setWordWrap(true);
    layout->addWidget(m_bodyLabel);

    // Navigation row: [Exit] [← Back] ←stretch→ [Next →]
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(6);

    m_closeButton = new QPushButton(tr("Exit"), m_callout);
    m_closeButton->setToolTip(tr("Stop the tour"));

    m_prevButton = new QPushButton(tr("← Back"), m_callout);

    m_nextButton = new QPushButton(tr("Next →"), m_callout);

    btnRow->addWidget(m_closeButton);
    btnRow->addWidget(m_prevButton);
    btnRow->addStretch();
    btnRow->addWidget(m_nextButton);
    layout->addLayout(btnRow);

    m_callout->adjustSize();

    connect(m_closeButton, &QPushButton::clicked, this, &TourOverlay::closeRequested);
    connect(m_prevButton,  &QPushButton::clicked, this, [this] { m_engine->goToPreviousStep(); });
    connect(m_nextButton,  &QPushButton::clicked, this, [this] { m_engine->advanceStep(); });

    applyTheme();
}

void TourOverlay::applyTheme()
{
    const bool dark = (ThemeManager::effectiveTheme() == Theme::Dark);

    if (dark) {
        m_dimColor       = QColor(0, 0, 0, 160);
        m_spotlightColor = QColor(255, 200, 50, 220);

        m_callout->setStyleSheet(
            "QFrame {"
            "  background: rgba(20,20,20,240);"
            "  border: 1px solid rgba(255,200,50,120);"
            "  border-radius: 8px;"
            "}"
            "QLabel { background: transparent; border: none; }"
            "QPushButton { border-radius: 4px; padding: 4px 12px; font-size: 13px; }"
        );
        m_stepCounter->setStyleSheet("color: rgba(255,200,50,200); font-size: 12px;");
        m_titleLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
        m_bodyLabel->setStyleSheet("color: rgba(220,220,220,230); font-size: 14px;");

        const QString navStyle =
            "QPushButton { color: white; background: rgba(255,255,255,40);"
            " border: 1px solid rgba(255,255,255,80); border-radius: 4px;"
            " padding: 4px 12px; font-size: 13px; }"
            "QPushButton:hover { background: rgba(255,255,255,60); }"
            "QPushButton:disabled { color: rgba(255,255,255,80); background: rgba(80,80,80,60); }";
        m_closeButton->setStyleSheet(navStyle);
        m_prevButton->setStyleSheet(navStyle);
        m_nextButton->setStyleSheet(
            "QPushButton { color: white; background: rgba(255,160,20,200);"
            " border: 1px solid rgba(255,200,50,150); border-radius: 4px;"
            " padding: 4px 12px; font-size: 13px; }"
            "QPushButton:hover { background: rgba(255,180,40,230); }");
    } else {
        m_dimColor       = QColor(0, 0, 0, 90);
        m_spotlightColor = QColor(0, 120, 210, 220);

        m_callout->setStyleSheet(
            "QFrame {"
            "  background: rgba(255,255,255,240);"
            "  border: 1px solid rgba(0,100,200,150);"
            "  border-radius: 8px;"
            "}"
            "QLabel { background: transparent; border: none; }"
            "QPushButton { border-radius: 4px; padding: 4px 12px; font-size: 13px; }"
        );
        m_stepCounter->setStyleSheet("color: rgba(0,100,200,200); font-size: 12px;");
        m_titleLabel->setStyleSheet("color: rgb(20,20,20); font-size: 16px; font-weight: bold;");
        m_bodyLabel->setStyleSheet("color: rgba(50,50,50,230); font-size: 14px;");

        const QString navStyle =
            "QPushButton { color: rgb(20,20,20); background: rgba(0,0,0,20);"
            " border: 1px solid rgba(0,0,0,80); border-radius: 4px;"
            " padding: 4px 12px; font-size: 13px; }"
            "QPushButton:hover { background: rgba(0,0,0,40); }"
            "QPushButton:disabled { color: rgba(0,0,0,60); background: rgba(0,0,0,10); }";
        m_closeButton->setStyleSheet(navStyle);
        m_prevButton->setStyleSheet(navStyle);
        m_nextButton->setStyleSheet(
            "QPushButton { color: white; background: rgba(0,120,210,200);"
            " border: 1px solid rgba(0,100,200,150); border-radius: 4px;"
            " padding: 4px 12px; font-size: 13px; }"
            "QPushButton:hover { background: rgba(0,140,230,230); }");
    }

    update();
}

bool TourOverlay::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == parentWidget() && event->type() == QEvent::Resize) {
        resize(parentWidget()->size());
        repositionCallout();
    }
    return QWidget::eventFilter(obj, event);
}

void TourOverlay::onStepChanged(int step, int total, const TourStep &stepData)
{
    m_callout->hide();

    m_stepCounter->setText(tr("Step %1 of %2").arg(step + 1).arg(total));
    m_titleLabel->setText(stepData.title);
    m_bodyLabel->setText(stepData.body);

    m_prevButton->setEnabled(step > 0);
    m_nextButton->setText(step == total - 1 ? tr("Finish") : tr("Next →"));

    m_currentTarget = stepData.target;
    m_highlightRect = QRect{};
    if (m_resolver && !stepData.target.isEmpty() && stepData.target != "none") {
        m_highlightRect = m_resolver(stepData.target);
    }

    m_callout->adjustSize();
    repositionCallout();
    // Use update() (deferred) instead of repaint() (synchronous) so that all
    // pending QEvent::LayoutRequest events are processed before paintEvent
    // re-resolves the spotlight geometry.
    update();
    m_callout->show();
}

void TourOverlay::onTourFinished()
{
    hide();
    emit closeRequested();
}

void TourOverlay::repositionCallout()
{
    const int pad = kCalloutMargin;
    const int cw  = m_callout->width();
    const int ch  = m_callout->height();
    const int ow  = width();
    const int oh  = height();

    if (m_highlightRect.isEmpty()) {
        // Center the callout in the overlay
        m_callout->move((ow - cw) / 2, (oh - ch) / 2);
        return;
    }

    const QRect spot = m_highlightRect.adjusted(-kSpotlightPad, -kSpotlightPad,
                                                 kSpotlightPad, kSpotlightPad);

    // Try below the spotlight
    int cx = qBound(pad, spot.left() + (spot.width() - cw) / 2, ow - cw - pad);
    int cy = spot.bottom() + pad;

    if (cy + ch + pad > oh) {
        // Not enough room below — try above
        cy = spot.top() - ch - pad;
    }

    if (cy < pad) {
        // No room above either — center vertically, shift right of spotlight
        cy = (oh - ch) / 2;
        cx = qMin(spot.right() + pad, ow - cw - pad);
        if (cx + cw + pad > ow) {
            cx = qMax(pad, spot.left() - cw - pad);
        }
    }

    m_callout->move(cx, qBound(pad, cy, oh - ch - pad));
}

void TourOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    // Re-resolve at paint time: Qt guarantees all QEvent::LayoutRequest events
    // are processed before paintEvent fires, so the geometry is always settled.
    if (m_resolver && !m_currentTarget.isEmpty() && m_currentTarget != "none") {
        const QRect liveRect = m_resolver(m_currentTarget);
        if (liveRect != m_highlightRect) {
            m_highlightRect = liveRect;
            repositionCallout();
        }
    }

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (m_highlightRect.isEmpty()) {
        p.fillRect(rect(), m_dimColor);
        return;
    }

    const QRect spot = m_highlightRect.adjusted(-kSpotlightPad, -kSpotlightPad,
                                                 kSpotlightPad, kSpotlightPad);

    QPainterPath full;
    full.addRect(rect());
    QPainterPath hole;
    hole.addRoundedRect(spot, 6, 6);
    p.fillPath(full.subtracted(hole), m_dimColor);

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(m_spotlightColor, 2));
    p.drawRoundedRect(spot, 6, 6);
}
