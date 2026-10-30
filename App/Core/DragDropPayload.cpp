// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/DragDropPayload.h"

#include <QDataStream>
#include <QMimeData>

#include "App/Core/MimeTypes.h"
#include "App/Element/ElementMetadata.h"
#include "App/IO/Serialization.h"

DragDropPayload readDragDropPayload(QDataStream &stream)
{
    DragDropPayload payload;
    stream >> payload.offset;
    stream >> payload.type;
    payload.icFileName = Serialization::readBoundedString(stream);
    if (!stream.atEnd()) { stream >> payload.isEmbedded; }
    if (!stream.atEnd()) { payload.blobName = Serialization::readBoundedString(stream); }
    return payload;
}

std::optional<DragDropPayload> readDragDropPayload(const QMimeData *mimeData)
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
    try {
        Serialization::readPandaHeader(stream);
        const auto payload = readDragDropPayload(stream);
        if ((stream.status() != QDataStream::Ok) || !ElementMetadataRegistry::hasMetadata(payload.type)) {
            return std::nullopt;
        }
        return payload;
    } catch (const std::exception &) {
        return std::nullopt;
    }
}
