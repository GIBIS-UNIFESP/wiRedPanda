// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief An element type's descriptor, and the registry of every type's, filled at start-up.
 */

#pragma once

#include <functional>

#include <QMap>
#include <QString>
#include <QStringList>

#include "App/Core/Enums.h"
#include "App/Element/Parameters.h"

/**
 * \struct ElementTypeDescriptor
 * \brief What one element type is: its group, names, ports, pictures and capabilities.
 *
 * \details Each element .cpp adds its type's descriptor through its ElementInfo<T>
 * specialization's describe(); of(), add() and has() are the registry of every
 * type's. All fields are populated at static-initialization time (before
 * QApplication) except \c pixmapPath, which is a callable invoked lazily at
 * construction time.
 */
struct ElementTypeDescriptor {
    /// Returns the descriptor of \a type. Asserts if \a type has none; a release build warns and returns an empty one.
    static const ElementTypeDescriptor &of(ElementType type);
    /// Adds \a descriptor to the registry (called once per element type at startup).
    static void add(const ElementTypeDescriptor &descriptor);
    /// Returns \c true if \a type has a descriptor.
    static bool has(ElementType type);

    ElementType type = ElementType::Unknown;   ///< Unique element type identifier.
    ElementGroup group = ElementGroup::Unknown; ///< UI palette group this element belongs to.

    // --- Display properties ---

    std::function<QString()> pixmapPath; ///< Callable returning the default pixmap resource path (lazily evaluated).
    const char *titleText = nullptr;     ///< Untranslated title shown in the element palette (QT_TRANSLATE_NOOP).
    const char *translatedName = nullptr; ///< Untranslated element name used for tooltips (QT_TRANSLATE_NOOP).
    const char *trContext = nullptr;     ///< Translation context string for QCoreApplication::translate().

    // --- Parameters ---

    QList<ParamSpec> parameters; ///< The type's parameters (its schema), each element holding a value of each.

    // --- Port configuration ---

    quint64 minInputSize = 0;  ///< Minimum number of input ports.
    quint64 maxInputSize = 0;  ///< Maximum number of input ports.
    quint64 minOutputSize = 0; ///< Minimum number of output ports.
    quint64 maxOutputSize = 0; ///< Maximum number of output ports.

    // --- Appearance configuration ---

    QStringList defaultAppearances;     ///< Built-in appearance resource paths.
    QStringList alternativeAppearances; ///< User-selectable alternative appearance paths.

    // --- Feature flags ---

    bool canChangeAppearance = false; ///< True if the user can choose a custom appearance.
    bool hasTrigger = false;    ///< True if the element supports a keyboard trigger shortcut.
    bool hasLabel = false;      ///< True if the element supports a user-editable label.
    bool rotatesGraphic = true; ///< True if rotating/flipping re-orients the graphic itself; false keeps the icon fixed and only repositions ports.

private:
    /// Returns a reference to the singleton registry map.
    static QMap<ElementType, ElementTypeDescriptor> &registry();
};
