// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Exercise/ExerciseOverlay.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

#include "App/Core/ThemeManager.h"
#include "App/Exercise/ExerciseEngine.h"

ExerciseOverlay::ExerciseOverlay(ExerciseEngine *engine, QWidget *parent)
    : QWidget(parent)
    , m_engine(engine)
{
    setFixedWidth(480);
    setAttribute(Qt::WA_TranslucentBackground);

    setupUi();

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &ExerciseOverlay::applyTheme);
}

void ExerciseOverlay::setupUi()
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(12, 12, 12, 12);
    outerLayout->setSpacing(8);

    // Step counter row
    auto *topRow = new QHBoxLayout;
    m_stepCounter = new QLabel(this);
    topRow->addWidget(m_stepCounter);
    topRow->addStretch();
    outerLayout->addLayout(topRow);

    // Instruction text
    m_instructionLabel = new QLabel(this);
    m_instructionLabel->setWordWrap(true);
    outerLayout->addWidget(m_instructionLabel);

    // Hint text (hidden by default)
    m_hintLabel = new QLabel(this);
    m_hintLabel->setWordWrap(true);
    m_hintLabel->hide();
    outerLayout->addWidget(m_hintLabel);

    // Navigation row: [Exit] [Hint] ←stretch→ [← Back] [Next →]
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(6);

    m_closeButton = new QPushButton(tr("Exit"), this);
    m_closeButton->setToolTip(tr("Close exercise"));

    m_hintButton = new QPushButton(tr("Hint"), this);

    m_prevButton = new QPushButton(tr("← Back"), this);

    m_nextButton = new QPushButton(tr("Next →"), this);
    m_nextButton->setEnabled(false);

    btnRow->addWidget(m_closeButton);
    btnRow->addWidget(m_hintButton);
    btnRow->addStretch();
    btnRow->addWidget(m_prevButton);
    btnRow->addWidget(m_nextButton);
    outerLayout->addLayout(btnRow);

    adjustSize();

    connect(m_closeButton, &QPushButton::clicked, this, &ExerciseOverlay::closeRequested);
    connect(m_hintButton,  &QPushButton::clicked, this, [this] {
        m_hintVisible = !m_hintVisible;
        m_hintLabel->setVisible(m_hintVisible);
        m_hintButton->setText(m_hintVisible ? tr("Hide hint") : tr("Hint"));
        adjustSize();
        repositionToParent();
    });
    connect(m_prevButton, &QPushButton::clicked, this, [this] {
        m_engine->goToPreviousStep();
    });
    connect(m_nextButton, &QPushButton::clicked, this, [this] {
        m_engine->advanceStep();
    });

    applyTheme();
}

void ExerciseOverlay::applyTheme()
{
    const bool dark = (ThemeManager::effectiveTheme() == Theme::Dark);

    if (dark) {
        m_bgColor      = QColor(30, 30, 30, 200);
        m_textColor    = QColor(255, 255, 255);
        m_hintColor    = QColor(255, 255, 180, 230);
        m_counterColor = QColor(255, 255, 255, 200);

        const QString navStyle =
            "QPushButton { color: white; background: rgba(255,255,255,40);"
            " border: 1px solid rgba(255,255,255,80); border-radius: 4px;"
            " padding: 3px 10px; font-size: 13px; }"
            "QPushButton:hover { background: rgba(255,255,255,60); }"
            "QPushButton:disabled { color: rgba(255,255,255,80); background: rgba(80,80,80,60); }";
        m_closeButton->setStyleSheet(navStyle);
        m_hintButton->setStyleSheet(navStyle);
        m_prevButton->setStyleSheet(navStyle);
        m_nextButton->setStyleSheet(
            "QPushButton { color: white; background: rgba(60,150,60,180);"
            " border: 1px solid rgba(255,255,255,80); border-radius: 4px;"
            " padding: 3px 10px; font-size: 13px; }"
            "QPushButton:hover { background: rgba(60,180,60,200); }"
            "QPushButton:disabled { background: rgba(100,100,100,80);"
            " color: rgba(255,255,255,100); }");
    } else {
        m_bgColor      = QColor(245, 245, 245, 230);
        m_textColor    = QColor(20, 20, 20);
        m_hintColor    = QColor(110, 90, 0, 230);
        m_counterColor = QColor(80, 80, 80, 200);

        const QString navStyle =
            "QPushButton { color: rgb(20,20,20); background: rgba(0,0,0,20);"
            " border: 1px solid rgba(0,0,0,80); border-radius: 4px;"
            " padding: 3px 10px; font-size: 13px; }"
            "QPushButton:hover { background: rgba(0,0,0,40); }"
            "QPushButton:disabled { color: rgba(0,0,0,60); background: rgba(0,0,0,10); }";
        m_closeButton->setStyleSheet(navStyle);
        m_hintButton->setStyleSheet(navStyle);
        m_prevButton->setStyleSheet(navStyle);
        m_nextButton->setStyleSheet(
            "QPushButton { color: white; background: rgba(40,130,40,220);"
            " border: 1px solid rgba(0,100,0,150); border-radius: 4px;"
            " padding: 3px 10px; font-size: 13px; }"
            "QPushButton:hover { background: rgba(40,160,40,230); }"
            "QPushButton:disabled { background: rgba(150,150,150,80);"
            " color: rgba(255,255,255,120); }");
    }

    m_stepCounter->setStyleSheet(
        QString("color: rgba(%1,%2,%3,%4); font-size: 12px;")
            .arg(m_counterColor.red()).arg(m_counterColor.green())
            .arg(m_counterColor.blue()).arg(m_counterColor.alpha()));
    m_instructionLabel->setStyleSheet(
        QString("color: rgb(%1,%2,%3); font-size: 14px; font-weight: bold;")
            .arg(m_textColor.red()).arg(m_textColor.green()).arg(m_textColor.blue()));
    m_hintLabel->setStyleSheet(
        QString("color: rgba(%1,%2,%3,%4); font-size: 13px;")
            .arg(m_hintColor.red()).arg(m_hintColor.green())
            .arg(m_hintColor.blue()).arg(m_hintColor.alpha()));

    update();
}

