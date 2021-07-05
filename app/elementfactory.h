/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ELEMENTFACTORY_H
#define ELEMENTFACTORY_H

#include <QGraphicsItem>
#include <QObject>

#include "elementtype.h"

class Editor;
class GraphicElement;
class ItemWithId;
class QNEConnection;

class ElementFactory : public QObject
{
    Q_OBJECT

public:
    static ElementFactory *instance;

    static ElementType textToType(QString text);
    static QString typeToText(ElementType type);
    static QString translatedName(ElementType type);
    static QPixmap getPixmap(ElementType type);
    static GraphicElement *buildElement(ElementType type, QGraphicsItem *parent = nullptr);
    static QNEConnection *buildConnection(QGraphicsItem *parent = nullptr);
    static ItemWithId *getItemById(size_t id);
    static bool contains(size_t id);
    static void updateItemId(ItemWithId *item, size_t newId);
    static void removeItem(ItemWithId *item);
    static void addItem(ItemWithId *item);

    size_t getLastId() const;
    size_t next_id();
    void clear();

private:
    QMap<size_t, ItemWithId *> m_map;
    size_t m_lastId;
    ElementFactory();
};

#endif /* ELEMENTFACTORY_H */
