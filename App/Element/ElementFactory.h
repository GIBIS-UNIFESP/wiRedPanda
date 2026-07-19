// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Singleton factory for all circuit element types.
 */

#pragma once

#include <functional>

#include <QHash>

#include "App/Core/Enums.h"

class GraphicElement;

/**
 * \class ElementFactory
 * \brief Factory for circuit elements.
 *
 * The ElementFactory class is a singleton that handles the creation of
 * circuit elements and provides access to element type properties.
 * ID management is handled by Scene, not by this class.
 */
class ElementFactory : public QObject
{
    Q_OBJECT

public:
    /// Returns the singleton ElementFactory instance.
    static ElementFactory &instance()
    {
        // gcov doesn't attribute a hit to this construction line: the very first call to
        // instance() happens during static initialization (each element .cpp's file-scope
        // "static inline const bool registered = [](){ ...; ElementFactory::registerCreator(...); }();"
        // lambda calls it before main() runs), and coverage instrumentation for code executed
        // that early isn't recorded here (same class of limitation as the Meyer's-singleton
        // pattern documented for ThemeManager::instance() in App/Core).
        static ElementFactory instance; // LCOV_EXCL_LINE
        return instance;
    }

    // --- Element creation ---

    /// Constructs and returns a new graphic element of the given \a type.
    static GraphicElement *buildElement(const ElementType type);

    /// Returns \c true if a creator function is registered for \a type.
    static bool hasCreator(ElementType type);

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

    /// Returns a short, translated functional description of \a type for palette tooltips
    /// (empty for types without one, e.g. IC).
    static QString description(const ElementType type);

    /// Returns the pixmap icon for the given element \a type.
    static QPixmap pixmap(const ElementType type);

private:
    QHash<ElementType, std::function<GraphicElement *()>> m_creatorMap;
};
