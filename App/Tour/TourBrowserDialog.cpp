// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Tour/TourBrowserDialog.h"

#include <QIcon>
#include <QListWidgetItem>
#include <QStyle>

#include "App/Core/Settings.h"

TourBrowserDialog::TourBrowserDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(std::make_unique<TourBrowserDialogUi>())
{
    m_ui->setupUi(this);
    populateList();

    connect(m_ui->tourList, &QListWidget::itemSelectionChanged,
            this, &TourBrowserDialog::onSelectionChanged);
    connect(m_ui->tourList, &QListWidget::itemDoubleClicked,
            this, [this] {
                if (m_ui->startButton->isEnabled()) {
                    accept();
                }
            });
}

TourBrowserDialog::~TourBrowserDialog() = default;

QString TourBrowserDialog::selectedTourPath() const
{
    return m_selectedPath;
}

QVector<TourBrowserDialog::TourEntry> TourBrowserDialog::availableTours()
{
    return {
        {
            ":/Tours/ui-overview.json",
            tr("wiRedPanda Interface Tour"),
            "ui-overview",
            tr("A guided walkthrough of the main interface — palette, canvas, toolbar, and properties panel.")
        },
    };
}

void TourBrowserDialog::populateList()
{
    const QStringList completed = Settings::completedTours();
    const QIcon checkIcon  = style()->standardIcon(QStyle::SP_DialogApplyButton);
    const QIcon circleIcon = style()->standardIcon(QStyle::SP_ArrowRight);

    for (const TourEntry &entry : availableTours()) {
        auto *item = new QListWidgetItem(m_ui->tourList);
        item->setText(entry.title);
        item->setData(Qt::UserRole,     entry.resourcePath);
        item->setData(Qt::UserRole + 1, entry.description);
        item->setIcon(completed.contains(entry.id) ? checkIcon : circleIcon);
    }
}

void TourBrowserDialog::onSelectionChanged()
{
    const QList<QListWidgetItem *> sel = m_ui->tourList->selectedItems();
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
