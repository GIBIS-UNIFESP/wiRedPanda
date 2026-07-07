// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/IO/Serialization.h"

#include <array>
#include <cstring>
#include <limits>
#include <utility>

#include <QApplication>
#include <QBitArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QKeySequence>
#include <QMetaType>
#include <QScopeGuard>
#include <QSysInfo>
#include <QtEndian>
#include <QVarLengthArray>
#include <QVariant>

#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/VersionInfo.h"
#include "App/Wiring/Connection.h"

namespace {

/// Reads a QVersionNumber from \a stream without calling QList::reserve() with a
/// fuzz-controlled segment count.  Qt's built-in operator>> deserialises the
/// internal segment list via QList<int>::reserve(n), which OOMs on a crafted n.
/// Caps at 8 segments — no real release version has more than 3 (e.g. "4.6.2").
QVersionNumber readVersionNumber(QDataStream &stream)
{
    quint32 count;
    stream >> count;
    if (count > 8) {
        throw PANDACEPTION_WITH_CONTEXT("Serialization",
                                        "Implausible version segment count %1 — stream may be corrupt",
                                        QString::number(count));
    }
    QList<int> segs;
    segs.reserve(static_cast<int>(count));
    for (quint32 i = 0; i < count; ++i) {
        int v; stream >> v;
        segs.append(v);
    }
    return QVersionNumber(segs);
}

/// Returns the bytes still readable from \a stream's device.
/// Sequential devices (pipes, sockets) return -1; we treat that as "no bound"
/// so the callers below never block legitimate sequential use, while still
/// catching the crafted-file OOM on the file-backed streams wiRedPanda uses.
qint64 bytesAvailable(const QDataStream &stream)
{
    if (!stream.device()) return 0;
    const qint64 n = stream.device()->bytesAvailable();
    return n < 0 ? std::numeric_limits<qint64>::max() / 2 : n;
}

/// Peeks the leading quint32 of \a stream without consuming it.
/// Returns false if fewer than 4 bytes are available.
bool peekU32(QDataStream &stream, quint32 &out)
{
    char buf[4];
    if (stream.device()->peek(buf, 4) != 4) return false;
    out = qFromBigEndian<quint32>(buf);
    return true;
}

/// Reads a QString from \a stream, rejecting byte counts that exceed the
/// remaining readable bytes before Qt's resize() allocates.
/// Delegates the actual deserialization to Qt's operator>> so that endianness
/// and encoding are handled correctly for every stream version.
QString readBoundedString(QDataStream &stream)
{
    quint32 byteLen = 0;
    if (peekU32(stream, byteLen)) {
        // 0xFFFFFFFF = null sentinel, 0 = empty — no allocation needed, skip check.
        if (byteLen != 0xFFFFFFFFu && byteLen != 0) {
            // byteLen bytes of data plus 4 bytes for the length field itself
            if (static_cast<qint64>(byteLen) + 4 > bytesAvailable(stream))
                throw PANDACEPTION_WITH_CONTEXT("Serialization",
                                                "QString byte count %1 exceeds remaining stream bytes",
                                                QString::number(byteLen));
        }
    }
    QString result;
    stream >> result;
    return result;
}

/// Reads a QByteArray from \a stream, rejecting sizes that exceed the
/// remaining readable bytes before Qt's resize() allocates.
QByteArray readBoundedByteArray(QDataStream &stream)
{
    quint32 len = 0;
    if (peekU32(stream, len)) {
        if (len != 0xFFFFFFFFu && len != 0) {
            if (static_cast<qint64>(len) + 4 > bytesAvailable(stream))
                throw PANDACEPTION_WITH_CONTEXT("Serialization",
                                                "QByteArray length %1 exceeds remaining stream bytes",
                                                QString::number(len));
        }
    }
    QByteArray result;
    stream >> result;
    return result;
}

/// Reads a QStringList from \a stream, bounding the element count by
/// bytesAvailable() before Qt's reserve() allocates the full list.
QStringList readBoundedStringList(QDataStream &stream)
{
    quint32 count = 0;
    if (peekU32(stream, count) && count > 0) {
        // Each serialised entry is at least 4 bytes (the empty-string quint32 header);
        // subtract 4 for the count field itself which is still in the stream.
        if (static_cast<qint64>(count) > (bytesAvailable(stream) - 4) / 4)
            throw PANDACEPTION_WITH_CONTEXT("Serialization",
                                            "QStringList count %1 implausible given remaining stream bytes",
                                            QString::number(count));
    }
    QStringList result;
    stream >> result;
    return result;
}

/// Reads a QKeySequence (serialised as QList<int>: quint32 count + count*qint32).
/// A key sequence holds at most 4 key combinations, so count > 4 is invalid.
QKeySequence readBoundedKeySequence(QDataStream &stream)
{
    quint32 count = 0;
    if (peekU32(stream, count)) {
        if (count > 4 || static_cast<qint64>(count) * 4 > bytesAvailable(stream))
            throw PANDACEPTION_WITH_CONTEXT("Serialization",
                                            "QKeySequence count %1 out of range — stream may be corrupt",
                                            QString::number(count));
    }
    QKeySequence ks; stream >> ks; return ks;
}

/// Reads a QBitArray (quint32 numBits + ceil(numBits/8) bytes).
QBitArray readBoundedBitArray(QDataStream &stream)
{
    quint32 numBits = 0;
    if (peekU32(stream, numBits) && numBits > 0) {
        const qint64 numBytes = (static_cast<qint64>(numBits) + 7) / 8;
        if (numBytes > bytesAvailable(stream))
            throw PANDACEPTION_WITH_CONTEXT("Serialization",
                                            "QBitArray numBits %1 exceeds stream size — stream may be corrupt",
                                            QString::number(numBits));
    }
    QBitArray ba; stream >> ba; return ba;
}

/// Reads a single QVariant from \a stream, intercepting variable-size types
/// that call QList::reserve() or QArrayData::allocate() with a fuzz-controlled
/// count before bounds-checking against available stream bytes.
///
/// Wire format (QDataStream::Qt_5_12):
///   quint32 typeId  |  quint8 isNull  |  type-specific data
///
/// IMPORTANT: Qt 6.9 reads type-specific data even when isNull != 0 (verified
/// empirically — the isNull byte is stored but does not skip data loading).
/// Variable-size types MUST therefore be intercepted unconditionally, not only
/// when isNull == 0.  Fixed-size types are safe to delegate in all cases.
/// Unknown types are rejected when isNull == 0 to catch corrupt streams.
QVariant readBoundedVariant(QDataStream &stream)
{
    // Peek at typeId + isNull (5 bytes total) without consuming them.
    char hdr[5] = {};
    if (stream.device()->peek(hdr, 5) < 5) {
        QVariant v; stream >> v; return v;
    }

    const quint32 typeId = qFromBigEndian<quint32>(hdr);
    // hdr[4] holds the isNull byte; deliberately unread because Qt 6.9 also reads
    // type-specific data when isNull != 0, so it is not a safe skip-guard (see
    // function header). Still peek 5 bytes so the consumeHeader lambda below
    // re-reads the exact same bytes Qt's QVariant operator>> expects.

    // Consume the 5 header bytes we peeked before dispatching.
    auto consumeHeader = [&]() { char s[5]; stream.readRawData(s, 5); };

    // -----------------------------------------------------------------------
    // Variable-size types: intercept unconditionally (Qt 6.9 reads their data
    // even for null-marked variants, so isNull is not a safe guard here).
    // -----------------------------------------------------------------------

    if (typeId == quint32(QMetaType::QString)) {
        consumeHeader();
        return QVariant::fromValue(readBoundedString(stream));
    }

    // QKeySequence wire typeId is version-dependent:
    //   Qt_5_12 format writes 75  (Qt 5 backward-compat value)
    //   Qt 6 default format writes 4107 (QMetaType::QKeySequence in Qt 6)
    // Files written before V_5_1 used Qt 6 default; afterwards Qt_5_12.
    constexpr quint32 kWireKeySeqQt5 = 75;
    constexpr quint32 kWireKeySeqQt6 = 4107; // quint32(QMetaType::QKeySequence)
    if (typeId == kWireKeySeqQt5 || typeId == kWireKeySeqQt6) {
        consumeHeader();
        return QVariant::fromValue(readBoundedKeySequence(stream));
    }

    if (typeId == quint32(QMetaType::QBitArray)) {
        consumeHeader();
        return QVariant::fromValue(readBoundedBitArray(stream));
    }

    if (typeId == quint32(QMetaType::QStringList) ||
        typeId == quint32(QMetaType::QVariantList) ||
        typeId == quint32(QMetaType::QVariantMap)) {
        consumeHeader();
        if (typeId == quint32(QMetaType::QStringList))
            return QVariant::fromValue(readBoundedStringList(stream));
        // QVariantList and QVariantMap are not used in metadata; reject to
        // avoid nested-container OOM (inner QVariants bypass this reader).
        throw PANDACEPTION_WITH_CONTEXT("Serialization",
                                        "Unsupported container type %1 in metadata — stream may be corrupt",
                                        QString::number(typeId));
    }

    if (typeId == quint32(QMetaType::QByteArray)) {
        consumeHeader();
        return readBoundedByteArray(stream);
    }

    // -----------------------------------------------------------------------
    // Fixed-size built-in types: safe to delegate to Qt unconditionally.
    // -----------------------------------------------------------------------

    static constexpr std::array<quint32, 16> kFixedSizeTypes{{
        quint32(QMetaType::Bool),      quint32(QMetaType::Int),
        quint32(QMetaType::UInt),      quint32(QMetaType::LongLong),
        quint32(QMetaType::ULongLong), quint32(QMetaType::Double),
        quint32(QMetaType::Float),     quint32(QMetaType::QChar),
        quint32(QMetaType::QDate),     quint32(QMetaType::QTime),
        quint32(QMetaType::QRect),     quint32(QMetaType::QRectF),
        quint32(QMetaType::QSize),     quint32(QMetaType::QSizeF),
        quint32(QMetaType::QPoint),    quint32(QMetaType::QPointF),
    }};
    const bool isFixedSize = std::find(kFixedSizeTypes.begin(),
                                       kFixedSizeTypes.end(), typeId) != kFixedSizeTypes.end();

    // Reject all unknown types regardless of isNull.  Qt 6.9 reads type-specific
    // data even for null-marked variants, so unknown types with isNull != 0 are
    // still vulnerable to fuzz-controlled allocation via QArrayData::allocate.
    if (!isFixedSize)
        throw PANDACEPTION_WITH_CONTEXT("Serialization",
                                        "Unsupported metadata value type %1 — stream may be corrupt",
                                        QString::number(typeId));

    QVariant result;
    stream >> result;
    return result;
}

} // namespace

