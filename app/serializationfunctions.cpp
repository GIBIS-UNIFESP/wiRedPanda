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
        if (auto *element = qgraphicsitem_cast<GraphicElement *>(item)) {
            ds << element;
        }
        if (auto *connection = qgraphicsitem_cast<QNEConnection *>(item)) {
            ds << connection;
        }
    }
}

bool SerializationFunctions::deserialize(QDataStream &ds, QList<QGraphicsItem *> &itemList, double version, QMap<quint64, QNEPort *> portMap)
{
    while (!ds.atEnd()) {
        int32_t type;
        ds >> type;
        qCDebug(three) << "Type:" << type;

        if (type != GraphicElement::Type && type != QNEConnection::Type) {
            throw(std::runtime_error(ERRORMSG(QObject::tr("Invalid type. Data is possibly corrupted.").toStdString())));
        }

        if (type == GraphicElement::Type) {
            ElementType elmType;
            ds >> elmType;
            qCDebug(three) << "Building " << ElementFactory::typeToText(static_cast<ElementType>(elmType)) << " element.";
            auto* elm = ElementFactory::buildElement(static_cast<ElementType>(elmType));
            if (elm) {
                itemList.append(elm);
                elm->load(ds, portMap, version);
                if (elm->elementType() == ElementType::IC) {
                    qCDebug(three) << "Loading IC.";
                    IC *ic = qgraphicsitem_cast<IC *>(elm);
                    if (!ICManager::instance()->loadIC(ic, ic->getFile())) {
                        return false;
                    }
                }
                elm->setSelected(true);
            } else {
                qCDebug(zero) << "Could not build element.";
                return false;
            }

            elm->setSelected(true);
        }

        if (type == QNEConnection::Type) {
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
            qCDebug(zero) << "Invalid type. Data is possibly corrupted: " << type;
            return false;
        }
    }
    qCDebug(zero) << "Finished deserializing.";
    return true;
}

double SerializationFunctions::loadVersion(QDataStream &ds)
{
    qCDebug(zero) << "Loading version.";
    QString str;
    ds >> str;
    if (!str.startsWith(QApplication::applicationName(), Qt::CaseInsensitive)) {
        throw(std::runtime_error(ERRORMSG(QObject::tr("Invalid file format.")).toStdString()));
    }
    qCDebug(zero) << "String:" << str;
    bool ok;
    double version = GlobalProperties::toDouble(str.split(" ").at(1), &ok);
    qCDebug(zero) << "Version:" << version;
    if (!ok) {
        throw(std::runtime_error(ERRORMSG(QObject::tr("Invalid version number.")).toStdString()));
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

bool SerializationFunctions::load(QDataStream &ds, QList<QGraphicsItem *> items)
{
    qCDebug(zero) << "Started loading file.";
    QString str;
    ds >> str;
    if (!str.startsWith(QApplication::applicationName(), Qt::CaseInsensitive)) {
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
    qCDebug(zero) << "Header Ok. Version:" << version;
    return (deserialize(ds, items, version));
}
