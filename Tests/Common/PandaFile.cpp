// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Common/PandaFile.h"

#include <algorithm>
#include <array>
#include <limits>
#include <new>
#include <optional>
#include <utility>
#include <zlib.h>

#include <QBitArray>
#include <QBuffer>
#include <QDataStream>
#include <QGraphicsItem>
#include <QHash>
#include <QKeySequence>
#include <QList>
#include <QMap>
#include <QMetaType>
#include <QStringList>
#include <QtEndian>
#include <QVariant>
#include <QVersionNumber>

#include "App/Element/ElementTypeDescriptor.h"
#include "App/Versions.h"

namespace {

/// The file's magic number for a circuit, as the writer stores it ("WPCF").
constexpr quint32 kCircuitMagic = 0x57504346;

/// The item tags the writer puts before each element and connection record.
constexpr int kElementTag = QGraphicsItem::UserType + 3;
constexpr int kConnectionTag = QGraphicsItem::UserType + 2;

/// The serial id of a port, as the writer computes it: the element's id above sixteen bits, the port's index below.
quint64 serialId(const int elementId, const int index)
{
    return (static_cast<quint64>(elementId) << 16) | (static_cast<quint64>(index) & 0xFFFF);
}

/// Whether the record of \a type carries parameters: the writer skips them for a type with no keyed parameter.
bool carriesParameters(const ElementType type)
{
    const auto &schema = ElementTypeDescriptor::of(type).parameters;
    return std::any_of(schema.cbegin(), schema.cend(), [](const ParamSpec &spec) { return spec.key != nullptr; });
}

/// A pin of one side: its serial id and the position of its element.
struct Pin
{
    quint64 key = 0;
    QPointF pos;
};

/// Whether \a a comes before \a b by position: top to bottom, then left to right, as the app orders pins.
bool precedes(const Pin &a, const Pin &b)
{
    if (a.pos == b.pos) {
        return false;
    }
    return (a.pos.y() < b.pos.y()) || (qFuzzyCompare(a.pos.y(), b.pos.y()) && (a.pos.x() < b.pos.x()));
}

/// The payload \a raw holds: qCompress()'s four-byte big-endian size, then the zlib stream, which must fill exactly the
/// size its header gives. A stream that ends early is refused here, not by qUncompress(), which reaches undefined
/// behaviour on one until Qt 6.11.3 and 6.12.0 (QTBUG-150081).
std::optional<QByteArray> inflatePayload(const QByteArray &raw)
{
    constexpr quint32 kMaxPayload = 1u << 30;
    if (raw.size() < 4) {
        return std::nullopt;
    }
    const quint32 expected = qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(raw.constData()));
    const QByteArray compressed = raw.mid(4);
    if ((expected > kMaxPayload) || std::cmp_greater(compressed.size(), std::numeric_limits<uInt>::max())) {
        return std::nullopt;
    }

    QByteArray decompressed(static_cast<qsizetype>(expected), Qt::Uninitialized);
    z_stream zs = {};
    if (inflateInit(&zs) != Z_OK) {
        return std::nullopt;
    }
    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(compressed.constData()));
    zs.avail_in = static_cast<uInt>(compressed.size());
    zs.next_out = reinterpret_cast<Bytef *>(decompressed.data());
    zs.avail_out = expected;
    const int result = inflate(&zs, Z_FINISH);
    inflateEnd(&zs);
    if ((result != Z_STREAM_END) || (zs.avail_out != 0)) {
        return std::nullopt;
    }
    return decompressed;
}

/// The length QDataStream writes for a null string or array, in place of its length.
constexpr quint32 kNullLength = 0xFFFFFFFFu;

/// The quint32 that comes next in \a in, read without consuming it. Empty when the stream holds fewer than four bytes,
/// or has already failed.
std::optional<quint32> peekLength(QDataStream &in)
{
    if (in.status() != QDataStream::Ok) {
        return std::nullopt;
    }
    char bytes[4] = {};
    if (in.device()->peek(bytes, 4) != 4) {
        return std::nullopt;
    }
    return qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(bytes));
}

/// Marks \a in as failed, the way a read of a corrupt stream does, so the readers after it take nothing.
void failStream(QDataStream &in)
{
    in.setStatus(QDataStream::ReadCorruptData);
}

