/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SERIALIZATIONFUNCTIONS_H
#define SERIALIZATIONFUNCTIONS_H

#include "qneport.h"

class QGraphicsItem;
class Editor;

class SerializationFunctions
{
public:
    static bool update(QString &fileName, QString icDirName);
    static void serialize(const QList<QGraphicsItem *> &items, QDataStream &ds);
    static QList<QGraphicsItem *>
    deserialize(QDataStream &ds, double version, QString parentFile, QMap<quint64, QNEPort *> portMap = QMap<quint64, QNEPort *>());
    static QList<QGraphicsItem *> load(QDataStream &ds, QString parentFile);
    static double loadVersion(QDataStream &ds);
    static QRectF loadRect(QDataStream &ds, double version);
    static QString loadDolphinFilename(QDataStream &ds, double version);

private:
    static QList<QGraphicsItem *> loadMoveData(QString icDirName, QDataStream &ds, double version);
};

#endif /* SERIALIZATIONFUNCTIONS_H */
