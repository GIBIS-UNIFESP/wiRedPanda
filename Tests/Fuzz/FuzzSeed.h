// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Writes the circuits the fuzz harnesses seed themselves with, and checks them at start-up.
 *
 * A harness whose seed the reader rejects fails silently: its target catches every rejection, so the run looks
 * clean while no input reaches the code under test. pandaFile() writes a seed as WorkSpace::save() writes a file,
 * and requireAccepted() aborts the harness, naming it and the reason, when the reader does not accept a seed.
 */

#pragma once

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <functional>

#include <QByteArray>
#include <QDataStream>
#include <QGraphicsItem>
#include <QIODevice>
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QVersionNumber>

#include "App/IO/Serialization.h"
#include "App/IO/VersionInfo.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"

namespace FuzzSeed {

/// A .panda file as WorkSpace::save() writes one: the header, then \a metadata and what \a writeItems writes,
/// compressed together as one payload.
inline QByteArray pandaFile(const QMap<QString, QVariant> &metadata,
                            const std::function<void(QDataStream &)> &writeItems)
{
    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);
    payloadStream << metadata;
    writeItems(payloadStream);

    QByteArray file;
    QDataStream stream(&file, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    Serialization::writePayload(stream, payload);
    return file;
}

/// A .panda file holding \a metadata and \a items, serialized as a portable file's.
inline QByteArray pandaFile(const QMap<QString, QVariant> &metadata, const QList<QGraphicsItem *> &items)
{
    return pandaFile(metadata, [&items](QDataStream &stream) {
        Serialization::serialize(items, stream, {.purpose = SerializationPurpose::PortableFile});
    });
}

/// Aborts the process, naming \a harness and the reason, unless the reader accepts \a seed: its header, its
/// compressed payload and its metadata must decode, and the circuit must load, into a scratch workspace, with at
/// least one element.
inline void requireAccepted(const QByteArray &seed, const char *harness)
{
    const auto reject = [harness](const char *reason, const char *detail = "") {
        std::fprintf(stderr, "%s: the seed is not accepted by the reader: %s%s\n", harness, reason, detail);
        std::abort();
    };

    // readPandaHeader() throws on a header it does not recognize, an empty seed's among them
    QDataStream stream(seed);
    QVersionNumber version;
    try {
        version = Serialization::readPandaHeader(stream);
    } catch (const std::exception &error) {
        reject("the header does not decode: ", error.what());
    }
    if (!VersionInfo::hasCompressedPayload(version)) {
        reject("the header names a version without a compressed payload");
    }

    QByteArray payload;
    try {
        payload = Serialization::readPayload(stream, version);
    } catch (const std::exception &error) {
        reject("the payload does not decompress: ", error.what());
    }

    QDataStream payloadStream(payload);
    payloadStream.setVersion(QDataStream::Qt_5_12);
    try {
        Serialization::readBoundedMetadata(payloadStream);
    } catch (const std::exception &error) {
        reject("the metadata does not decode: ", error.what());
    }
    if (payloadStream.status() != QDataStream::Ok) {
        reject("the metadata does not decode");
    }

    // The circuit, loaded as the application loads a file
    QDataStream loadStream(seed);
    WorkSpace workspace;
    try {
        const QVersionNumber loadVersion = Serialization::readPandaHeader(loadStream);
        workspace.load(loadStream, loadVersion, QString());
    } catch (const std::exception &error) {
        reject("loading it throws: ", error.what());
    }
    if (workspace.scene()->elementCount() == 0) {
        reject("the circuit loads with no elements");
    }
}

} // namespace FuzzSeed
