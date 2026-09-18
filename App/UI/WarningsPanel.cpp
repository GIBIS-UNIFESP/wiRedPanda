// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/WarningsPanel.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QVariant>

#include "App/Scene/Scene.h"
#include "App/Element/GraphicElement.h"

WarningsPanel::WarningsPanel(QWidget *parent, const bool summaryOnly)
    : QWidget(parent)
    , m_summaryOnly(summaryOnly)
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(8, 8, 8, 8);
    lay->setSpacing(6);

    auto *headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(4, 2, 4, 2);
    headerLayout->setSpacing(6);

    m_header = new QLabel(tr("Warnings"), this);
    QFont f = m_header->font();
    f.setBold(true);
    f.setPointSize(f.pointSize() + 1);
    m_header->setFont(f);
    headerLayout->addWidget(m_header);

    m_count = new QLabel(this);
    m_count->setObjectName(QStringLiteral("warningsCount"));
    m_count->setAlignment(Qt::AlignCenter);
    m_count->setMinimumWidth(24);
    m_count->setText(QStringLiteral("0"));
    headerLayout->addWidget(m_count);
    headerLayout->addStretch();
    lay->addLayout(headerLayout);

    m_description = new QLabel(summaryOnly
        ? tr("Components with active warnings")
        : tr("Messages for the selected component"), this);
    m_description->setObjectName(QStringLiteral("warningsDescription"));
    m_description->setStyleSheet(QStringLiteral("color: palette(mid);"));
    m_description->setWordWrap(true);
    lay->addWidget(m_description);

    m_currentWarnings = new QListWidget(this);
    m_currentWarnings->setSelectionMode(QAbstractItemView::NoSelection);
    m_currentWarnings->setMaximumHeight(100);
    m_currentWarnings->setFrameShape(QFrame::NoFrame);
    lay->addWidget(m_currentWarnings);

    m_warnings = new QListWidget(this);
    m_warnings->setObjectName(QStringLiteral("warningsList"));
    m_warnings->setSelectionMode(QAbstractItemView::SingleSelection);
    m_warnings->setFrameShape(QFrame::NoFrame);
    m_warnings->setAlternatingRowColors(true);
    m_warnings->setUniformItemSizes(true);
    m_warnings->setSpacing(2);
    lay->addWidget(m_warnings);

    m_emptyState = new QLabel(tr("No warnings"), this);
    m_emptyState->setAlignment(Qt::AlignCenter);
    m_emptyState->setStyleSheet(QStringLiteral("color: palette(mid); padding: 16px;"));
    lay->addWidget(m_emptyState);

    connect(m_warnings, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        const auto address = item->data(Qt::UserRole).value<quintptr>();
        emit warningElementActivated(reinterpret_cast<GraphicElement *>(address));
    });

    if (m_summaryOnly) {
        m_currentWarnings->hide();
    } else {
        m_header->hide();
        m_description->hide();
        m_warnings->hide();
        m_emptyState->hide();
        m_count->hide();
    }
}

void WarningsPanel::setScene(Scene *scene)
{
    m_scene = scene;
    refreshWarningsList();
}

void WarningsPanel::showElementWarnings(GraphicElement *element)
{
    m_currentWarnings->clear();
    if (!element) {
        m_header->hide();
        m_description->hide();
        m_currentWarnings->hide();
        return;
    }

    m_header->show();
    m_description->show();
    m_currentWarnings->show();
    const auto warnings = element->warnings();
    for (const auto &w : warnings) {
        m_currentWarnings->addItem(w);
    }
}

void WarningsPanel::refreshWarningsList()
{
    m_warnings->clear();
    if (!m_scene) {
        m_count->setText(QStringLiteral("0"));
        m_emptyState->show();
        return;
    }

    for (auto *element : m_scene->elements()) {
        if (!element->hasWarnings()) {
            continue;
        }

        const QString title = element->objectName().isEmpty()
            ? tr("Unnamed element")
            : element->objectName();
        auto *item = new QListWidgetItem(title);
        item->setData(Qt::UserRole, QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(element)));
        item->setToolTip(element->warnings().join(QStringLiteral("\n")));
        m_warnings->addItem(item);
    }
    m_count->setText(QString::number(m_warnings->count()));
    m_emptyState->setVisible(m_warnings->count() == 0);
}
