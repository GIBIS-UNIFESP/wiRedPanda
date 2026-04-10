// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ICDropZone.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMimeData>

#include "App/Core/Enums.h"
#include "App/IO/Serialization.h"

ICDropZone::ICDropZone(Section section, QWidget *parent)
    : QWidget(parent)
    , m_section(section)
{
    setAcceptDrops(true);
}

void ICDropZone::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event->mimeData()->hasFormat("wpanda/x-dnditemdata")
        && !event->mimeData()->hasFormat("application/x-wiredpanda-dragdrop")) {
        return;
    }

    QByteArray itemData;
    if (event->mimeData()->hasFormat("application/x-wiredpanda-dragdrop")) {
        itemData = event->mimeData()->data("application/x-wiredpanda-dragdrop");
    } else {
        itemData = event->mimeData()->data("wpanda/x-dnditemdata");
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    Serialization::readPandaHeader(stream);

    QPoint offset;
    ElementType type;
    QString icFileName;
    stream >> offset >> type >> icFileName;

    bool isEmbedded = false;
    if (!stream.atEnd()) { stream >> isEmbedded; }

    // Only accept opposite-type drops
    if (m_section == Section::Embedded && !isEmbedded) {
        event->acceptProposedAction(); // File-based dropped onto embedded section
    } else if (m_section == Section::FileBased && isEmbedded) {
        event->acceptProposedAction(); // Embedded dropped onto file-based section
    }
}

void ICDropZone::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void ICDropZone::dropEvent(QDropEvent *event)
{
    QByteArray itemData;
    if (event->mimeData()->hasFormat("application/x-wiredpanda-dragdrop")) {
        itemData = event->mimeData()->data("application/x-wiredpanda-dragdrop");
    } else if (event->mimeData()->hasFormat("wpanda/x-dnditemdata")) {
        itemData = event->mimeData()->data("wpanda/x-dnditemdata");
    } else {
        return;
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    Serialization::readPandaHeader(stream);

    QPoint offset;
    ElementType type;
    QString icFileName;
    stream >> offset >> type >> icFileName;

    bool isEmbedded = false;
    QString blobName;
    if (!stream.atEnd()) { stream >> isEmbedded; }
    if (!stream.atEnd()) { stream >> blobName; }

    if (m_section == Section::Embedded && !isEmbedded) {
        emit embedByFileRequested(icFileName);
        event->acceptProposedAction();
    } else if (m_section == Section::FileBased && isEmbedded) {
        emit extractByBlobNameRequested(blobName);
        event->acceptProposedAction();
    }
}

