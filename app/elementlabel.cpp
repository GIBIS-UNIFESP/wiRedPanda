// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
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
    m_iconLabel.setPixmap(pixmap);
    m_iconLabel.setScaledContents(true);
    m_iconLabel.setFixedSize(64, 64);

    m_nameLabel.setText((type == ElementType::IC) ? QFileInfo(icFileName).baseName().toUpper()
                                                  : ElementFactory::translatedName(type));

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
        m_nameLabel.setText(ElementFactory::translatedName(m_elementType));
    }
}

void ElementLabel::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    startDrag();
}

const ElementType &ElementLabel::elementType() const
{
    return m_elementType;
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
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
    QPoint offset = m_iconLabel.pixmap()->rect().center();
#else
    QPoint offset = m_iconLabel.pixmap().rect().center();
#endif
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    stream << offset << m_elementType << m_icFileName;

    auto *mimeData = new QMimeData();
    mimeData->setData("wpanda/x-dnditemdata", itemData);

    auto *drag = new QDrag(parent());
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap());
    drag->setHotSpot(offset);
    drag->exec(Qt::CopyAction, Qt::CopyAction);
}

QMimeData *ElementLabel::mimeData()
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
    QPoint offset = m_iconLabel.pixmap()->rect().center();
#else
    QPoint offset = m_iconLabel.pixmap().rect().center();
#endif
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    stream << offset << m_elementType << m_icFileName;

    auto *mimeData = new QMimeData();
    mimeData->setData("wpanda/x-dnditemdata", itemData);

    return mimeData;
}

void ElementLabel::updateTheme()
{
    m_pixmap = ElementFactory::pixmap(m_elementType);
    m_iconLabel.setPixmap(m_pixmap);
}
