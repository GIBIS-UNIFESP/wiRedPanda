// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/RecordingOverlay.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QStyleOption>
#include <QTimer>

RecordingOverlay::RecordingOverlay(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setupUI();

    m_blinkTimer = new QTimer(this);
    m_blinkTimer->setInterval(500);
    connect(m_blinkTimer, &QTimer::timeout, this, &RecordingOverlay::toggleBlink);
}

void RecordingOverlay::setupUI()
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 6, 10, 6);
    layout->setSpacing(8);

    m_redDotLabel = new QLabel(QStringLiteral("● REC"), this);
    m_redDotLabel->setStyleSheet(QStringLiteral("color: #ff3b30; font-weight: bold; font-size: 13px;"));
    setToolTip(tr("viVIDFrog Simulation Recorder"));

    m_timeLabel = new QLabel(QStringLiteral("00:00 (0 frames)"), this);
    m_timeLabel->setStyleSheet(QStringLiteral("color: #ffffff; font-weight: 500; font-size: 12px; font-family: monospace;"));

    m_stopButton = new QPushButton(tr("Stop"), this);
    m_stopButton->setCursor(Qt::PointingHandCursor);
    m_stopButton->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: #e53935;"
        "  color: white;"
        "  font-weight: bold;"
        "  font-size: 11px;"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 3px 8px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #d32f2f;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #b71c1c;"
        "}"
    ));

    layout->addWidget(m_redDotLabel);
    layout->addWidget(m_timeLabel);
    layout->addWidget(m_stopButton);

    connect(m_stopButton, &QPushButton::clicked, this, &RecordingOverlay::stopRequested);

    setStyleSheet(QStringLiteral("background-color: rgba(25, 25, 25, 220); border-radius: 6px;"));
    adjustSize();
}

void RecordingOverlay::toggleBlink()
{
    m_dotVisible = !m_dotVisible;
    m_redDotLabel->setStyleSheet(m_dotVisible
        ? QStringLiteral("color: #ff3b30; font-weight: bold; font-size: 13px;")
        : QStringLiteral("color: transparent; font-weight: bold; font-size: 13px;"));
}

void RecordingOverlay::paintEvent(QPaintEvent * /*event*/)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void RecordingOverlay::updateStatus(int frameCount, double durationSeconds)
{
    int totalSec = static_cast<int>(durationSeconds);
    int mins = totalSec / 60;
    int secs = totalSec % 60;

    m_timeLabel->setText(QString("%1:%2 (%3 frames)")
                             .arg(mins, 2, 10, QLatin1Char('0'))
                             .arg(secs, 2, 10, QLatin1Char('0'))
                             .arg(frameCount));

    if (!m_blinkTimer->isActive()) {
        m_blinkTimer->start();
    }
}
