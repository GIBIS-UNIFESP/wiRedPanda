/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "elementtype.h"

#include <QGraphicsItem>

class GraphicElement;
class ItemWithId;
class QNEConnection;

class ElementFactory : public QObject
{
    Q_OBJECT

public:
    static ElementFactory &instance()
    {
        static ElementFactory instance;
        return instance;
    }

    static ElementType textToType(const QString &text);
    static QString typeToText(const ElementType type);
    static QString typeToTitleText(const ElementType type);
    static QString translatedName(const ElementType type);
    static QPixmap pixmap(const ElementType type);
    static GraphicElement *buildElement(const ElementType type);
    static QNEConnection *buildConnection(QGraphicsItem *parent = nullptr);
    static ItemWithId *itemById(const int id);
    static bool contains(const int id);
    static void updateItemId(ItemWithId *item, const int newId);
    static void removeItem(ItemWithId *item);
    static void addItem(ItemWithId *item);

private:
    int nextId();

    QMap<int, ItemWithId *> m_map;
    int m_lastId = 0;
};
