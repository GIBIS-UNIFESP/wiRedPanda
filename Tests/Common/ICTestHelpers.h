// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDataStream>
#include <QFile>
#include <QMap>
#include <QRectF>
#include <QVariant>

#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/Scene/ICRegistry.h"

namespace ICTestHelpers {

inline QByteArray readFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        return {};
    }
    return f.readAll();
}

/// A minimal, valid .panda payload (header + empty-element metadata) -- enough for
/// IC::loadFile()/loadFromBlob() to accept as real IC file content.
inline QByteArray minimalPandaBytes()
{
    QMap<QString, QVariant> metadata;
    metadata["dolphinFileName"] = QString();
    metadata["sceneRect"] = QRectF();

    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);
    payloadStream << metadata;

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    Serialization::writePayload(stream, payload);
    return data;
}

/// Stores the raw .panda file bytes as a blob and loads the IC.
inline void embedIC(IC *ic, const QByteArray &pandaBytes, const QString &blobName,
                    const QString &contextDir, ICRegistry *reg)
{
    reg->setBlob(blobName, pandaBytes);
    ic->setBlobName(blobName);
    ic->loadFromBlob(pandaBytes, contextDir);
}

} // namespace ICTestHelpers
