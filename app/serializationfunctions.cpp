// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "serializationfunctions.h"

#include "common.h"
#include "editor.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "ic.h"
#include "icmanager.h"
#include "qneconnection.h"
#include "qneport.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QMessageBox>
#include <QSaveFile>
#include <iostream>
#include <stdexcept>

void SerializationFunctions::saveHeader(QDataStream &ds, const QString &dolphinFilename, const QRectF &rect)
{
    ds << QApplication::applicationName() + " " + QString::number(GlobalProperties::version);
    ds << dolphinFilename;
    ds << rect;
}

void SerializationFunctions::serialize(const QList<QGraphicsItem *> &items, QDataStream &ds)
{
    for (auto *item : items) {
        if (item->type() == GraphicElement::Type) {
            auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
            ds << GraphicElement::Type;
            ds << static_cast<quint64>(elm->elementType());
            elm->save(ds);
        }
    }
    for (auto *item : items) {
        if (item->type() == QNEConnection::Type) {
            qCDebug(zero) << "Writing Connection.";
            auto *conn = qgraphicsitem_cast<QNEConnection *>(item);
            ds << QNEConnection::Type;
            conn->save(ds);
        }
    }
}

QList<QGraphicsItem *> SerializationFunctions::deserialize(QDataStream &ds, double version, QMap<quint64, QNEPort *> portMap)
{
    QList<QGraphicsItem *> itemList;
    while (!ds.atEnd()) {
        int32_t type;
        ds >> type;
        qCDebug(three) << "Type:" << type;
        if (type == GraphicElement::Type) {
            quint64 elmType;
            ds >> elmType;
            qCDebug(three) << "Building" << ElementFactory::typeToText(static_cast<ElementType>(elmType)) << "element.";
            GraphicElement *elm = ElementFactory::buildElement(static_cast<ElementType>(elmType));
            if (elm) {
                itemList.append(elm);
                elm->load(ds, portMap, version);
                if (elm->elementType() == ElementType::IC) {
                    qCDebug(three) << "Loading IC.";
                    IC *ic = qgraphicsitem_cast<IC *>(elm);
                    ICManager::instance()->loadIC(ic, ic->getFile());
                }
                elm->setSelected(true);
            } else {
                throw(std::runtime_error(ERRORMSG("Could not build element."))); // TODO: must remove this message from here and not throw an exception.
            }
        } else if (type == QNEConnection::Type) {
            qCDebug(three) << "Reading Connection.";
            QNEConnection *conn = ElementFactory::buildConnection();
            qCDebug(three) << "Connection built.";
            conn->setSelected(true);
            qCDebug(three) << "Selected true.";
            if (!conn->load(ds, portMap)) {
                qCDebug(three) << "Deleting connection.";
                delete conn;
            } else {
                qCDebug(three) << "Appending connection.";
                itemList.append(conn);
            }
        } else {
            qCDebug(zero) << type;
            throw(std::runtime_error(ERRORMSG("Invalid type. Data is possibly corrupted.")));
        }
    }
    qCDebug(zero) << "Finished deserializing.";
    return itemList;
}

double SerializationFunctions::loadVersion(QDataStream &ds)
{
    qCDebug(zero) << "Loading version.";
    QString str;
    ds >> str;
    if (!str.startsWith(QApplication::applicationName(), Qt::CaseInsensitive)) {
        throw(std::runtime_error(ERRORMSG("Invalid file format.")));
    }
    qCDebug(zero) << "String:" << str;
    bool ok;
    double version = GlobalProperties::toDouble(str.split(" ").at(1), &ok);
    qCDebug(zero) << "Version:" << version;
    if (!ok) {
        throw(std::runtime_error(ERRORMSG("Invalid version number.")));
    }
    return version;
}

QString SerializationFunctions::loadDolphinFilename(QDataStream &ds, double version)
{
    QString str = "";
    if (version >= 3.0) {
        ds >> str;
        if ((version < 3.3) && (str == "none")) {
            str = "";
        }
    }
    return str;
}

QRectF SerializationFunctions::loadRect(QDataStream &ds, double version)
{
    QRectF rect;
    if (version >= 1.4) {
        ds >> rect;
    }
    return rect;
}

QList<QGraphicsItem *> SerializationFunctions::load(QDataStream &ds)
{
    qCDebug(zero) << "Started loading file.";
    QString str;
    ds >> str;
    if (!str.startsWith(QApplication::applicationName(), Qt::CaseInsensitive)) {
        throw(std::runtime_error(ERRORMSG("Invalid file format.")));
    }
    bool ok;
    double version = GlobalProperties::toDouble(str.split(" ").at(1), &ok);
    if (!ok) {
        throw(std::runtime_error(ERRORMSG("Invalid version number.")));
    }
    loadDolphinFilename(ds, version);
    loadRect(ds, version);
    qCDebug(zero) << "Header Ok. Version:" << version;
    QList<QGraphicsItem *> items = deserialize(ds, version);
    qCDebug(zero) << "Finished reading items.";
    return items;
}