/// Whether a string or array of \a length, its length prefix included, fits in the bytes left in \a in. A null one has
/// no bytes behind its prefix, so it always fits.
bool fitsInStream(QDataStream &in, const quint32 length)
{
    return (length == kNullLength) || (static_cast<qint64>(length) + 4 <= in.device()->bytesAvailable());
}

/// Reads a QString, refusing a length the bytes left cannot hold before the string allocates for it.
QString readBoundedString(QDataStream &in)
{
    const auto length = peekLength(in);
    if (!length || !fitsInStream(in, *length)) {
        failStream(in);
        return {};
    }
    QString value;
    in >> value;
    return value;
}

/// Reads a QByteArray, refusing a length the bytes left cannot hold before the array allocates for it.
QByteArray readBoundedByteArray(QDataStream &in)
{
    const auto length = peekLength(in);
    if (!length || !fitsInStream(in, *length)) {
        failStream(in);
        return {};
    }
    QByteArray value;
    in >> value;
    return value;
}

/// Reads a QBitArray: its bit count, then the bits packed into whole bytes, refused when the bytes left cannot hold
/// them.
QBitArray readBoundedBitArray(QDataStream &in)
{
    const auto bitCount = peekLength(in);
    if (!bitCount || ((static_cast<qint64>(*bitCount) + 7) / 8 + 4 > in.device()->bytesAvailable())) {
        failStream(in);
        return {};
    }
    QBitArray bits;
    in >> bits;
    return bits;
}

/// Reads a QKeySequence: a count of at most four key combinations, then one int each. Refused when the count is larger
/// or the bytes left cannot hold it.
QKeySequence readBoundedKeySequence(QDataStream &in)
{
    constexpr quint32 kMaxCombinations = 4;
    const auto count = peekLength(in);
    if (!count || (*count > kMaxCombinations) ||
        (static_cast<qint64>(*count) * 4 + 4 > in.device()->bytesAvailable())) {
        failStream(in);
        return {};
    }
    QKeySequence sequence;
    in >> sequence;
    return sequence;
}

/// Reads a QStringList. Each string takes at least four bytes, its length, so a count the bytes left cannot hold is
/// refused before the list allocates for it, and each string is then bounded as it is read.
QStringList readBoundedStringList(QDataStream &in)
{
    const auto count = peekLength(in);
    if (!count || (static_cast<qint64>(*count) * 4 + 4 > in.device()->bytesAvailable())) {
        failStream(in);
        return {};
    }
    in.skipRawData(4);
    QStringList list;
    for (quint32 index = 0; index < *count && in.status() == QDataStream::Ok; ++index) {
        list.append(readBoundedString(in));
    }
    return list;
}

/// The fixed-size kinds of QVariant that Qt reads whole, so the stream allocates nothing for their data. Every other
/// kind not taken by readBoundedVariant() is refused.
bool isFixedSizeKind(const quint32 typeId)
{
    static constexpr std::array<quint32, 16> kinds{{
        quint32(QMetaType::Bool),      quint32(QMetaType::Int),
        quint32(QMetaType::UInt),      quint32(QMetaType::LongLong),
        quint32(QMetaType::ULongLong), quint32(QMetaType::Double),
        quint32(QMetaType::Float),     quint32(QMetaType::QChar),
        quint32(QMetaType::QDate),     quint32(QMetaType::QTime),
        quint32(QMetaType::QRect),     quint32(QMetaType::QRectF),
        quint32(QMetaType::QSize),     quint32(QMetaType::QSizeF),
        quint32(QMetaType::QPoint),    quint32(QMetaType::QPointF),
    }};
    return std::find(kinds.cbegin(), kinds.cend(), typeId) != kinds.cend();
}

