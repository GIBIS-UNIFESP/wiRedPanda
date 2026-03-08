// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Singleton factory and registry for all circuit element types.
 */

#pragma once

#include <memory>

#include <QGraphicsItem>

#include "App/Core/Enums.h"

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

    /// Constructs and returns the logic element that drives \a elm.
    static std::shared_ptr<LogicElement> buildLogicElement(GraphicElement *elm);

    // --- Item registry ---

    /// Registers \a item in the scene's ID map.
    static void addItem(ItemWithId *item);

    /**
     * \brief Removes \a item from the ID→item map.
     * \param item Item to unregister.
     */
    static void removeItem(ItemWithId *item);

    /// Returns the item with the given \a id, or \c nullptr if not found.
    static ItemWithId *itemById(const int id);

    /// Returns \c true if an item with \a id is registered.
    static bool contains(const int id);

    /// Changes the registry entry for \a item to \a newId.
    static void updateItemId(ItemWithId *item, const int newId);

    /// Sets the counter used to issue new IDs.
    static void setLastId(const int newLastId);

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

    /// Returns the static (compile-time) property descriptor for \a type.
    static const ElementStaticProperties& getStaticProperties(ElementType type);

    /// Clears any internally cached pixmaps or property data.
    static void clearCache();

private:
    // --- Members ---

    QMap<int, ItemWithId *> m_map;
    int m_lastId = 0;
    QMap<ElementType, ElementStaticProperties> m_propertyCache;

    // --- Internal methods ---

    int nextId();
    const ElementStaticProperties& ensurePropertiesCached(ElementType type);
};

