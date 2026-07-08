// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/DragDropPayload.h"

#include <QDataStream>

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
