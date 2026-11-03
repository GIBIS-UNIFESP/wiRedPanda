// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>

#include <QByteArray>
#include <QMap>
#include <QPointF>
#include <QString>
#include <QVariant>
#include <QVector>

#include "App/Core/Enums.h"

/// One element record of a .panda file, as the file holds it: its type, id, position, and ports by serial id.
struct PandaElement
{
    ElementType type = ElementType::Unknown;
    int id = 0;
    QPointF pos;
    QString label;
    QMap<QString, QVariant> parameters;  ///< The keyed parameters the file holds, under their keys.
    QString icName;                      ///< An IC: the name its file is given, relative to the file's folder.
    QVector<quint64> inputKeys;          ///< The serial id of each input port, in port order.
    QVector<quint64> outputKeys;         ///< The serial id of each output port, in port order.
};

/// One wire of a .panda file: the serial ids of its two ports.
struct PandaConnection
{
    quint64 startKey = 0;
    quint64 endKey = 0;
};

/**
 * \brief A .panda file read by this reader, which shares no code with the app's loader: the element records, the
 * connections and the metadata, in the order the file holds them.
 * \details The reader follows the format the writer saves today (the current format revision, compressed payload). It
 * reads only what the reference needs: each element's type, id, position, label, keyed parameters, IC name, and the
 * serial ids of its ports, which a connection names. Element records are followed by their type's descriptor, for
 * whether a record carries parameters and whether it is an IC.
 */
struct PandaFile
{
    QMap<QString, QVariant> metadata;
    QVector<PandaElement> elements;
    QVector<PandaConnection> connections;

    /// Returns the serial ids of the pins on one side: the outputs of the input elements (\a inputs), or the inputs of
    /// the output elements. A file's saved order (its inputOrder or outputOrder metadata) comes first, the pins it does
    /// not name follow by position; positions are the elements' as the file holds them, read before any snap.
    [[nodiscard]] QVector<quint64> pinOrder(bool inputs) const;
};

/// Reads the .panda file \a bytes. Empty when the bytes are not a current-format circuit file.
std::optional<PandaFile> readPandaFile(const QByteArray &bytes);