QString Serialization::readBoundedString(QDataStream &stream)
{
    return ::readBoundedString(stream); // delegate to the file-local free function
}

QMap<QString, QVariant> Serialization::readBoundedMetadata(QDataStream &stream)
{
    quint32 count;
    stream >> count;
    if (stream.status() != QDataStream::Ok)
        throw PANDACEPTION("Stream error reading metadata map count");
    // Minimum per entry: 4 bytes (key byteLen=0) + 4 (typeId) + 1 (isNull) = 9 bytes
    if (count > 0 && static_cast<qint64>(count) > bytesAvailable(stream) / 9)
        throw PANDACEPTION("Metadata map count %1 implausible given remaining stream bytes",
                           QString::number(count));
    QMap<QString, QVariant> result;
    for (quint32 i = 0; i < count; ++i) {
        QString  key = readBoundedString(stream);
        QVariant val = readBoundedVariant(stream);
        if (stream.status() != QDataStream::Ok) break;
        result.insert(std::move(key), std::move(val));
    }
    return result;
}

QMap<QString, QByteArray> Serialization::readBoundedBlobMap(QDataStream &stream)
{
    quint32 count;
    stream >> count;
    if (stream.status() != QDataStream::Ok)
        throw PANDACEPTION("Stream error reading blob map count");
    // Minimum per entry: 4 (key byteLen=0) + 4 (value byteLen=0) = 8 bytes
    if (count > 0 && static_cast<qint64>(count) > bytesAvailable(stream) / 8)
        throw PANDACEPTION("Blob map count %1 implausible given remaining stream bytes",
                           QString::number(count));
    QMap<QString, QByteArray> result;
    for (quint32 i = 0; i < count; ++i) {
        QString   key = readBoundedString(stream);
        QByteArray val = readBoundedByteArray(stream);
        if (stream.status() != QDataStream::Ok) break;
        result.insert(std::move(key), std::move(val));
    }
    return result;
}

