/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QMap>
#include <QRectF>
#include <QString>

class QGraphicsItem;
class Editor;
class QNEPort;
class Scene;

class SerializationFunctions
{
public:
    /**
     * @brief saveHeader: Saves the following header information into the panda file, in this order: app name, app version, associated dolphin filename, scene rectangle.
     */
    static void saveHeader(QDataStream &ds, const QString &dolphinFilename, const QRectF &rect);
    /**
     * @brief serialize: Serializes the list of QGraphicItems through a binary data stream.
     */
    static void serialize(const QList<QGraphicsItem *> &items, QDataStream &ds);
    /**
     * @brief deserialize: Deserializes a list of QGraphicItems coming through a binary data stream. It stops at the end of the stream.
     * @param itemList is used to return the list of deserialized items.
     * @param portMap is used to return a map of all input and output ports. This mapping may be used to check and to create connections between element ports.
     */
    static bool deserialize(QDataStream &ds, QList<QGraphicsItem *> &itemList, double version, QMap<quint64, QNEPort *> portMap = QMap<quint64, QNEPort *>());
    /**
     * @brief load: Loads a .panda file project. The procedure includes loading and checking file header information, canvas status, and deserializing the graphical elements through a binary data stream.
     * @param parentFile is the name of the parent file of the current project. It is used as a basis to search for ICs so that it is possible to load them.
     */
    static bool load(QDataStream &ds, QList<QGraphicsItem *> items);
    /**
     * @brief loadVersion Checks if it is a WiRedPanda project file and reads its version.
     * @throws std::runtime_error if it is not a valid WiRedPanda project file.
     */
    static double loadVersion(QDataStream &ds);
    /**
     * @brief loadRect: returns the canvas pose from the last saved session.
     */
    static QRectF loadRect(QDataStream &ds, double version);
    static QString loadDolphinFilename(QDataStream &ds, double version);
};

