// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "listitemwidget.h"

#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QVariant>

#include "elementfactory.h"
#include "graphicelement.h"
#include "label.h"

Label *ListItemWidget::getLabel() const
{
    return label;
}

void ListItemWidget::mousePressEvent(QMouseEvent *)
{
    label->startDrag();
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

    label = new Label(parent);
    label->setPixmapData(pixmap);
    label->setName(name);
    label->setAuxData(icFileName);
    label->setElementType(type);

    nameLabel = new QLabel(name, this);
    nameLabel->setText(name);
    itemLayout->addWidget(label);
    itemLayout->addStretch();
    itemLayout->addWidget(nameLabel);
    itemLayout->addStretch();
    itemLayout->setMargin(0);
}

void ListItemWidget::updateName()
{
    ElementType type = label->elementType();
    if (type != ElementType::IC) {
        QString name = ElementFactory::translatedName(type);
        nameLabel->setText(name);
        label->setName(name);
    }
}