void Serialization::writePandaHeader(QDataStream &stream)
{
    stream.setVersion(QDataStream::Qt_5_12);
    stream << MAGIC_HEADER_CIRCUIT;
    stream << FormatRev::current;
}

QVersionNumber Serialization::readPandaHeader(QDataStream &stream)
{
    stream.setVersion(QDataStream::Qt_5_12);

    QIODevice *device = stream.device();
    if (!device) {
        throw PANDACEPTION("Invalid file format.");
    }

    // Every speculative check below peeks (never consumes) so a truncated or
    // legacy file can't latch QDataStream::status() to ReadPastEnd — that
    // status is sticky, and device()->seek() rewinding the position does not
    // reset it, so a later legitimate status check would see a stale error
    // from a speculative read here instead of a real one. The live stream is
    // only ever touched once the format is unambiguously confirmed.
    quint32 magicHeader = 0;
    const bool haveMagic = peekU32(stream, magicHeader);

    if (haveMagic && magicHeader == MAGIC_HEADER_CIRCUIT) {
        stream >> magicHeader; // consume the confirmed magic
        return readVersionNumber(stream);
    }

    // Not the modern magic — the same 4 peeked bytes double as the length
    // prefix of a legacy "wiRedPanda X.Y" text header.
    constexpr quint32 NullStringMarker = 0xFFFFFFFFu;
    constexpr qint64 MaxLegacyHeaderBytes = 128;

    if (haveMagic && magicHeader != NullStringMarker && magicHeader <= MaxLegacyHeaderBytes) {
        const qint64 probeLen = sizeof(quint32) + magicHeader;
        QByteArray strProbeBuf(static_cast<int>(probeLen), '\0');
        if (device->peek(strProbeBuf.data(), probeLen) == probeLen) {
            QDataStream strProbe(strProbeBuf);
            strProbe.setVersion(QDataStream::Qt_5_12);
            QString appName;
            strProbe >> appName;
            if (strProbe.status() == QDataStream::Ok && appName.startsWith("wiRedPanda", Qt::CaseInsensitive)) {
                // Confirmed — now safe to consume the header from the live stream.
                QString consumed;
                stream >> consumed;
                const QStringList split = consumed.split(' ');
                if (split.size() < 2) {
                    throw PANDACEPTION("Invalid legacy file header: %1", consumed);
                }
                return QVersionNumber::fromString(split.at(1));
            }
        }
    }

    // Headerless v4.1 clipboard format: the stream starts directly with the
    // scene's center-point QPointF (2 doubles) — no header at all.
    char pointProbeBuf[2 * sizeof(double)];
    if (device->peek(pointProbeBuf, sizeof(pointProbeBuf)) != static_cast<qint64>(sizeof(pointProbeBuf))) {
        throw PANDACEPTION("Invalid file format.");
    }
    QDataStream pointProbe(QByteArray(pointProbeBuf, sizeof(pointProbeBuf)));
    pointProbe.setVersion(QDataStream::Qt_5_12);
    QPointF center;
    pointProbe >> center;
    if (pointProbe.status() != QDataStream::Ok || center.isNull()) {
        throw PANDACEPTION("Invalid file format.");
    }

    // Version 4.1 is the last release that used this headerless format; the
    // live stream is left untouched so the caller reads the QPointF itself next.
    return QVersionNumber(4, 1);
}

