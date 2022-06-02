// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "serializationfunctions.h"

#include "common.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "ic.h"
#include "icmanager.h"
#include "qneconnection.h"

#include <QApplication>

void SerializationFunctions::saveHeader(QDataStream &stream, const QString &dolphinFileName, const QRectF &rect)
{
    stream << QApplication::applicationName() + " " + QString::number(GlobalProperties::version);
    stream << dolphinFileName;
    stream << rect;
}

void SerializationFunctions::serialize(const QList<QGraphicsItem *> &items, QDataStream &stream)
{
    for (auto *item : items) {
        if (auto *element = qgraphicsitem_cast<GraphicElement *>(item)) {
            stream << element;
        }
    }

    for (auto *item : items) {
        if (auto *connection = qgraphicsitem_cast<QNEConnection *>(item)) {
            stream << connection;
        }
    }
}

QList<QGraphicsItem *> SerializationFunctions::deserialize(QDataStream &stream, const double version, QMap<quint64, QNEPort *> portMap)
{
    QList<QGraphicsItem *> itemList;

    while (!stream.atEnd()) {
        int type;
        stream >> type;
        qCDebug(three) << "Type:" << type;

        if (type != GraphicElement::Type && type != QNEConnection::Type) {
            throw Pandaception(tr("Invalid type. Data is possibly corrupted."));
        }

        if (type == GraphicElement::Type) {
            ElementType elmType;
            stream >> elmType;

            auto *elm = ElementFactory::buildElement(elmType);
            itemList.append(elm);
            elm->load(stream, portMap, version);

            if (elm->elementType() == ElementType::IC) {
                qCDebug(three) << "Loading IC.";
                IC *ic = qgraphicsitem_cast<IC *>(elm);
                ICManager::loadIC(ic, ic->file());
            }

            elm->setSelected(true);
        }

        if (type == QNEConnection::Type) {
            qCDebug(three) << "Reading Connection.";
            QNEConnection *conn = ElementFactory::buildConnection();
            qCDebug(three) << "Connection built.";
            conn->setSelected(true);
            qCDebug(three) << "Selected true.";
            conn->load(stream, portMap);
            qCDebug(three) << "Appending connection.";
            itemList.append(conn);
        }
    }

    qCDebug(zero) << "Finished deserializing.";
    return itemList;
}

double SerializationFunctions::loadVersion(QDataStream &stream)
{
    qCDebug(zero) << "Loading version.";
    QString str;
    stream >> str;
    if (!str.startsWith(QApplication::applicationName(), Qt::CaseInsensitive)) {
        throw Pandaception(tr("Invalid file format."));
    }
    qCDebug(zero) << "String:" << str;
    bool ok;
    double version = str.remove(QApplication::applicationName(), Qt::CaseInsensitive).toDouble(&ok);
    qCDebug(zero) << "Version:" << version;
    if (!ok) {
        throw Pandaception(tr("Invalid version number."));
    }
    return version;
}

QString SerializationFunctions::loadDolphinFileName(QDataStream &stream, const double version)
{
    QString str = "";
    if (version >= 3.0) {
        stream >> str;
        if ((version < 3.3) && (str == "none")) {
            str.clear();
        }
    }
    return str;
}

QRectF SerializationFunctions::loadRect(QDataStream &stream, const double version)
{
    QRectF rect;
    if (version >= 1.4) {
        stream >> rect;
    }
    return rect;
}

QList<QGraphicsItem *> SerializationFunctions::load(QDataStream &stream)
{
    qCDebug(zero) << "Started loading file.";
    QString str;
    stream >> str;
    if (!str.startsWith(QApplication::applicationName(), Qt::CaseInsensitive)) {
        throw Pandaception(tr("Invalid file format."));
    }
    bool ok;
    const double version = str.remove(QApplication::applicationName(), Qt::CaseInsensitive).toDouble(&ok);
    if (!ok) {
        throw Pandaception(tr("Invalid version number."));
    }
    loadDolphinFileName(stream, version);
    loadRect(stream, version);
    qCDebug(zero) << "Header Ok. Version:" << version;
    auto items = deserialize(stream, version);
    qCDebug(zero) << "Finished reading items.";
    return items;
}
