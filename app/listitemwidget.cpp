// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "listitemwidget.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QMenu>
#include <QMouseEvent>

#include "elementfactory.h"
#include "label.h"
#include "mainwindow.h"

Label *ListItemWidget::getLabel() const
{
    return m_label;
}

void ListItemWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_label->elementType() == ElementType::IC) {
        if (event->button() == Qt::MouseButton::RightButton) {
            m_menu->exec(event->globalPos());
            return;
        }
    }
    m_label->startDrag();
}

ListItemWidget::ListItemWidget(const QPixmap &pixmap, ElementType type, const QString &icFileName, QWidget *parent)
    : QFrame(parent)
{
    auto *itemLayout = new QHBoxLayout();
    QString name = ElementFactory::translatedName(type);
    if (type == ElementType::IC) {
        name = QFileInfo(icFileName).baseName().toUpper();
        setupIcMenu();
    }
    itemLayout->setSpacing(6);
    itemLayout->setObjectName(QStringLiteral("itemLayout"));
    /*  itemLayout->setSizeConstraint( QLayout::SetFixedSize ); */

    setLayout(itemLayout);

    m_label = new Label(parent);
    m_label->setPixmapData(pixmap);
    m_label->setName(name);
    m_label->setAuxData(icFileName);
    m_label->setElementType(type);

    m_nameLabel = new QLabel(name, this);
    m_nameLabel->setText(name);
    itemLayout->addWidget(m_label);
    itemLayout->addStretch();
    itemLayout->addWidget(m_nameLabel);
    itemLayout->addStretch();
    itemLayout->setMargin(0);
}

QString ListItemWidget::getFileName()
{
    return m_label->auxData();
}

void ListItemWidget::updateName()
{
    ElementType type = m_label->elementType();
    if (type != ElementType::IC) {
        QString name = ElementFactory::translatedName(type);
        m_nameLabel->setText(name);
        m_label->setName(name);
    }
}

void ListItemWidget::setupIcMenu()
{
    m_menu = new QMenu(this);

    auto removeAction = m_menu->addAction(QIcon(QPixmap(":/toolbar/delete.png")), tr("Delete"), this,
        [this] { Q_EMIT removeIC(this); });
    removeAction->setObjectName(QStringLiteral("remove-ic"));

    auto mainWindow = static_cast<MainWindow *>(parent());
    connect(this, &ListItemWidget::removeIC, mainWindow, &MainWindow::removeIc);
}
