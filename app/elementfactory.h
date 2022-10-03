// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "enums.h"

#include <QGraphicsItem>
#include <memory>

class GraphicElement;
class ItemWithId;
class LogicElement;
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
    static GraphicElement *buildElement(const ElementType type);
    static ItemWithId *itemById(const int id);
    static std::shared_ptr<LogicElement> buildLogicElement(GraphicElement *elm);
    static QPixmap pixmap(const ElementType type);
    static QString property(const ElementType type, const QString &property);
    static QString translatedName(const ElementType type);
    static QString typeToText(const ElementType type);
    static QString typeToTitleText(const ElementType type);
    static bool contains(const int id);
    static void addItem(ItemWithId *item);
    static void removeItem(ItemWithId *item);
    static void updateItemId(ItemWithId *item, const int newId);

private:
    int nextId();

    QMap<int, ItemWithId *> m_map;
    int m_lastId = 0;
};
