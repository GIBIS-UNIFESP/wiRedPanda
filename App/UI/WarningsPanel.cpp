// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/WarningsPanel.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QDateTime>
#include <QVariant>

#include "App/Scene/Scene.h"
#include "App/Element/GraphicElement.h"

WarningsPanel::WarningsPanel(QWidget *parent, const bool errorsOnly)
    : QWidget(parent)
    , m_errorsOnly(errorsOnly)
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

    m_currentWarnings = new QListWidget(this);
    m_currentWarnings->setSelectionMode(QAbstractItemView::NoSelection);
    m_currentWarnings->setMaximumHeight(100);
    m_currentWarnings->setFrameShape(QFrame::NoFrame);
    lay->addWidget(m_currentWarnings);

    m_errors = new QListWidget(this);
    m_errors->setObjectName(QStringLiteral("errorsList"));
    m_errors->setSelectionMode(QAbstractItemView::SingleSelection);
    m_errors->setFrameShape(QFrame::NoFrame);
    m_errors->setAlternatingRowColors(true);
    m_errors->setUniformItemSizes(true);
    m_errors->setSpacing(2);
    lay->addWidget(m_errors);

    connect(m_errors, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        const auto address = item->data(Qt::UserRole).value<quintptr>();
        emit errorElementActivated(reinterpret_cast<GraphicElement *>(address));
    });

    m_log = new QTextEdit(this);
    m_log->setReadOnly(true);
    lay->addWidget(m_log, 1);

    if (m_errorsOnly) {
        m_currentWarnings->hide();
        m_log->hide();
    } else {
        m_errors->hide();
        m_count->hide();
    }
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
        m_count->setText(QStringLiteral("0"));
        return;
    }

    for (auto *element : m_scene->elements()) {
        if (!element->hasErrors()) {
            continue;
        }

        const QString title = element->objectName().isEmpty()
            ? tr("Unnamed element")
            : element->objectName();
        auto *item = new QListWidgetItem(title);
        item->setData(Qt::UserRole, QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(element)));
        m_errors->addItem(item);
    }
    m_count->setText(QString::number(m_errors->count()));
}

void WarningsPanel::addLogEntry(const QString &entry)
{
    const QString time = QLocale::system().toString(QDateTime::currentDateTime(), QLocale::ShortFormat);
    m_log->append(time + " — " + entry);
}
