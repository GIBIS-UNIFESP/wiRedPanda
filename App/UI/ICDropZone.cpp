// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ICDropZone.h"

#include <optional>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMimeData>

#include "App/Core/Enums.h"
#include "App/Core/MimeTypes.h"
#include "App/IO/Serialization.h"

namespace {

struct DragPayload {
    QPoint offset;
    ElementType type;
    QString icFileName;
    bool isEmbedded = false;
    QString blobName;
};

std::optional<DragPayload> extractDragPayload(const QMimeData *mimeData)
{
    QByteArray itemData;
    if (mimeData->hasFormat(MimeType::DragDrop)) {
        itemData = mimeData->data(MimeType::DragDrop);
    } else if (mimeData->hasFormat(MimeType::DragDropLegacy)) {
        itemData = mimeData->data(MimeType::DragDropLegacy);
    } else {
        return std::nullopt;
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    Serialization::readPandaHeader(stream);

    DragPayload payload;
    stream >> payload.offset >> payload.type >> payload.icFileName;
    if (!stream.atEnd()) { stream >> payload.isEmbedded; }
    if (!stream.atEnd()) { stream >> payload.blobName; }
    return payload;
}

} // namespace

ICDropZone::ICDropZone(Section section, QWidget *parent)
    : QWidget(parent)
    , m_section(section)
{
    setAcceptDrops(true);
}

void ICDropZone::dragEnterEvent(QDragEnterEvent *event)
{
    auto payload = extractDragPayload(event->mimeData());
    if (!payload) return;

    // Only accept opposite-type drops
    if (m_section == Section::Embedded && !payload->isEmbedded) {
        event->acceptProposedAction(); // File-based dropped onto embedded section
    } else if (m_section == Section::FileBased && payload->isEmbedded) {
        event->acceptProposedAction(); // Embedded dropped onto file-based section
    }
}

void ICDropZone::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void ICDropZone::dropEvent(QDropEvent *event)
{
    auto payload = extractDragPayload(event->mimeData());
    if (!payload) return;

    if (m_section == Section::Embedded && !payload->isEmbedded) {
        emit embedByFileRequested(payload->icFileName);
        event->acceptProposedAction();
    } else if (m_section == Section::FileBased && payload->isEmbedded) {
        emit extractByBlobNameRequested(payload->blobName);
        event->acceptProposedAction();
    }
}

