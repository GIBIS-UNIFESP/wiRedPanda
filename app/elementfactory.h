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
    static ElementFactory *instance;

    static ElementType textToType(const QString &text);
    static QString typeToText(ElementType type);
    static QString typeToTitleText(ElementType type);
    static QString translatedName(ElementType type);
    static QPixmap getPixmap(ElementType type);
    static GraphicElement *buildElement(ElementType type);
    static QNEConnection *buildConnection(QGraphicsItem *parent = nullptr);
    static ItemWithId *getItemById(int id);
    static bool contains(int id);
    static void updateItemId(ItemWithId *item, int newId);
    static void removeItem(ItemWithId *item);
    static void addItem(ItemWithId *item);

    int getLastId() const;
    int next_id();
    void clear();

private:
    QMap<int, ItemWithId *> m_map;
    int m_lastId;
    ElementFactory();
};

