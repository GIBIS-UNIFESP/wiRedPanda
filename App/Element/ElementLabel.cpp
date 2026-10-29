// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementLabel.h"

#include <QApplication>
#include <QDrag>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMimeData>
#include <QMouseEvent>

#include "App/Core/MimeTypes.h"
#include "App/Element/ElementFactory.h"
#include "App/IO/Serialization.h"

ElementLabel::ElementLabel(const QPixmap &pixmap, ElementType type, const QString &icFileName, QWidget *parent, bool isEmbedded)
    : QFrame(parent)
    , m_elementType(type)
    , m_pixmap(pixmap)
    , m_icFileName(icFileName)
    , m_isEmbedded(isEmbedded)
{
    m_iconLabel.setPixmap(pixmap);
    m_iconLabel.setScaledContents(true);
    m_iconLabel.setFixedSize(64, 64);

    if (m_isEmbedded) {
        m_nameLabel.setText(icFileName.toUpper());
    } else if (type == ElementType::IC) {
        m_nameLabel.setText(QFileInfo(icFileName).baseName().toUpper());
    } else {
        m_nameLabel.setText(ElementFactory::translatedName(type));
    }

    // Describe what the item is and how to place it. Set on the frame and both child labels
    // so the tooltip appears wherever the pointer rests on the item.
    QString tip;
    if (m_isEmbedded) {
        tip = tr("Embedded IC: %1").arg(icFileName);
    } else if (type == ElementType::IC) {
        tip = tr("IC from file: %1").arg(QFileInfo(icFileName).fileName());
    } else {
        tip = ElementFactory::description(type);
    }
    const QString addHint = tr("Drag or double-click to add.");
    tip = tip.isEmpty() ? addHint : tip + QLatin1Char('\n') + addHint;
    setToolTip(tip);
    m_iconLabel.setToolTip(tip);
    m_nameLabel.setToolTip(tip);

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
    // Only remember where the press started. Deferring the drag until the pointer
    // actually moves leaves the press/release free for double-click detection, which a
    // blocking drag->exec() on press would otherwise swallow.
    if (event->button() == Qt::LeftButton) {
        m_dragStartPos = event->pos();
    }
}

void ElementLabel::mouseMoveEvent(QMouseEvent *event)
{
    // Empirically confirmed unreachable: this widget never calls setMouseTracking(true), and
    // QApplication's own event delivery (even via a directly-constructed/injected QMouseEvent,
    // not just real input) filters out MouseMove events entirely before dispatch when no button
    // is held and tracking is off -- mouseMoveEvent() itself is simply never invoked in that case,
    // so event->buttons() is guaranteed non-empty (Qt::LeftButton, the only button this label
    // reacts to at all) whenever this line runs. Kept as a defensive guard in case that ever changes.
    if (!(event->buttons() & Qt::LeftButton)) { // LCOV_EXCL_LINE
        return; // LCOV_EXCL_LINE
    }
    if ((event->pos() - m_dragStartPos).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }
    startDrag();
}

void ElementLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    // Drag-free shortcut: add this element straight to the active scene. mimeData()
    // hands off a freshly-allocated payload the scene takes ownership of.
    emit addToSceneRequested(mimeData());
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

bool ElementLabel::isEmbedded() const
{
    return m_isEmbedded;
}

QPoint ElementLabel::dragOffset() const
{
    return m_iconLabel.pixmap(Qt::ReturnByValue).rect().center();
}

void ElementLabel::startDrag()
{
    auto *drag = new QDrag(parent());
    drag->setMimeData(mimeData());
    // The scene draws the element where the drop will put it (see DragPreview); QDrag is given no picture.
    const DragPreview::Scope preview(dragPicture());
    // CopyAction for both proposed and allowed: placing an element on the scene
    // always creates a new instance, never moves the palette entry.
    drag->exec(Qt::CopyAction, Qt::CopyAction);
}

DragPreview::Picture ElementLabel::dragPicture() const
{
    // The element draws its picture from its top-left corner, which the drop puts at the cursor less the offset
    return {pixmap(), QPointF(dragOffset()), QPointF()};
}

QMimeData *ElementLabel::mimeData() const
{
    const QPoint offset = dragOffset();
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    stream << offset << m_elementType << m_icFileName << m_isEmbedded << (m_isEmbedded ? m_icFileName : QString());

    auto *mimeData = new QMimeData();
    mimeData->setData(MimeType::DragDrop, itemData);

    return mimeData;
}

void ElementLabel::updateTheme()
{
    m_pixmap = ElementFactory::pixmap(m_elementType);
    m_iconLabel.setPixmap(m_pixmap);
}
