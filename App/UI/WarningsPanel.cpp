// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/WarningsPanel.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QVariant>
#include <QVBoxLayout>

#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"
#include "App/Wiring/Port.h"

WarningsPanel::WarningsPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    m_count = new QLabel(tr("0 issues"), this);
    layout->addWidget(m_count);

    m_warnings = new QListWidget(this);
    m_warnings->setObjectName(QStringLiteral("warningsList"));
    m_warnings->setSelectionMode(QAbstractItemView::SingleSelection);
    m_warnings->setFrameShape(QFrame::NoFrame);
    m_warnings->setAlternatingRowColors(true);
    layout->addWidget(m_warnings);

    connect(m_warnings, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        const auto elementId = item->data(Qt::UserRole).toInt();
        emit warningElementActivated(elementId);
    });
}

void WarningsPanel::setScene(Scene *scene)
{
    m_scene = scene;
    refreshWarningsList();
}

void WarningsPanel::refreshWarningsList()
{
    m_warnings->clear();
    if (!m_scene) {
        m_count->setText(tr("0 issues"));
        m_warnings->hide();
        return;
    }

    int issueCount = 0;
    for (auto *element : m_scene->elements()) {
        const auto invalid = element->invalidInputs();
        for (const auto *port : invalid) {
            const QString pin = port->name().isEmpty()
                ? tr("input %1").arg(port->index() + 1)
                : port->name();
            const QString kind = port->connections().size() > 1
                ? tr("Multiple drivers")
                : tr("Not connected");
            const QString title = tr("%1 (%2) - %3: %4")
                                      .arg(element->label())
                                      .arg(element->objectName())
                                      .arg(pin)
                                      .arg(kind);
            auto *item = new QListWidgetItem(title);
            item->setData(Qt::UserRole, element->id());
            m_warnings->addItem(item);
            ++issueCount;
        }
    }
    m_count->setText(issueCount == 1 ? tr("1 issue") : tr("%1 issues").arg(issueCount));
    m_warnings->setVisible(issueCount != 0);
}

void WarningsPanel::retranslateUi()
{
    refreshWarningsList();
}
