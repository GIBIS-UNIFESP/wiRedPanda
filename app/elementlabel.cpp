// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementlabel.h"

#include "elementfactory.h"

#include <QDrag>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMimeData>
#include <QMouseEvent>

ElementLabel::ElementLabel(const QPixmap *pixmap, ElementType type, const QString &icFileName, QWidget *parent)
    : ElementLabel(*pixmap, type, icFileName, parent)
{
    // for compiling on Qt versions < 5.15
}

ElementLabel::ElementLabel(const QPixmap &pixmap, ElementType type, const QString &icFileName, QWidget *parent)
    : QFrame(parent)
    , m_elementType(type)
    , m_pixmap(pixmap)
    , m_icFileName(icFileName)
{
    QString name_ = ElementFactory::translatedName(type);
    if (type == ElementType::IC) {
        name_ = QFileInfo(icFileName).baseName().toUpper();
    }

    m_iconLabel.setPixmap(pixmap);
    m_nameLabel.setText(name_);

    auto *itemLayout = new QHBoxLayout();
    itemLayout->setSpacing(6);
    itemLayout->setObjectName(QStringLiteral("itemLayout"));

    setLayout(itemLayout);

    itemLayout->addWidget(&m_iconLabel);
    itemLayout->addStretch();
    itemLayout->addWidget(&m_nameLabel);
    itemLayout->addStretch();
    itemLayout->setContentsMargins(0, 0, 0, 0);
}

void ElementLabel::updateName()
{
    if (m_elementType != ElementType::IC) {
        QString name_ = ElementFactory::translatedName(m_elementType);
        m_nameLabel.setText(name_);
    }
}

void ElementLabel::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    startDrag();
}

const QPixmap &ElementLabel::pixmap() const
{
    return m_pixmap;
}

QString ElementLabel::name() const
{
    return m_nameLabel.text();
}

QString ElementLabel::icFileName() const
{
    return m_icFileName;
}

void ElementLabel::startDrag()
{
    QPixmap pixMap = pixmap();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    QPoint pos = m_iconLabel.pixmap().rect().center();
#else
    QPoint pos = m_iconLabel.pixmap()->rect().center();
#endif
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    stream << pos << m_elementType << m_icFileName;

    auto *mimeData_ = new QMimeData;
    mimeData_->setData("application/x-dnditemdata", itemData);
    auto *drag = new QDrag(parent());
    drag->setMimeData(mimeData_);
    drag->setPixmap(pixMap);
    drag->setHotSpot(pos);
    drag->exec(Qt::CopyAction, Qt::CopyAction);
}

QMimeData *ElementLabel::mimeData()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    QPoint pos = m_iconLabel.pixmap().rect().center();
#else
    QPoint pos = m_iconLabel.pixmap()->rect().center();
#endif
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    stream << pos << m_elementType << m_icFileName;

    auto *mimeData = new QMimeData;
    mimeData->setData("application/x-dnditemdata", itemData);

    return mimeData;
}

void ElementLabel::updateTheme()
{
    m_pixmap = ElementFactory::pixmap(m_elementType);
    m_iconLabel.setPixmap(m_pixmap);
}
