// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Singleton factory and registry for all circuit element types.
 */

#pragma once

#include <functional>
#include <memory>

#include <QGraphicsItem>
#include <QMap>

#include "App/Core/Enums.h"

class GraphicElement;
class ItemWithId;
class LogicElement;
class QNEConnection;

/**
 * \class ElementFactory
 * \brief Factory and registry for circuit elements.
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
    /// Returns the singleton ElementFactory instance.
    static ElementFactory &instance()
    {
        static ElementFactory instance;
        return instance;
    }

    // --- Element creation ---

    /// Constructs and returns a new graphic element of the given \a type.
    static GraphicElement *buildElement(const ElementType type);

    // --- Item registry ---

    /// Returns the item with the given \a id, or \c nullptr if not found.
    static ItemWithId *itemById(const int id);

    /// Returns \c true if an item with \a id is registered.
    static bool contains(const int id);

    /// Returns \c true if a creator function is registered for \a type.
    static bool hasCreator(ElementType type);

    /// Registers \a item in the scene's ID map.
    static void addItem(ItemWithId *item);

    /// Removes \a item from the ID→item map.
    static void removeItem(ItemWithId *item);

    /// Sets the counter used to issue new IDs.
    static void setLastId(const int newLastId);

    /// Changes the registry entry for \a item to \a newId.
    static void updateItemId(ItemWithId *item, const int newId);

    /// Registers a creator lambda for \a type, used by buildElement().
    static void registerCreator(ElementType type, std::function<GraphicElement *()> creator);

    // --- Type name conversion ---

    /// Converts an element type name string to its ElementType enum value.
    static ElementType textToType(const QString &text);

    /// Converts \a type to its internal name string.
    static QString typeToText(const ElementType type);

    /// Returns the display title string for \a type.
    static QString typeToTitleText(const ElementType type);

    /// Returns the translated human-readable name for \a type.
    static QString translatedName(const ElementType type);

    /// Returns the pixmap icon for the given element \a type.
    static QPixmap pixmap(const ElementType type);

private:
    int nextId();

    QMap<int, ItemWithId *> m_map;
    QMap<ElementType, std::function<GraphicElement *()>> m_creatorMap;
    int m_lastId = 0;
};

