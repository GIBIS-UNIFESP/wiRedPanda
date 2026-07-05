// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Tour/TourBrowserDialog.h"

#include <QDesktopServices>
#include <QIcon>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QStyle>
#include <QUrl>

#include "App/Core/Settings.h"
#include "App/Core/ExerciseTourResources.h"

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
    connect(m_ui->openFolderButton, &QPushButton::clicked, this, [this] {
        const QString dir = ExerciseTourResources::preferredContentDir(QStringLiteral("Tours"));
        if (dir.isEmpty()) {
            QMessageBox::warning(this, windowTitle(),
                tr("Could not create or access a folder for custom tours."));
            return;
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    });

#ifdef Q_OS_WASM
    m_ui->openFolderButton->setVisible(false);
#endif
}

TourBrowserDialog::~TourBrowserDialog() = default;

QString TourBrowserDialog::selectedTourPath() const
{
    return m_selectedPath;
}

void TourBrowserDialog::populateList()
{
    const QStringList completed = Settings::completedTours();
    const QIcon checkIcon  = style()->standardIcon(QStyle::SP_DialogApplyButton);
    const QIcon circleIcon = style()->standardIcon(QStyle::SP_ArrowRight);

    for (const ExerciseTourResourceEntry &entry : ExerciseTourResources::discover(QStringLiteral("Tours"))) {
        auto *item = new QListWidgetItem(m_ui->tourList);
        item->setText(ExerciseTourResources::translate(entry.id + QStringLiteral(".title"), entry.title));
        item->setData(Qt::UserRole,     entry.path);
        item->setData(Qt::UserRole + 1, ExerciseTourResources::translate(entry.id + QStringLiteral(".description"), entry.description));
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