void ExerciseOverlay::repositionToParent()
{
    if (!parentWidget()) {
        return;
    }
    adjustSize();
    const int x = (parentWidget()->width() - width()) / 2;
    const int y = parentWidget()->height() - height() - 12;
    move(x, y);
}

void ExerciseOverlay::onStepChanged(int step, int total, const ExerciseStep &stepData)
{
    m_stepCounter->setText(tr("Step %1 of %2").arg(step + 1).arg(total));
    m_instructionLabel->setText(stepData.instruction);
    m_hintLabel->setText(stepData.hint);

    // Reset hint state on step change
    m_hintVisible = false;
    m_hintLabel->hide();
    m_hintButton->setText(tr("Hint"));

    const bool isObserveStep = stepData.requiredElements.isEmpty() && stepData.requiredConnections.isEmpty();
    const bool isLastStep    = (step == total - 1);

    updateNextButton(isLastStep);
    // Next button enabled only for observe steps (auto-advance handles others)
    m_nextButton->setEnabled(isObserveStep);

    m_prevButton->setEnabled(step > 0);

    adjustSize();
    repositionToParent();
}

void ExerciseOverlay::onRetranslated()
{
    const ExerciseStep &stepData = m_engine->currentStepData();
    const int step  = m_engine->currentStep();
    const int total = m_engine->totalSteps();

    m_stepCounter->setText(tr("Step %1 of %2").arg(step + 1).arg(total));
    m_instructionLabel->setText(stepData.instruction);
    m_hintLabel->setText(stepData.hint);
    updateNextButton(step == total - 1);

    m_closeButton->setText(tr("Exit"));
    m_closeButton->setToolTip(tr("Close exercise"));
    m_hintButton->setText(m_hintVisible ? tr("Hide hint") : tr("Hint"));
    m_prevButton->setText(tr("← Back"));

    adjustSize();
    repositionToParent();
}

void ExerciseOverlay::onExerciseCompleted()
{
    m_stepCounter->setText({});
    m_instructionLabel->setText(tr("Exercise complete! Well done."));
    m_hintLabel->hide();
    m_hintButton->hide();
    m_prevButton->hide();
    m_nextButton->setText(tr("Close"));
    m_nextButton->setEnabled(true);
    disconnect(m_nextButton, &QPushButton::clicked, nullptr, nullptr);
    connect(m_nextButton, &QPushButton::clicked, this, &ExerciseOverlay::closeRequested);
    adjustSize();
    repositionToParent();
}

void ExerciseOverlay::updateNextButton(bool isLastStep)
{
    m_nextButton->setText(isLastStep ? tr("Finish") : tr("Next →"));
}

void ExerciseOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(m_bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 8, 8);
}