void Serialization::writeDolphinHeader(QDataStream &stream)
{
    stream.setVersion(QDataStream::Qt_5_12);
    stream << MAGIC_HEADER_WAVEFORM;
    stream << FormatRev::current;
}

void Serialization::readDolphinHeader(QDataStream &stream)
{
    stream.setVersion(QDataStream::Qt_5_12);

    QIODevice *device = stream.device();
    if (!device) {
        throw PANDACEPTION("Invalid file format.");
    }

    // Same probe-first approach as readPandaHeader: peek (never consume) so a
    // truncated/legacy file's speculative read can't latch a sticky
    // ReadPastEnd status that a later seek() wouldn't reset.
    quint32 magicHeader = 0;
    const bool haveMagic = peekU32(stream, magicHeader);

    if (haveMagic && magicHeader == MAGIC_HEADER_WAVEFORM) {
        stream >> magicHeader; // consume the confirmed magic
        readVersionNumber(stream); // advance stream past version, enforce segment count bound
        return;
    }

    // Legacy beWavedDolphin format: "<byteLen><appName>" as raw UTF-16 code
    // units (byteLen in bytes, so byteLen/2 UTF-16 units). The same 4 peeked
    // bytes double as byteLen. "beWavedDolphin X.Y" is at most ~22 chars =
    // 44 bytes; 256 is a generous cap.
    if (!haveMagic || magicHeader == 0xFFFFFFFFu || magicHeader > 256 || magicHeader % 2 != 0) {
        throw PANDACEPTION("Invalid file format.");
    }
    const qint64 probeLen = sizeof(quint32) + magicHeader;
    QByteArray strProbeBuf(static_cast<int>(probeLen), '\0');
    if (device->peek(strProbeBuf.data(), probeLen) != probeLen) {
        throw PANDACEPTION("Invalid file format.");
    }
    // strProbeBuf's buffer has no 2-byte alignment guarantee, so the UTF-16 code
    // units are copied into a properly-aligned scratch buffer instead of being
    // reinterpret_cast in place — that cast is a strict-aliasing/alignment
    // violation that -Wcast-align catches on armhf/riscv64 (issue #453).
    // Plain byte copy, no endian conversion: unlike the QDataStream-framed
    // "wiRedPanda X.Y" legacy header in readPandaHeader above (which is read
    // back via stream >> QString), this beWavedDolphin field is a raw
    // native-order UTF-16 dump — introducing qFromBigEndian/qFromLittleEndian
    // here, or reusing readPandaHeader's QDataStream-based probe, would change
    // behaviour, not just alignment.
    const int codeUnitCount = static_cast<int>(magicHeader / 2);
    QVarLengthArray<char16_t, 128> appNameBuf(codeUnitCount);
    std::memcpy(appNameBuf.data(), strProbeBuf.constData() + sizeof(quint32),
                static_cast<size_t>(magicHeader));
    const QString appName = QString::fromUtf16(appNameBuf.constData(), codeUnitCount);
    if (!appName.startsWith("beWavedDolphin")) {
        throw PANDACEPTION("Invalid file format.");
    }

    // Confirmed — now safe to consume the header from the live stream.
    quint32 consumedLen = 0;
    stream >> consumedLen;
    QByteArray raw(static_cast<int>(consumedLen), '\0');
    stream.readRawData(raw.data(), static_cast<int>(consumedLen));
}

