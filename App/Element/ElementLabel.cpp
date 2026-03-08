// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementLabel.h"

#include <QDrag>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMimeData>
#include <QMouseEvent>

#include "App/Element/ElementFactory.h"
#include "App/IO/Serialization.h"

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

    // ICs are identified by their .panda file name rather than a translated string
    // because each IC has a unique user-chosen name rather than a fixed element type name.
    m_nameLabel.setText(type == ElementType::IC ? QFileInfo(icFileName).baseName().toUpper()
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
    // The hot spot is the icon's centre so the element appears centred under
    // the cursor when dropped onto the scene.
    QPoint offset = m_iconLabel.pixmap(Qt::ReturnByValue).rect().center();
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    // Embed a full panda header so the drop target can version-check the data
    // using the same code path as file loading.
    Serialization::writePandaHeader(stream);
    stream << offset << m_elementType << m_icFileName;

    auto *mimeData = new QMimeData();
    // Custom MIME type prevents accidental drops on foreign Qt applications
    // that happen to accept generic drag-and-drop data.
    mimeData->setData("application/x-wiredpanda-dragdrop", itemData);

    auto *drag = new QDrag(parent());
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap());
    drag->setHotSpot(offset);
    // CopyAction for both proposed and allowed: placing an element on the scene
    // always creates a new instance, never moves the palette entry.
    drag->exec(Qt::CopyAction, Qt::CopyAction);
}

QMimeData *ElementLabel::mimeData()
{
    QPoint offset = m_iconLabel.pixmap(Qt::ReturnByValue).rect().center();
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    stream << offset << m_elementType << m_icFileName;

    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-wiredpanda-dragdrop", itemData);

    return mimeData;
}

void ElementLabel::updateTheme()
{
    m_pixmap = ElementFactory::pixmap(m_elementType);
    m_iconLabel.setPixmap(m_pixmap);
}
