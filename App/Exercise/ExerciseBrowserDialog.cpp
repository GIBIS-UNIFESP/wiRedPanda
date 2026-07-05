// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Exercise/ExerciseBrowserDialog.h"

#include <QDesktopServices>
#include <QIcon>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QStyle>
#include <QUrl>

#include "App/Core/ExerciseTourResources.h"
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
    connect(m_ui->openFolderButton, &QPushButton::clicked, this, [this] {
        const QString dir = ExerciseTourResources::preferredContentDir(QStringLiteral("Exercises"));
        if (dir.isEmpty()) {
            QMessageBox::warning(this, windowTitle(),
                tr("Could not create or access a folder for custom exercises."));
            return;
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    });

#ifdef Q_OS_WASM
    m_ui->openFolderButton->setVisible(false);
#endif
}

ExerciseBrowserDialog::~ExerciseBrowserDialog() = default;

QString ExerciseBrowserDialog::selectedExercisePath() const
{
    return m_selectedPath;
}

void ExerciseBrowserDialog::populateList()
{
    const QStringList completed = Settings::completedExercises();
    const QIcon checkIcon = style()->standardIcon(QStyle::SP_DialogApplyButton);
    const QIcon circleIcon = style()->standardIcon(QStyle::SP_ArrowRight);

    for (const ExerciseTourResourceEntry &entry : ExerciseTourResources::discover(QStringLiteral("Exercises"))) {
        auto *item = new QListWidgetItem(m_ui->exerciseList);
        item->setText(ExerciseTourResources::translate(entry.id + QStringLiteral(".title"), entry.title));
        item->setData(Qt::UserRole,        entry.path);
        item->setData(Qt::UserRole + 1,    ExerciseTourResources::translate(entry.id + QStringLiteral(".description"), entry.description));
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
