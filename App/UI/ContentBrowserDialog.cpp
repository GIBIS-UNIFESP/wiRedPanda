// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ContentBrowserDialog.h"

#include <QDesktopServices>
#include <QIcon>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QStyle>
#include <QUrl>

#include "App/Core/ExerciseTourResources.h"
#include "App/Core/Settings.h"

ContentBrowserDialog::ContentBrowserDialog(ContentCategory category, QWidget *parent)
    : QDialog(parent)
    , m_category(category)
    , m_ui(std::make_unique<ContentBrowserDialogUi>())
{
    m_ui->setupUi(this, m_category);
    populateList();

    connect(m_ui->contentList, &QListWidget::itemSelectionChanged,
            this, &ContentBrowserDialog::onSelectionChanged);
    connect(m_ui->contentList, &QListWidget::itemDoubleClicked,
            this, [this] {
                if (m_ui->startButton->isEnabled()) {
                    accept();
                }
            });
    connect(m_ui->openFolderButton, &QPushButton::clicked, this, [this] {
        const QString dir = ExerciseTourResources::preferredContentDir(categoryKey());
        if (dir.isEmpty()) {
            const QString message = m_category == ContentCategory::Exercises
                ? QCoreApplication::translate("ExerciseBrowserDialog", "Could not create or access a folder for custom exercises.")
                : QCoreApplication::translate("TourBrowserDialog", "Could not create or access a folder for custom tours.");
            QMessageBox::warning(this, windowTitle(), message);
            return;
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    });

#ifdef Q_OS_WASM
    m_ui->openFolderButton->setVisible(false);
#endif
}

ContentBrowserDialog::~ContentBrowserDialog() = default;

QString ContentBrowserDialog::selectedPath() const
{
    return m_selectedPath;
}

QString ContentBrowserDialog::categoryKey() const
{
    return m_category == ContentCategory::Exercises ? QStringLiteral("Exercises") : QStringLiteral("Tours");
}

QStringList ContentBrowserDialog::completedItems() const
{
    return m_category == ContentCategory::Exercises ? Settings::completedExercises() : Settings::completedTours();
}

void ContentBrowserDialog::populateList()
{
    const QStringList completed = completedItems();
    const QIcon checkIcon = style()->standardIcon(QStyle::SP_DialogApplyButton);
    const QIcon circleIcon = style()->standardIcon(QStyle::SP_ArrowRight);

    for (const ExerciseTourResourceEntry &entry : ExerciseTourResources::discover(categoryKey())) {
        auto *item = new QListWidgetItem(m_ui->contentList);
        item->setText(ExerciseTourResources::translate(entry.id + QStringLiteral(".title"), entry.title));
        item->setData(Qt::UserRole,     entry.path);
        item->setData(Qt::UserRole + 1, ExerciseTourResources::translate(entry.id + QStringLiteral(".description"), entry.description));
        item->setIcon(completed.contains(entry.id) ? checkIcon : circleIcon);
    }
}

void ContentBrowserDialog::onSelectionChanged()
{
    const QList<QListWidgetItem *> sel = m_ui->contentList->selectedItems();
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