void Serialization::serialize(const QList<QGraphicsItem *> &items, QDataStream &stream)
{
    // Port serial IDs are computed from the element ID via Port::makeSerialId()
    // (used by both GraphicElementSerializer::save() and ConnectionSerializer::save()).
    // Elements with id=-1 (the unassigned sentinel) would all produce the same keys,
    // causing portMap collisions on the next load and destroying connection topology.
    //
    // Only elements with id <= 0 need a temporary replacement ID. Elements that
    // already have a valid positive scene ID must keep it: callers such as
    // CommandUtils::saveItems() write "other" elements into the same stream
    // before calling serialize(), and reassigning those already-used IDs to
    // items in this list creates portMap key collisions on load.
    //
    // Temp IDs start above the highest positive ID already present in items so
    // they cannot collide with any valid ID in the list.
    QList<std::pair<GraphicElement *, int>> savedIds;
    int localId = 0;
    for (auto *item : items) {
        if (auto *ge = qgraphicsitem_cast<GraphicElement *>(item)) {
            if (ge->id() > localId) {
                localId = ge->id();
            }
        }
    }
    for (auto *item : items) {
        if (auto *ge = qgraphicsitem_cast<GraphicElement *>(item)) {
            if (ge->id() <= 0) {
                savedIds.append({ge, ge->id()});
                ge->setId(++localId);
            }
        }
    }

    // Restore original IDs on scope exit, even if an exception escapes below.
    // Without this guard, a thrown exception would leave elements with sequential
    // IDs instead of their original values, causing ID collisions on scene insertion.
    auto restoreIds = qScopeGuard([&savedIds] {
        for (const auto &[ge, id] : savedIds) {
            ge->setId(id);
        }
    });

    // Elements must be written before connections because deserialization reads
    // elements first to build the port map, then resolves connection endpoints
    // using that map.  Reversing the order would cause every connection load to fail.
    // Serialize all graphic elements (gates, inputs, outputs, ICs)
    // Type tag written here at serialization layer for symmetry with deserialize()
    for (auto *item : items) {
        if (auto *element = qgraphicsitem_cast<GraphicElement *>(item)) {
            stream << static_cast<int>(GraphicElement::Type);
            stream << element->elementType();
            element->save(stream);
        }
    }

    // Serialize all connections (wires)
    // Type tag written here at serialization layer for symmetry with deserialize()
    for (auto *item : items) {
        if (auto *connection = qgraphicsitem_cast<Connection *>(item)) {
            stream << static_cast<int>(Connection::Type);
            connection->save(stream);
        }
    }
}

