// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Common/PandaFile.h"

#include <algorithm>
#include <limits>
#include <utility>
#include <zlib.h>

#include <QBuffer>
#include <QDataStream>
#include <QGraphicsItem>
#include <QHash>
#include <QList>
#include <QStringList>
#include <QtEndian>
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

    QMap<QString, QVariant> map;
    QList<QMap<QString, QVariant>> inputs;
    QList<QMap<QString, QVariant>> outputs;
    QList<QMap<QString, QVariant>> appearances;
    in >> map >> inputs >> outputs >> appearances;

    PandaElement element;
    element.type = type;
    element.id = map.value(QStringLiteral("id")).toInt();
    element.pos = map.value(QStringLiteral("pos")).toPointF();
    element.label = map.value(QStringLiteral("label")).toString();
    if (carriesParameters(element.type)) {
        in >> element.parameters;
    }
    if (element.type == ElementType::IC) {
        QMap<QString, QVariant> icMap;
        in >> icMap;
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

    QDataStream in(*payload);
    in.setVersion(QDataStream::Qt_5_12);
    PandaFile file;
    in >> file.metadata;
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
            QMap<QString, QVariant> map;
            in >> map;
            if (in.status() != QDataStream::Ok) {
                return std::nullopt;
            }
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
