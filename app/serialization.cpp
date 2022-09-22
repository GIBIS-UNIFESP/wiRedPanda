// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "serialization.h"

#include "common.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "qneconnection.h"

#include <QApplication>

void Serialization::saveHeader(QDataStream &stream, const QString &dolphinFileName, const QRectF &rect)
{
    stream << QApplication::applicationName() + " " + QString::number(GlobalProperties::version);
    stream << dolphinFileName;
    stream << rect;
}

void Serialization::serialize(const QList<QGraphicsItem *> &items, QDataStream &stream)
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

QList<QGraphicsItem *> Serialization::deserialize(QDataStream &stream, QMap<quint64, QNEPort *> portMap, const double version)
{
    QList<QGraphicsItem *> itemList;

    while (!stream.atEnd()) {
        int type; stream >> type;
        qCDebug(three) << tr("Type: ") << type;

        switch (type) {
        case GraphicElement::Type: {
            ElementType elmType; stream >> elmType;

            auto *elm = ElementFactory::buildElement(elmType);
            itemList.append(elm);
            elm->load(stream, portMap, version);
            break;
        }

        case QNEConnection::Type: {
            qCDebug(three) << tr("Building connection.");
            auto *conn = new QNEConnection();

            qCDebug(three) << tr("Loading connection.");
            conn->load(stream, portMap);

            qCDebug(three) << tr("Appending connection.");
            itemList.append(conn);
            break;
        }

        default:
            throw Pandaception(tr("Invalid type. Data is possibly corrupted."));
        }
    }

    qCDebug(zero) << tr("Finished deserializing.");
    return itemList;
}

double Serialization::loadVersion(QDataStream &stream)
{
    qCDebug(zero) << tr("Loading version.");

    QString str; stream >> str;

    if (!str.startsWith(QApplication::applicationName(), Qt::CaseInsensitive)) {
        throw Pandaception(tr("Invalid file format."));
    }

    qCDebug(zero) << tr("String: ") << str;

    bool ok;
    double version = str.remove(QApplication::applicationName(), Qt::CaseInsensitive).toDouble(&ok);
    qCDebug(zero) << tr("Version: ") << version;

    if (!ok) {
        throw Pandaception(tr("Invalid version number."));
    }

    return version;
}

QString Serialization::loadDolphinFileName(QDataStream &stream, const double version)
{
    QString str;

    if (version >= 3.0) {
        stream >> str;

        if ((version < 3.3) && (str == "none")) {
            str.clear();
        }
    }

    return str;
}

QRectF Serialization::loadRect(QDataStream &stream, const double version)
{
    QRectF rect;

    if (version >= 1.4) {
        stream >> rect;
    }

    return rect;
}