QList<QGraphicsItem *> Serialization::deserialize(QDataStream &stream, SerializationContext &context)
{
    // portMap maps the raw pointer value (quint64) that was stored at save time to
    // the newly allocated Port object at load time.  Raw pointer values are used
    // as keys because they were the only unique, stable port identity available when
    // the binary format was designed; they are meaningless as pointers after reload
    // but work perfectly as opaque 64-bit tokens for this cross-reference purpose.
    // When portMap is empty (top-level file load), Connection::load() falls back
    // to directly casting the stored integer back to a pointer — only safe during
    // the same process session (e.g., clipboard paste with in-process copy/paste).
    QList<QGraphicsItem *> itemList;
    // Exception safety: if any throw escapes the loop, the guard cleans up every
    // item already constructed.  Callers expect ownership transfer on success only —
    // dismissing the guard at the end of the happy path hands the items to them.
    //
    // Order matters.  Connections must be deleted before elements: an element's
    // InputPort destructor calls Port::drainConnections(true), which
    // delete()s every connection still attached to the port.  If those connections
    // are also in itemList, a naive qDeleteAll would walk onto a freed pointer
    // and double-free.  A connection's destructor only detaches itself from its
    // endpoint ports — safe regardless of element state — so doing connections
    // first is unconditionally the right order.  Surfaced by the libFuzzer
    // harness against malformed .panda input that triggers a mid-loop throw.
    auto cleanupGuard = qScopeGuard([&itemList]() {
        for (qsizetype i = 0; i < itemList.size(); ++i) {
            if (itemList[i] && itemList[i]->type() == Connection::Type) {
                delete itemList[i];
                itemList[i] = nullptr;
            }
        }
        qDeleteAll(itemList);
    });

    while (!stream.atEnd()) {
        int type; stream >> type;

        // Check stream integrity after reading type tag
        if (stream.status() != QDataStream::Ok) {
            throw PANDACEPTION("Stream error reading type tag at position %1: status %2",
                               stream.device()->pos(), static_cast<int>(stream.status()));
        }

        qCDebug(three) << "Type: " << typeName(type);

        switch (type) {
        case GraphicElement::Type: {
            ElementType elmType; stream >> elmType;

            // Check stream integrity after reading element type
            if (stream.status() != QDataStream::Ok) {
                throw PANDACEPTION("Stream error reading element type at position %1: status %2",
                                   stream.device()->pos(), static_cast<int>(stream.status()));
            }

            // Append before load() so cleanupGuard owns the element if load throws
            // (e.g. nested IC whose backing file is missing).
            auto *elm = ElementFactory::buildElement(elmType);
            itemList.append(elm);
            elm->load(stream, context);

            // Check stream integrity after element load
            if (stream.status() != QDataStream::Ok) {
                throw PANDACEPTION("Stream error loading element at position %1: status %2",
                                   stream.device()->pos(), static_cast<int>(stream.status()));
            }

            break;
        }

        case Connection::Type: {
            qCDebug(three) << "Building connection.";
            auto *conn = new Connection();
            // Same as the element case: append first so a throw in conn->load() can't
            // strand the freshly allocated connection.
            itemList.append(conn);

            qCDebug(three) << "Loading connection.";
            conn->load(stream, context);

            // Check stream integrity after connection load
            if (stream.status() != QDataStream::Ok) {
                throw PANDACEPTION("Stream error loading connection at position %1: status %2",
                                   stream.device()->pos(), static_cast<int>(stream.status()));
            }
            break;
        }

        default:
            throw PANDACEPTION("Invalid type. Data is possibly corrupted.");
        }
    }

    qCDebug(zero) << "Finished deserializing.";
    cleanupGuard.dismiss();
    return itemList;
}

