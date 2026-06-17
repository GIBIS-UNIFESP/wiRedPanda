// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Exercise/ExerciseBrowserDialog.h"

#include <QIcon>
#include <QListWidgetItem>
#include <QStyle>

#include "App/Core/Settings.h"

ExerciseBrowserDialog::ExerciseBrowserDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(std::make_unique<ExerciseBrowserDialogUi>())
{
    m_ui->setupUi(this);
    populateList();

    connect(m_ui->exerciseList, &QListWidget::itemSelectionChanged,
            this, &ExerciseBrowserDialog::onSelectionChanged);
    connect(m_ui->exerciseList, &QListWidget::itemDoubleClicked,
            this, [this] {
                if (m_ui->startButton->isEnabled()) {
                    accept();
                }
            });
}

ExerciseBrowserDialog::~ExerciseBrowserDialog() = default;

QString ExerciseBrowserDialog::selectedExercisePath() const
{
    return m_selectedPath;
}

QVector<ExerciseBrowserDialog::ExerciseEntry> ExerciseBrowserDialog::availableExercises()
{
    return {
        {
            ":/Exercises/basic-and-gate.json",
            tr("Building an AND Gate Circuit"),
            "basic-and-gate",
            tr("Learn to place logic gates, add inputs and outputs, and wire them together.")
        },
        {
            ":/Exercises/clock-led-waveform.json",
            tr("Clock and LED: Waveform Basics"),
            "clock-led-waveform",
            tr("Build a minimal Clock–LED circuit and explore the waveform viewer: combinational mode and manual cell editing.")
        },
    };
}

void ExerciseBrowserDialog::populateList()
{
    const QStringList completed = Settings::completedExercises();
    const QIcon checkIcon = style()->standardIcon(QStyle::SP_DialogApplyButton);
    const QIcon circleIcon = style()->standardIcon(QStyle::SP_ArrowRight);

    for (const ExerciseEntry &entry : availableExercises()) {
        auto *item = new QListWidgetItem(m_ui->exerciseList);
        item->setText(entry.title);
        item->setData(Qt::UserRole,        entry.resourcePath);
        item->setData(Qt::UserRole + 1,    entry.description);
        item->setIcon(completed.contains(entry.id) ? checkIcon : circleIcon);
    }
}

void ExerciseBrowserDialog::onSelectionChanged()
{
    const QList<QListWidgetItem *> sel = m_ui->exerciseList->selectedItems();
    const bool hasSelection = !sel.isEmpty();

    m_ui->startButton->setEnabled(hasSelection);

    if (hasSelection) {
        m_selectedPath = sel.first()->data(Qt::UserRole).toString();
        m_ui->descriptionLabel->setText(sel.first()->data(Qt::UserRole + 1).toString());
    } else {
        m_selectedPath.clear();
        m_ui->descriptionLabel->clear();
    }
}
