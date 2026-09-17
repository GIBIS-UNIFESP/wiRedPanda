// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/WarningsPanel.h"

#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QDateTime>

#include "App/Scene/Scene.h"
#include "App/Element/GraphicElement.h"

WarningsPanel::WarningsPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    m_header = new QLabel(tr("Warnings"), this);
    QFont f = m_header->font();
    f.setBold(true);
    m_header->setFont(f);
    lay->addWidget(m_header);

    m_currentWarnings = new QListWidget(this);
    m_currentWarnings->setSelectionMode(QAbstractItemView::NoSelection);
    m_currentWarnings->setMaximumHeight(100);
    lay->addWidget(m_currentWarnings);

    m_errorsToggle = new QPushButton(tr("Errors"), this);
    m_errorsToggle->setCheckable(true);
    m_errorsToggle->setChecked(true);
    lay->addWidget(m_errorsToggle);

    m_errors = new QListWidget(this);
    m_errors->setObjectName(QStringLiteral("errorsList"));
    m_errors->setSelectionMode(QAbstractItemView::SingleSelection);
    m_errors->setVisible(true);
    m_errors->setMaximumHeight(140);
    lay->addWidget(m_errors);

    connect(m_errorsToggle, &QPushButton::toggled, m_errors, &QWidget::setVisible);
    connect(m_errors, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        const auto address = item->data(Qt::UserRole).value<quintptr>();
        emit errorElementActivated(reinterpret_cast<GraphicElement *>(address));
    });

    m_log = new QTextEdit(this);
    m_log->setReadOnly(true);
    lay->addWidget(m_log, 1);
}

void WarningsPanel::setScene(Scene *scene)
{
    m_scene = scene;
    refreshErrorList();
}

void WarningsPanel::showElementWarnings(GraphicElement *element)
{
    m_currentWarnings->clear();
    if (!element) return;
    const auto warnings = element->warnings();
    for (const auto &w : warnings) {
        m_currentWarnings->addItem(w);
    }
}

void WarningsPanel::refreshErrorList()
{
    m_errors->clear();
    if (!m_scene) {
        return;
    }

    for (auto *element : m_scene->elements()) {
        if (!element->hasErrors()) {
            continue;
        }

        const QString title = element->objectName().isEmpty()
            ? tr("Element")
            : element->objectName();
        auto *item = new QListWidgetItem(title + QStringLiteral(": ") + element->warnings().join(QStringLiteral("; ")));
        item->setData(Qt::UserRole, QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(element)));
        m_errors->addItem(item);
    }
}

void WarningsPanel::addLogEntry(const QString &entry)
{
    const QString time = QLocale::system().toString(QDateTime::currentDateTime(), QLocale::ShortFormat);
    m_log->append(time + " — " + entry);
}