QString Serialization::loadDolphinFileName(QDataStream &stream, const QVersionNumber &version)
{
    QString filename;

    if (VersionInfo::hasDolphinFilename(version)) {
        filename = readBoundedString(stream);

        // Versions 3.0–3.2 used the sentinel string "none" instead of an empty
        // QString to indicate that no waveform file was associated; normalize it here
        // so callers can simply check isEmpty()
        if ((!VersionInfo::hasDolphinSentinelFix(version)) && (filename == "none")) {
            filename.clear();
        }
    }

    // Versions before 3.0 didn't store a dolphin filename at all; return empty string
    return filename;
}

QRectF Serialization::loadRect(QDataStream &stream, const QVersionNumber &version)
{
    QRectF rect;

    // The stored rect is always discarded by the caller (WorkSpace recomputes it from
    // items after load), but it must still be read to advance the stream past this field
    if (VersionInfo::hasSceneRect(version)) {
        stream >> rect;
    }

    return rect;
}

void Serialization::createVersionedBackup(const QString &fileName, const QVersionNumber &version)
{
    QFileInfo info(fileName);
    const QString backupName = info.absolutePath() + "/"
        + info.completeBaseName()
        + ".v" + version.toString()
        + "." + info.suffix();

    if (!QFile::exists(backupName)) {
        if (QFile::copy(fileName, backupName)) {
            qCDebug(three) << "Created versioned backup: " << backupName;
        } else {
            throw PANDACEPTION("Failed to create versioned backup: %1", backupName);
        }
    }
}

