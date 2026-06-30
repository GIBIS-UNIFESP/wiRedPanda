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
          ":/Exercises/basic-or-not-gates.json",
          tr("More basic gates: OR and NOT"),
          "basic-or-not-gates",
          tr("Learn to use the OR and NOT gates.")
        },
        {
          ":/Exercises/combinational-circuits-one.json",
          tr("Combinational Circuits I: MUX and DEMUX"),
          "combinational-circuits-one",
          tr("Learn about the MUX and DEMUX circuits.")
        },
        {
          ":/Exercises/combinational-circuits-two.json",
          tr("Combinational Circuits II: Decoder"),
          "combinational-circuits-two",
          tr("Learn how to build a 2-to-4 Decoder.")
        },
        {
          ":/Exercises/combinational-circuits-three.json",
          tr("Combinational Circuits III: Adder"),
          "combinational-circuits-two",
          tr("Learn how to build an adder circuit.")
        },
        {
          ":/Exercises/clock-led-waveform.json",
          tr("Clock and LED: Waveform Basics"),
          "clock-led-waveform",
          tr("Build a minimal Clock–LED circuit and explore the waveform viewer: combinational mode and manual cell editing.")
        },
        {
          ":/Exercises/sequential-circuits-one.json",
          tr("Sequential Circuits I: Latches"),
          "sequential-circuits-one",
          tr("Learn how to use Latches.")
        },
        {
          ":/Exercises/sequential-circuits-two.json",
          tr("Sequential Circuits II: Flip-Flops"),
          "sequential-circuits-two",
          tr("Learn about Flip-Flops.")
        },
        {
          ":/Exercises/sequential-circuits-three.json",
          tr("Sequential Circuits III: Counter"),
          "sequential-circuits-three",
          tr("Learn how to build a counter circuit.")
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
