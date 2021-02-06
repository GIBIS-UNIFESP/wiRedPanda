// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "serializationfunctions.h"

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

#include "editor.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "ic.h"
#include "icmanager.h"
#include "qneconnection.h"
#include "qneport.h"

bool SerializationFunctions::update(const QString &fileName, const QString &dirName)
{
    double version;
    QRectF rect;
    QList<QGraphicsItem *> itemList;
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
        qDebug() << "Started reading IC file " << QString::fromStdString(fileName.toStdString());
        QDataStream ds(&file);
        version = loadVersion(ds);
        loadDolphinFilename(ds, version);
        rect = loadRect(ds, version);
        qDebug() << "Version: " << version;
        qDebug() << "Element deserialization.";
        itemList = loadMoveData(dirName, ds, version);
        qDebug() << "Finished loading data";
    }
    QSaveFile fl(fileName);
    qDebug() << "Before saving data";
    if (fl.open(QFile::WriteOnly)) {
        qDebug() << "Start updating IC " << QString::fromStdString(fileName.toStdString());
        QDataStream ds(&fl);
        ds << QApplication::applicationName() + " " + QString::number(GlobalProperties::version);
        ds << rect;
        qDebug() << "Element serialization.";
        serialize(itemList, ds);
    }
    if (!fl.commit()) {
        std::cerr << "Could not save file: " << fl.errorString().toStdString() << "." << std::endl;
        return false;
    }

    qDebug() << "Finished updating IC " << QString::fromStdString(fileName.toStdString());
    return true;
}

void SerializationFunctions::serialize(const QList<QGraphicsItem *> &items, QDataStream &ds)
{
    for (QGraphicsItem *item : items) {
        if (item->type() == GraphicElement::Type) {
            auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
            ds << GraphicElement::Type;
            ds << static_cast<quint64>(elm->elementType());
            elm->save(ds);
        }
    }
    for (QGraphicsItem *item : items) {
        if (item->type() == QNEConnection::Type) {
            qDebug() << "Writing Connection.";
            auto *conn = qgraphicsitem_cast<QNEConnection *>(item);
            ds << QNEConnection::Type;
            conn->save(ds);
        }
    }
}

QList<QGraphicsItem *> SerializationFunctions::deserialize(QDataStream &ds, double version, const QString &parentFile, QMap<quint64, QNEPort *> portMap)
{
    QList<QGraphicsItem *> itemList;
    while (!ds.atEnd()) {
        int32_t type;
        ds >> type;
        qDebug() << "Type: " << type;
        if (type == GraphicElement::Type) {
            quint64 elmType;
            ds >> elmType;
            qDebug() << "Building " << QString::fromStdString(ElementFactory::typeToText(static_cast<ElementType>(elmType)).toStdString()) << " element.";
            GraphicElement *elm = ElementFactory::buildElement(static_cast<ElementType>(elmType));
            if (elm) {
                itemList.append(elm);
                elm->load(ds, portMap, version);
                if (elm->elementType() == ElementType::IC) {
                    qDebug() << "Loading IC.";
                    IC *ic = qgraphicsitem_cast<IC *>(elm);
                    ICManager::instance()->loadIC(ic, ic->getFile(), parentFile);
                }
                elm->setSelected(true);
            } else {
                throw(std::runtime_error("Could not build element."));
            }
        } else if (type == QNEConnection::Type) {
            qDebug() << "Reading Connection.";
            QNEConnection *conn = ElementFactory::buildConnection();
            conn->setSelected(true);
            if (!conn->load(ds, portMap)) {
                qDebug() << "Deleting connection.";
                delete conn;
            } else {
                itemList.append(conn);
            }
        } else {
            qDebug() << type;
            throw(std::runtime_error("Invalid type. Data is possibly corrupted."));
        }
    }
    return itemList;
}

double SerializationFunctions::loadVersion(QDataStream &ds)
{
    qDebug() << "Loading version.";
    QString str;
    ds >> str;
    if (!str.startsWith(QApplication::applicationName())) {
        throw(std::runtime_error("Invalid file format."));
    }
    qDebug() << "String: " << QString::fromStdString(str.toStdString());
    bool ok;
    double version = GlobalProperties::toDouble(str.split(" ").at(1), &ok);
    qDebug() << "Version: " << version;
    if (!ok) {
        throw(std::runtime_error("Invalid version number."));
    }
    return version;
}

QString SerializationFunctions::loadDolphinFilename(QDataStream &ds, double version)
{
    QString str = "none";
    if (version >= 3.0)
        ds >> str;
    return str;
}

QRectF SerializationFunctions::loadRect(QDataStream &ds, double version)
{
    QRectF rect;
    if (version >= 1.4)
        ds >> rect;
    return rect;
}

QList<QGraphicsItem *> SerializationFunctions::loadMoveData(const QString &dirName, QDataStream &ds, double version)
{
    QMap<quint64, QNEPort *> portMap;
    QList<QGraphicsItem *> itemList;
    while (!ds.atEnd()) {
        int type;
        ds >> type;
        qDebug() << "Type: " << type;
        if (type == GraphicElement::Type) {
            quint64 elmType;
            ds >> elmType;
            qDebug() << "Building " << QString::fromStdString(ElementFactory::typeToText(static_cast<ElementType>(elmType)).toStdString()) << " element.";
            GraphicElement *elm = ElementFactory::buildElement(static_cast<ElementType>(elmType));
            if (elm) {
                itemList.append(elm);
                elm->load(ds, portMap, version);
                if (elm->elementType() == ElementType::IC) {
                    IC *ic = qgraphicsitem_cast<IC *>(elm);
                    QString oldName = ic->getFile();
                    QString newName = dirName + "/boxes/" + QFileInfo(oldName).fileName();
                    ic->setFile(newName);
                }
                elm->updateSkinsPath(dirName + "/skins/");
            } else {
                throw(std::runtime_error("Could not build element."));
            }
        } else if (type == QNEConnection::Type) {
            qDebug() << "Reading Connection.";
            QNEConnection *conn = ElementFactory::buildConnection();
            if (!conn->load(ds, portMap)) {
                qDebug() << "Deleting connection.";
                delete conn;
            } else {
                itemList.append(conn);
            }
        } else {
            qDebug() << type;
            throw(std::runtime_error("Invalid type. Data is possibly corrupted."));
        }
    }
    qDebug() << "Finished loading data.";
    return itemList;
}

QList<QGraphicsItem *> SerializationFunctions::load(QDataStream &ds, const QString &parentFile)
{
    qDebug() << "Started loading file.";
    QString str;
    ds >> str;
    if (!str.startsWith(QApplication::applicationName())) {
        throw(std::runtime_error("Invalid file format."));
    }
    bool ok;
    double version = GlobalProperties::toDouble(str.split(" ").at(1), &ok);
    if (!ok) {
        throw(std::runtime_error("Invalid version number."));
    }
    loadDolphinFilename(ds, version);
    loadRect( ds, version );
    qDebug() << "Header Ok. Version: " << version;
    QList<QGraphicsItem *> items = deserialize(ds, version, parentFile);
    qDebug() << "Finished reading items.";
    return items;
}