Serialization::Preamble Serialization::readPreamble(QDataStream &stream)
{
    Preamble result;
    result.version = readPandaHeader(stream);

    if (VersionInfo::hasUnifiedMetadata(result.version)) {
        result.metadata = readBoundedMetadata(stream);
    } else {
        loadDolphinFileName(stream, result.version);
        loadRect(stream, result.version);
        if (VersionInfo::hasMetadata(result.version)) {
            result.metadata = readBoundedMetadata(stream);
        }
    }

    if (stream.status() != QDataStream::Ok) {
        throw PANDACEPTION("Stream error reading preamble: status %1", static_cast<int>(stream.status()));
    }

    return result;
}

QMap<QString, QByteArray> Serialization::deserializeBlobRegistry(const QMap<QString, QVariant> &metadata,
                                                                  const QVersionNumber &fileVersion)
{
    QMap<QString, QByteArray> result;
    const QString key = metadata.contains("embeddedICs") ? QStringLiteral("embeddedICs")
                                                        : QStringLiteral("blobRegistry");
    if (metadata.contains(key)) {
        QByteArray regBytes = metadata.value(key).toByteArray();
        QDataStream regStream(&regBytes, QIODevice::ReadOnly);
        if (VersionInfo::hasVersionedBlobRegistry(fileVersion))
            regStream.setVersion(QDataStream::Qt_5_12);
        // Pre-V_5_0 files: no setVersion — use Qt default matching the build that wrote them.
        regStream >> result;
    }
    return result;
}

void Serialization::serializeBlobRegistry(const QMap<QString, QByteArray> &blobs, QMap<QString, QVariant> &metadata)
{
    if (!blobs.isEmpty()) {
        QByteArray regBytes;
        QDataStream regStream(&regBytes, QIODevice::WriteOnly);
        regStream.setVersion(QDataStream::Qt_5_12);
        regStream << blobs;
        metadata["embeddedICs"] = regBytes;
    }
}

QString Serialization::typeName(const int type) {
    // These offsets must stay in sync with the ::Type enum constants defined in
    // Port, Connection, and GraphicElement (all use QGraphicsItem::UserType + N)
    static const QHash<int, QString> typeMap = {
        { QGraphicsItem::UserType + 1, "Port" },
        { QGraphicsItem::UserType + 2, "Connection" },
        { QGraphicsItem::UserType + 3, "GraphicElement" },
    };

    return typeMap.value(type, "UnknownType");
}

void Serialization::copyPandaFile(const QFileInfo &srcPath, const QFileInfo &destPath,
                                  QSet<QString> *visited)
{
    QSet<QString> ownedVisited;
    if (!visited) {
        visited = &ownedVisited;
    }
    const QString canonical = srcPath.canonicalFilePath();
    if (!canonical.isEmpty()) {
        if (visited->contains(canonical)) {
            return;
        }
        visited->insert(canonical);
    }

    if (!QFile::exists(destPath.absoluteFilePath())) {
        QFile srcFile(srcPath.absoluteFilePath());
        if (!srcFile.copy(destPath.absoluteFilePath())) {
            throw PANDACEPTION("Error copying file: %1", srcFile.errorString());
        }
    }

    // Read the metadata section to find file-backed IC dependencies,
    // then recursively copy each one. No full deserialization needed.
    QFile file(srcPath.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QDataStream stream(&file);
    Preamble preamble;
    try {
        preamble = readPreamble(stream);
    } catch (...) {
        return; // Corrupt or non-panda file — nothing to recurse into
    }
    if (!VersionInfo::hasMetadata(preamble.version)) {
        return;
    }

    const QStringList icFiles = preamble.metadata.value("fileBackedICs").toStringList();
    for (const QString &icFile : icFiles) {
        const QFileInfo icSrc(QDir(srcPath.absolutePath()), icFile);
        const QFileInfo icDest(QDir(destPath.absolutePath()), icFile);
        if (icSrc.exists()) {
            copyPandaFile(icSrc, icDest, visited);
        }
    }
}
