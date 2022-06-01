/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QCoreApplication>
#include <QMap>

class QGraphicsItem;
class QNEPort;

class SerializationFunctions
{
    Q_DECLARE_TR_FUNCTIONS(SerializationFunctions)

public:
    //! Saves the following header information into the panda file, in this order: app name, app version, associated dolphin fileName, scene rectangle.
    static void saveHeader(QDataStream &stream, const QString &dolphinFileName, const QRectF &rect);

    //! Serializes the list of QGraphicItems through a binary data stream.
    static void serialize(const QList<QGraphicsItem *> &items, QDataStream &stream);

    /**
     * @brief deserialize: Deserializes a list of QGraphicItems coming through a binary data stream. It stops at the end of the stream.
     * @return the list of deserialized items.
     * @param portMap is used to return a map of all input and output ports. This mapping may be used to check and to create connections between element ports.
     */
    static QList<QGraphicsItem *> deserialize(QDataStream &stream, const double version, QMap<quint64, QNEPort *> portMap = {});

    //! Loads a .panda file project. The procedure includes loading and checking file header information, canvas status, and deserializing the graphical elements through a binary data stream.
    static QList<QGraphicsItem *> load(QDataStream &stream);

    //! Checks if it is a WiRedPanda project file and reads its version.
    static double loadVersion(QDataStream &stream);

    //! returns the canvas pose from the last saved session.
    static QRectF loadRect(QDataStream &stream, const double version);
    static QString loadDolphinFileName(QDataStream &stream, const double version);
};