/// Reads one QVariant. Its wire format is a type id, a null flag, then the data, and Qt reads the data even when the
/// flag is set, so the variable-size kinds are read here, bounded, whatever the flag. A kind the writer does not store
/// in an element or in the metadata, a list or a map among them (each nests variants of its own), is refused.
QVariant readBoundedVariant(QDataStream &in)
{
    // QKeySequence's type id as a Qt 5 file and a Qt 6 file write it, as the app's loader reads both
    constexpr quint32 kKeySequenceQt5 = 75;
    constexpr quint32 kKeySequenceQt6 = quint32(QMetaType::QKeySequence);

    // The type id and the null flag, peeked so that a fixed-size kind is left for Qt to read whole
    char header[5] = {};
    if (in.status() != QDataStream::Ok || in.device()->peek(header, 5) != 5) {
        failStream(in);
        return {};
    }
    const quint32 typeId = qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(header));
    const auto skipHeader = [&in]() { in.skipRawData(5); };

    if (typeId == quint32(QMetaType::QString)) {
        skipHeader();
        return QVariant::fromValue(readBoundedString(in));
    }
    if (typeId == quint32(QMetaType::QByteArray)) {
        skipHeader();
        return QVariant::fromValue(readBoundedByteArray(in));
    }
    if (typeId == quint32(QMetaType::QBitArray)) {
        skipHeader();
        return QVariant::fromValue(readBoundedBitArray(in));
    }
    if (typeId == kKeySequenceQt5 || typeId == kKeySequenceQt6) {
        skipHeader();
        return QVariant::fromValue(readBoundedKeySequence(in));
    }
    if (typeId == quint32(QMetaType::QStringList)) {
        skipHeader();
        return QVariant::fromValue(readBoundedStringList(in));
    }
    if (isFixedSizeKind(typeId)) {
        QVariant value;
        in >> value;
        return value;
    }
    failStream(in);
    return {};
}

/// Reads a QMap<QString, QVariant>. Each entry takes at least nine bytes (an empty key's length, a variant's type id
/// and its null flag), so a count the bytes left cannot hold is refused before the map reads any entry.
QMap<QString, QVariant> readBoundedMap(QDataStream &in)
{
    QMap<QString, QVariant> map;
    const auto count = peekLength(in);
    if (!count || (static_cast<qint64>(*count) * 9 + 4 > in.device()->bytesAvailable())) {
        failStream(in);
        return map;
    }
    in.skipRawData(4);
    for (quint32 index = 0; index < *count && in.status() == QDataStream::Ok; ++index) {
        QString key = readBoundedString(in);
        QVariant value = readBoundedVariant(in);
        if (in.status() == QDataStream::Ok) {
            map.insert(std::move(key), std::move(value));
        }
    }
    return map;
}

/// Reads a QList of maps. Each map takes at least four bytes, its count, so a list count the bytes left cannot hold is
/// refused before the list allocates for it, and each map is then bounded as it is read.
QList<QMap<QString, QVariant>> readBoundedMapList(QDataStream &in)
{
    QList<QMap<QString, QVariant>> maps;
    const auto count = peekLength(in);
    if (!count || (static_cast<qint64>(*count) * 4 + 4 > in.device()->bytesAvailable())) {
        failStream(in);
        return maps;
    }
    in.skipRawData(4);
    for (quint32 index = 0; index < *count && in.status() == QDataStream::Ok; ++index) {
        maps.append(readBoundedMap(in));
    }
    return maps;
}

/// Reads one element record, or returns nothing when the stream does not hold one.
std::optional<PandaElement> readElement(QDataStream &in)
{
    quint64 typeValue = 0;
    in >> typeValue;
    const auto type = static_cast<ElementType>(static_cast<int>(typeValue));
    // The type's descriptor says whether the record carries parameters and which group the type is in: a type it does
    // not know has no descriptor, so the file is refused before any descriptor is looked up for it
    if (!ElementTypeDescriptor::has(type)) {
        return std::nullopt;
    }

    const QMap<QString, QVariant> map = readBoundedMap(in);
    const QList<QMap<QString, QVariant>> inputs = readBoundedMapList(in);
    const QList<QMap<QString, QVariant>> outputs = readBoundedMapList(in);
    const QList<QMap<QString, QVariant>> appearances = readBoundedMapList(in);
    if (in.status() != QDataStream::Ok) {
        return std::nullopt;
    }

    PandaElement element;
    element.type = type;
    element.id = map.value(QStringLiteral("id")).toInt();
    element.pos = map.value(QStringLiteral("pos")).toPointF();
    element.label = map.value(QStringLiteral("label")).toString();
    if (carriesParameters(element.type)) {
        element.parameters = readBoundedMap(in);
    }
    if (element.type == ElementType::IC) {
        const QMap<QString, QVariant> icMap = readBoundedMap(in);
        element.icName = icMap.value(QStringLiteral("name")).toString();
    }
    if (in.status() != QDataStream::Ok) {
        return std::nullopt;
    }

    // A port's serial id follows its element's id and its index among the element's ports, outputs after inputs
    for (qsizetype index = 0; index < inputs.size(); ++index) {
        element.inputKeys.append(serialId(element.id, static_cast<int>(index)));
    }
    for (qsizetype index = 0; index < outputs.size(); ++index) {
        element.outputKeys.append(serialId(element.id, static_cast<int>(inputs.size() + index)));
    }
    return element;
}

} // namespace

