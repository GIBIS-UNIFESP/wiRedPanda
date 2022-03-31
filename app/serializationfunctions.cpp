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
<<<<<<< HEAD
    double version;
    QString dolphinFilename;
    QRectF rect;
    QList<QGraphicsItem *> itemList;
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
        COMMENT("Started reading IC file " << fileName.toStdString(), 0);
        QDataStream ds(&file);
        version = loadVersion(ds);
        dolphinFilename = loadDolphinFilename(ds, version);
        rect = loadRect(ds, version);
        COMMENT("Version: " << version, 0);
        COMMENT("Element deserialization.", 0);
        itemList = loadMoveData(dirName, ds, version);
        COMMENT("Finished loading data", 0);
    }
    QSaveFile fl(fileName);
    COMMENT("Before saving data", 0);
    if (fl.open(QFile::WriteOnly)) {
        COMMENT("Start updating IC " << fileName.toStdString(), 0);
        QDataStream ds(&fl);
        COMMENT("Saving header information.", 0);
        saveHeader(ds, dolphinFilename, rect);
        COMMENT("Element serialization.", 0);
        serialize(itemList, ds);
    }
    if (!fl.commit()) {
        std::cerr << "Could not save file: " + fl.errorString().toStdString() + "." << std::endl;
        return false;
    }

    COMMENT("Finished updating IC " << fileName.toStdString(), 0);
    return true;
=======
    ds << QApplication::applicationName() + " " + QString::number(GlobalProperties::version);
    ds << dolphinFilename;
    ds << rect;
>>>>>>> master
}

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
            COMMENT("Writing Connection.", 0);
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
        COMMENT("Type: " << type, 3);
        if (type == GraphicElement::Type) {
            quint64 elmType;
            ds >> elmType;
            COMMENT("Building " << ElementFactory::typeToText(static_cast<ElementType>(elmType)).toStdString() << " element.", 3);
            GraphicElement *elm = ElementFactory::buildElement(static_cast<ElementType>(elmType));
            if (elm) {
                itemList.append(elm);
                elm->load(ds, portMap, version);
                if (elm->elementType() == ElementType::IC) {
                    COMMENT("Loading IC.", 3);
                    IC *ic = qgraphicsitem_cast<IC *>(elm);
                    ICManager::instance()->loadIC(ic, ic->getFile());
                }
                elm->setSelected(true);
            } else {
                throw(std::runtime_error(ERRORMSG("Could not build element."))); // TODO: must remove this message from here and not throw an exception.
            }
        } else if (type == QNEConnection::Type) {
            COMMENT("Reading Connection.", 3);
            QNEConnection *conn = ElementFactory::buildConnection();
<<<<<<< HEAD
            COMMENT("Connection built.", 0);
            conn->setSelected(true);
            COMMENT("Selected true.", 0);
=======
            COMMENT("Connection built.", 3);
            conn->setSelected(true);
            COMMENT("Selected true.", 3);
>>>>>>> master
            if (!conn->load(ds, portMap)) {
                COMMENT("Deleting connection.", 3);
                delete conn;
            } else {
<<<<<<< HEAD
                COMMENT("Appending connection.", 0);
=======
                COMMENT("Appending connection.", 3);
>>>>>>> master
                itemList.append(conn);
            }
        } else {
            qDebug() << type;
            throw(std::runtime_error(ERRORMSG("Invalid type. Data is possibly corrupted.")));
        }
    }
    COMMENT("Finished deserializing.", 0);
    return itemList;
}

double SerializationFunctions::loadVersion(QDataStream &ds)
{
    COMMENT("Loading version.", 0);
    QString str;
    ds >> str;
    if (!str.startsWith(QApplication::applicationName(), Qt::CaseInsensitive)) {
        throw(std::runtime_error(ERRORMSG("Invalid file format.")));
    }
    COMMENT("String: " << str.toStdString(), 0);
    bool ok;
    double version = GlobalProperties::toDouble(str.split(" ").at(1), &ok);
    COMMENT("Version: " << version, 0);
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
    COMMENT("Started loading file.", 0);
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
    COMMENT("Header Ok. Version: " << version, 0);
    QList<QGraphicsItem *> items = deserialize(ds, version);
    COMMENT("Finished reading items.", 0);
    return items;
}
