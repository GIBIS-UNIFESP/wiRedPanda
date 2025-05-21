// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "enums.h"

#include <QGraphicsItem>
#include <memory>

class GraphicElement;
class ItemWithId;
class LogicElement;
class QNEConnection;

/*!
 * @struct ElementStaticProperties
 * @brief Contains static display properties for an element type
 *
 * This structure holds the graphical and textual properties that
 * are common to all instances of a particular element type.
 */
struct ElementStaticProperties {
    QString pixmapPath;
    QString titleText;
    QString translatedName;
};

/*!
 * @class ElementFactory
 * @brief Factory and registry for circuit elements
 *
 * The ElementFactory class is a singleton that handles the creation and
 * tracking of all circuit elements. It provides methods to build graphic
 * elements, maintains a registry of all elements by ID, and
 * provides access to element properties.
 */
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
    static QPixmap pixmap(const ElementType type);
    static QString translatedName(const ElementType type);
    static QString typeToText(const ElementType type);
    static QString typeToTitleText(const ElementType type);
    static bool contains(const int id);
    static const ElementStaticProperties& getStaticProperties(ElementType type);
    static std::shared_ptr<LogicElement> buildLogicElement(GraphicElement *elm);
    static void addItem(ItemWithId *item);
    static void clearCache();
    static void removeItem(ItemWithId *item);
    static void updateItemId(ItemWithId *item, const int newId);

private:
    int nextId();

    QMap<int, ItemWithId *> m_map;
    int m_lastId = 0;

    QMap<ElementType, ElementStaticProperties> m_propertyCache;
    const ElementStaticProperties& ensurePropertiesCached(ElementType type);
};