QVector<quint64> PandaFile::pinOrder(const bool inputs) const
{
    const ElementGroup group = inputs ? ElementGroup::Input : ElementGroup::Output;

    // By position, a stable sort keeping one element's pins in port order
    QVector<Pin> positional;
    for (const PandaElement &element : elements) {
        if (ElementTypeDescriptor::of(element.type).group != group) {
            continue;
        }
        for (const quint64 key : (inputs ? element.outputKeys : element.inputKeys)) {
            positional.append({key, element.pos});
        }
    }
    std::stable_sort(positional.begin(), positional.end(), precedes);

    // The saved order names pins by serial id; a name no pin has is skipped, and a name given twice takes its pin once
    const QStringList saved = metadata.value(inputs ? QStringLiteral("inputOrder") : QStringLiteral("outputOrder"))
                                  .toStringList();
    QHash<quint64, int> indexOf;
    for (int index = 0; index < positional.size(); ++index) {
        indexOf.insert(positional.at(index).key, index);
    }
    QVector<bool> taken(positional.size(), false);
    QVector<Pin> result;
    for (const QString &name : saved) {
        const auto found = indexOf.constFind(name.toULongLong());
        if (found == indexOf.cend() || taken[found.value()]) {
            continue;
        }
        taken[found.value()] = true;
        result.append(positional.at(found.value()));
    }

    // A pin the saved order does not name goes before the first pin it precedes, taking its place by position
    for (int index = 0; index < positional.size(); ++index) {
        if (taken[index]) {
            continue;
        }
        const Pin &pin = positional.at(index);
        const auto place = std::find_if(result.begin(), result.end(), [&pin](const Pin &other) {
            return precedes(pin, other);
        });
        result.insert(place, pin);
    }

    QVector<quint64> keys;
    keys.reserve(result.size());
    for (const Pin &pin : std::as_const(result)) {
        keys.append(pin.key);
    }
    return keys;
}

namespace {

/// Reads the records of an already-inflated payload. Every length is checked against the bytes left before anything
/// allocates for it, nested ones included, so a corrupt file is refused rather than sized by its own bytes.
std::optional<PandaFile> parsePayload(const QByteArray &payload)
{
    QDataStream in(payload);
    in.setVersion(QDataStream::Qt_5_12);
    PandaFile file;
    file.metadata = readBoundedMap(in);
    if (in.status() != QDataStream::Ok) {
        return std::nullopt;
    }
    while (!in.atEnd()) {
        int tag = 0;
        in >> tag;
        if (tag == kElementTag) {
            auto element = readElement(in);
            if (!element) {
                return std::nullopt;
            }
            file.elements.append(*element);
        } else if (tag == kConnectionTag) {
            const QMap<QString, QVariant> map = readBoundedMap(in);
            PandaConnection connection;
            connection.startKey = map.value(QStringLiteral("startPortId")).toULongLong();
            connection.endKey = map.value(QStringLiteral("endPortId")).toULongLong();
            file.connections.append(connection);
        } else {
            return std::nullopt;
        }
        if (in.status() != QDataStream::Ok) {
            return std::nullopt;
        }
    }
    return file;
}

} // namespace

std::optional<PandaFile> readPandaFile(const QByteArray &bytes)
{
    // The header is the magic number and the format revision, uncompressed; the payload follows it, compressed
    QBuffer buffer;
    buffer.setData(bytes);
    buffer.open(QIODevice::ReadOnly);
    QDataStream header(&buffer);
    header.setVersion(QDataStream::Qt_5_12);
    quint32 magic = 0;
    QVersionNumber version;
    header >> magic >> version;
    if ((header.status() != QDataStream::Ok) || (magic != kCircuitMagic) || (version != FormatRev::current)) {
        return std::nullopt;
    }

    const std::optional<QByteArray> payload = inflatePayload(buffer.readAll());
    if (!payload || payload->isEmpty()) {
        return std::nullopt;
    }

    try {
        return parsePayload(*payload);
    } catch (const std::bad_alloc &) {
        return std::nullopt;
    }
}
