// Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "listitemwidget.h"

#include <QFileInfo>
#include <QHBoxLayout>

#include "elementfactory.h"
#include "label.h"

Label *ListItemWidget::getLabel() const
{
    return m_label;
}

void ListItemWidget::mousePressEvent(QMouseEvent *)
{
    m_label->startDrag();
}

ListItemWidget::ListItemWidget(const QPixmap &pixmap, ElementType type, const QString &icFileName, QWidget *parent)
    : QFrame(parent)
{
    auto *itemLayout = new QHBoxLayout();
    QString name = ElementFactory::translatedName(type);
    if (type == ElementType::IC) {
        name = QFileInfo(icFileName).baseName().toUpper();
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
    itemLayout->setContentsMargins(0, 0, 0, 0);
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
