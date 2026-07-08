// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Shared reader for wiRedPanda's drag-and-drop MIME payload format.
 */

#pragma once

#include <QPoint>
#include <QString>

#include "App/Core/Enums.h"

class QDataStream;

/**
 * \struct DragDropPayload
 * \brief Decoded contents of a wiRedPanda drag-and-drop MIME payload.
 *
 * \details Carries either a file-based IC reference (\c icFileName) or an embedded IC
 * reference (\c isEmbedded + \c blobName), never both — populated by
 * \c readDragDropPayload().
 */
struct DragDropPayload {
    QPoint offset;                                ///< Drag-start offset from the dragged item's origin.
    ElementType type = ElementType::Unknown;       ///< Element type being dragged.
    QString icFileName;                           ///< File-based IC path (empty if embedded).
    bool isEmbedded = false;                       ///< True if this references an embedded IC blob.
    QString blobName;                              ///< Embedded IC blob name (empty if file-based).
};

/**
 * \brief Reads a drag-and-drop payload from \a stream.
 * \param stream Stream positioned just past \c Serialization::readPandaHeader().
 * \return The decoded payload.
 *
 * \details Bounds \c icFileName and \c blobName via \c Serialization::readBoundedString() —
 * the drag source is not a trusted process (any application can offer wiRedPanda's public MIME
 * type with a crafted payload), so these must not use Qt's raw, unbounded
 * \c QDataStream::operator>>(QString&). The optional trailing \c isEmbedded/\c blobName fields
 * are read only if present, preserving compatibility with payloads that predate them.
 */
DragDropPayload readDragDropPayload(QDataStream &stream);
