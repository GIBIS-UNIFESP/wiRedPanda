// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/WarningsPanel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSet>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"
#include "App/Wiring/Port.h"

WarningsPanel::WarningsPanel(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("warningsPanel"));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    auto *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(4, 2, 4, 2);
    headerLayout->setSpacing(6);

    m_header = new QLabel(tr("Warnings"), this);
    QFont f = m_header->font();
    f.setBold(true);
    m_header->setFont(f);
    headerLayout->addWidget(m_header);

    m_count = new QLabel(tr("0 issues"), this);
    m_count->setObjectName(QStringLiteral("warningsCount"));
    headerLayout->addWidget(m_count);
    headerLayout->addStretch();

    m_closeBtn = new QPushButton(QStringLiteral("✕"), this);
    m_closeBtn->setObjectName(QStringLiteral("closeWarningsButton"));
    m_closeBtn->setFlat(true);
    m_closeBtn->setFixedSize(20, 20);
    m_closeBtn->setToolTip(tr("Close warnings panel"));
    headerLayout->addWidget(m_closeBtn);

    layout->addLayout(headerLayout);

    m_description = new QLabel(tr("Components with active warnings"), this);
    m_description->setObjectName(QStringLiteral("warningsDescription"));
    m_description->setStyleSheet(QStringLiteral("color: palette(mid); font-size: 11px;"));
    m_description->setWordWrap(true);
    layout->addWidget(m_description);

    m_warnings = new QTreeWidget(this);
    m_warnings->setObjectName(QStringLiteral("warningsTree"));
    m_warnings->setHeaderHidden(true);
    m_warnings->setRootIsDecorated(true);
    m_warnings->setIndentation(16);
    m_warnings->setAnimated(true);
    m_warnings->setWordWrap(true);
    m_warnings->setExpandsOnDoubleClick(false);
    m_warnings->setSelectionMode(QAbstractItemView::SingleSelection);
    m_warnings->setFrameShape(QFrame::NoFrame);
    m_warnings->setAlternatingRowColors(true);
    layout->addWidget(m_warnings);

    m_emptyState = new QLabel(tr("No warnings"), this);
    m_emptyState->setObjectName(QStringLiteral("warningsEmptyState"));
    m_emptyState->setAlignment(Qt::AlignCenter);
    m_emptyState->setStyleSheet(QStringLiteral("color: palette(mid); padding: 24px;"));
    layout->addWidget(m_emptyState);

    connect(m_closeBtn, &QPushButton::clicked, this, [this] {
        hide();
        emit closeRequested();
    });

    connect(m_warnings, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item, int column) {
        Q_UNUSED(column)
        if (!item) {
            return;
        }
        const auto elementId = item->data(0, Qt::UserRole).toInt();
        emit warningElementActivated(elementId);

        if (item->childCount() > 0) {
            item->setExpanded(!item->isExpanded());
        }
    });
}

void WarningsPanel::setScene(Scene *scene)
{
    if (m_scene == scene) {
        refresh();
        return;
    }
    m_items.clear();
    m_warnings->clear();
    m_scene = scene;
    refresh();
}

void WarningsPanel::refresh()
{
    if (!m_scene) {
        m_items.clear();
        m_warnings->clear();
        m_issueCount = 0;
        m_count->setText(tr("0 issues"));
        m_warnings->hide();
        m_emptyState->show();
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

        const QString typeName = !element->objectName().isEmpty()
            ? element->objectName()
            : ElementFactory::translatedName(element->elementType());
        const QString name = element->label().isEmpty()
            ? typeName
            : QStringLiteral("%1 (%2)").arg(element->label(), typeName);
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

        const QString title = QStringLiteral("%1 (%2)").arg(name, issuesStr);
        const QString toolTip = descriptions.join(QLatin1Char('\n'));

        auto it = m_items.find(id);
        QTreeWidgetItem *item = nullptr;
        if (it != m_items.end()) {
            item = it.value();
            if (item->text(0) != title) {
                item->setText(0, title);
            }
            item->setToolTip(0, toolTip);
        } else {
            item = new QTreeWidgetItem(m_warnings);
            item->setText(0, title);
            QFont f = item->font(0);
            f.setBold(true);
            item->setFont(0, f);
            item->setData(0, Qt::UserRole, id);
            item->setToolTip(0, toolTip);
            // By default, stay collapsed/hidden to avoid clutter
            item->setExpanded(false);
            m_items.insert(id, item);
        }

        // Synchronize child detail items while preserving top-level expanded state
        bool childrenChanged = (item->childCount() != descriptions.size());
        if (!childrenChanged) {
            for (int i = 0; i < descriptions.size(); ++i) {
                if (item->child(i)->text(0) != descriptions.at(i)) {
                    childrenChanged = true;
                    break;
                }
            }
        }
        if (childrenChanged) {
            qDeleteAll(item->takeChildren());
            for (const auto &desc : descriptions) {
                auto *child = new QTreeWidgetItem(item);
                child->setText(0, desc);
                child->setData(0, Qt::UserRole, id);
                child->setToolTip(0, desc);
            }
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

    const QString issuesSummary = (m_issueCount == 1)
        ? tr("1 issue")
        : tr("%1 issues").arg(m_issueCount);
    m_count->setText(issuesSummary);

    m_warnings->setVisible(m_issueCount != 0);
    m_emptyState->setVisible(m_issueCount == 0);
}

QSize WarningsPanel::sizeHint() const
{
    return QSize(260, 400);
}

QSize WarningsPanel::minimumSizeHint() const
{
    return QSize(160, 100);
}

void WarningsPanel::retranslateUi()
{
    m_header->setText(tr("Warnings"));
    m_description->setText(tr("Components with active warnings"));
    m_emptyState->setText(tr("No warnings"));
    m_closeBtn->setToolTip(tr("Close warnings panel"));
    refresh();
}
