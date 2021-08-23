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

#include "common.h"
#include "editor.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "ic.h"
#include "icmanager.h"
#include "qneconnection.h"
#include "qneport.h"

void SerializationFunctions::saveHeader(QDataStream &ds, const QString &dolphinFilename, const QRectF &rect)
{
  ds << QApplication::applicationName() + " " + QString::number(GlobalProperties::version);
  ds << dolphinFilename;
  ds << rect;
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
            COMMENT("Writing Connection.", 0);
            auto *conn = qgraphicsitem_cast<QNEConnection *>(item);
            ds << QNEConnection::Type;
            conn->save(ds);
        }
    }
}

bool SerializationFunctions::deserialize(QDataStream &ds, QList<QGraphicsItem *> &itemList, double version, QMap<quint64, QNEPort *> portMap)
{
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
                    if (!ICManager::instance()->loadIC(ic, ic->getFile())) {
                        return false;
                    }
                }
                elm->setSelected(true);
            } else {
                qDebug() << "Could not build element.";
                return false;
            }
        } else if (type == QNEConnection::Type) {
            COMMENT("Reading Connection.", 3);
            QNEConnection *conn = ElementFactory::buildConnection();
            COMMENT("Connection built.", 3);
            conn->setSelected(true);
            COMMENT("Selected true.", 3);
            if (!conn->load(ds, portMap)) {
                COMMENT("Deleting connection.", 3);
                delete conn;
            } else {
                COMMENT("Appending connection.", 3);
                itemList.append(conn);
            }
        } else {
            qDebug() << "Invalid type. Data is possibly corrupted: " << type;
            return false;
        }
    }
    COMMENT("Finished deserializing.", 0);
    return true;
}

double SerializationFunctions::loadVersion(QDataStream &ds)
{
    COMMENT("Loading version.", 0);
    QString str;
    ds >> str;
    if (!str.startsWith(QApplication::applicationName())) {
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
        if ((version < 3.3)&&(str=="none")) {
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

bool SerializationFunctions::load(QDataStream &ds, QList<QGraphicsItem *> items)
{
    COMMENT("Started loading file.", 0);
    QString str;
    ds >> str;
    if (!str.startsWith(QApplication::applicationName())) {
        qDebug() << "Invalid file format.";
        return false;
    }
    bool ok;
    double version = GlobalProperties::toDouble(str.split(" ").at(1), &ok);
    if (!ok) {
        qDebug() << "Invalid version number.";
        return false;
    }
    loadDolphinFilename(ds, version);
    loadRect( ds, version );
    COMMENT("Header Ok. Version: " << version, 0);
    return (deserialize(ds, items, version));
}
