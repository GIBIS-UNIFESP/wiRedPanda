// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/WarningsDock.h"

#include <QFrame>
#include <QLabel>
#include <QListWidget>
#include <QSet>
#include <QStringList>
#include <QVBoxLayout>

#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"
#include "App/Wiring/Port.h"

WarningsDock::WarningsDock(QWidget *parent)
    : QDockWidget(parent)
{
    setObjectName(QStringLiteral("warningsDock"));
    setWindowTitle(tr("Warnings"));

    auto *container = new QWidget(this);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    m_count = new QLabel(tr("0 issues"), container);
    m_count->setObjectName(QStringLiteral("warningsCount"));
    layout->addWidget(m_count);

    m_warnings = new QListWidget(container);
    m_warnings->setObjectName(QStringLiteral("warningsList"));
    m_warnings->setSelectionMode(QAbstractItemView::SingleSelection);
    m_warnings->setFrameShape(QFrame::NoFrame);
    m_warnings->setAlternatingRowColors(true);
    layout->addWidget(m_warnings);

    setWidget(container);

    connect(m_warnings, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (!item) {
            return;
        }
        const auto elementId = item->data(Qt::UserRole).toInt();
        emit warningElementActivated(elementId);
    });
}

void WarningsDock::setScene(Scene *scene)
{
    if (m_scene == scene) {
        refresh();
        return;
    }
    m_scene = scene;
    refresh();
}

void WarningsDock::refresh()
{
    if (!m_scene) {
        m_items.clear();
        m_warnings->clear();
        m_issueCount = 0;
        m_count->setText(tr("0 issues"));
        m_warnings->hide();
        return;
    }

    m_issueCount = 0;
    QSet<int> currentElementIds;

    for (auto *element : m_scene->elements()) {
        if (!element) {
            continue;
        }
        const auto invalid = element->invalidInputs();
        if (invalid.isEmpty()) {
            continue;
        }

        const int id = element->id();
        currentElementIds.insert(id);
        m_issueCount += static_cast<int>(invalid.size());

        const QString name = element->label().isEmpty()
            ? element->objectName()
            : QStringLiteral("%1 (%2)").arg(element->label(), element->objectName());
        const QString issuesStr = (invalid.size() == 1)
            ? tr("1 issue")
            : tr("%1 issues").arg(invalid.size());

        QStringList descriptions;
        descriptions.reserve(invalid.size());
        for (const auto *port : invalid) {
            if (!port) {
                continue;
            }
            const QString pin = port->name().isEmpty()
                ? tr("input %1").arg(port->index() + 1)
                : port->name();
            const QString kind = port->connections().size() > 1
                ? tr("Multiple drivers")
                : tr("Not connected");
            descriptions.append(QStringLiteral("%1: %2").arg(pin, kind));
        }

        const QString title = QStringLiteral("%1 (%2) - %3").arg(name, issuesStr, descriptions.join(QStringLiteral(", ")));
        const QString toolTip = descriptions.join(QLatin1Char('\n'));

        auto it = m_items.find(id);
        if (it != m_items.end()) {
            QListWidgetItem *item = it.value();
            if (item->text() != title) {
                item->setText(title);
            }
            item->setToolTip(toolTip);
        } else {
            auto *item = new QListWidgetItem(title);
            item->setData(Qt::UserRole, id);
            item->setToolTip(toolTip);
            m_warnings->addItem(item);
            m_items.insert(id, item);
        }
    }

    // Remove rows for elements that no longer have issues
    for (auto it = m_items.begin(); it != m_items.end();) {
        if (!currentElementIds.contains(it.key())) {
            delete it.value();
            it = m_items.erase(it);
        } else {
            ++it;
        }
    }

    m_count->setText(m_issueCount == 1 ? tr("1 issue") : tr("%1 issues").arg(m_issueCount));
    m_warnings->setVisible(m_issueCount != 0);
}

void WarningsDock::retranslateUi()
{
    setWindowTitle(tr("Warnings"));
    refresh();
}
