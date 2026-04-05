// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Element metadata struct and global registry for compile-time element registration.
 */

#pragma once

#include <functional>

#include <QMap>
#include <QString>
#include <QStringList>

#include "App/Core/Enums.h"

/**
 * \struct ElementMetadata
 * \brief Compile-time-registered properties for one element type.
 *
 * \details Each element .cpp registers one instance via an ElementInfo<T>
 * specialization.  All fields are populated at static-initialization time
 * (before QApplication) except \c pixmapPath, which is a callable invoked
 * lazily at construction time.
 */
struct ElementMetadata {
    ElementType type = ElementType::Unknown;   ///< Unique element type identifier.
    ElementGroup group = ElementGroup::Unknown; ///< UI palette group this element belongs to.

    // --- Display properties ---

    std::function<QString()> pixmapPath; ///< Callable returning the default pixmap resource path (lazily evaluated).
    const char *titleText = nullptr;     ///< Untranslated title shown in the element palette (QT_TRANSLATE_NOOP).
    const char *translatedName = nullptr; ///< Untranslated element name used for tooltips (QT_TRANSLATE_NOOP).
    const char *trContext = nullptr;     ///< Translation context string for QCoreApplication::translate().

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
    bool hasColors = false;     ///< True if the element supports color selection.
    bool hasAudio = false;      ///< True if the element supports audio output.
    bool hasAudioBox = false;   ///< True if the element shows an audio selection box.
    bool hasTrigger = false;    ///< True if the element supports a keyboard trigger shortcut.
    bool hasFrequency = false;  ///< True if the element exposes a configurable clock frequency.
    bool hasDelay = false;      ///< True if the element exposes a configurable clock phase delay.
    bool hasLabel = false;      ///< True if the element supports a user-editable label.
    bool hasTruthTable = false; ///< True if the element has an editable truth table.
    bool hasVolume = false;     ///< True if the element supports volume control.
    bool rotatable = true;      ///< True if the user can rotate this element.
};

/**
 * \class ElementMetadataRegistry
 * \brief Global registry of ElementMetadata, one entry per ElementType.
 *
 * \details Populated during static initialization by ElementInfo<T>::registered
 * lambdas.  Look up metadata at runtime via metadata().
 */
class ElementMetadataRegistry
{
public:
    /// Returns the metadata for \a type. Asserts if \a type is not registered.
    static const ElementMetadata &metadata(ElementType type);
    /// Registers \a meta in the global map (called once per element type at startup).
    static void registerMetadata(const ElementMetadata &meta);
    /// Returns \c true if metadata has been registered for \a type.
    static bool hasMetadata(ElementType type);

private:
    /// Returns a reference to the singleton registry map.
    static QMap<ElementType, ElementMetadata> &registry();
};

