// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Self-registering element trait template and compile-time constraint validation.
 */

#pragma once

#include "App/Core/Enums.h"
#include "App/Element/ElementTypeDescriptor.h"

/**
 * \struct ElementConstraints
 * \brief Compile-time-validatable subset of ElementTypeDescriptor.
 *
 * \details These constexpr-friendly constraints enable static validation of
 * an element type's descriptor at compile time.  They are converted to ElementTypeDescriptor
 * via descriptorFromConstraints().
 */
struct ElementConstraints {
    ElementType type = ElementType::Unknown;
    ElementGroup group = ElementGroup::Unknown;

    quint64 minInputSize = 0;
    quint64 maxInputSize = 0;
    quint64 minOutputSize = 0;
    quint64 maxOutputSize = 0;

    bool canChangeAppearance = false;
    bool hasColors = false;
    bool hasAudioBox = false;
    bool hasTrigger = false;
    bool hasFrequency = false;
    bool hasDelay = false;
    bool hasLabel = false;
    bool hasTruthTable = false;
    bool hasVolume = false;
    bool rotatesGraphic = true;
};

/**
 * \brief Validates element constraints at compile time.
 * \param c Constraints to validate.
 * \return \c true if \a c is well-formed (known type/group, consistent port sizes).
 */
constexpr bool validate(const ElementConstraints &c)
{
    // Unknown elements not allowed
    if (c.type == ElementType::Unknown) {
        return false;
    }

    // Group must be set
    if (c.group == ElementGroup::Unknown) {
        return false;
    }

    // Input/output sizes must be consistent
    if (c.minInputSize > c.maxInputSize) {
        return false;
    }
    if (c.minOutputSize > c.maxOutputSize) {
        return false;
    }

    return true;
}

/**
 * \brief Converts ElementConstraints to an ElementTypeDescriptor with all constraint-derived fields set.
 *
 * \details Specializations of ElementInfo<T>::describe() call this first, then fill in
 * the remaining display fields: \c pixmapPath, \c titleText, \c translatedName,
 * \c trContext, \c defaultAppearances, and \c alternativeAppearances.
 * \param c Source constraints.
 * \return Partially populated ElementTypeDescriptor.
 */
inline ElementTypeDescriptor descriptorFromConstraints(const ElementConstraints &c)
{
    ElementTypeDescriptor descriptor;
    descriptor.type = c.type;
    descriptor.group = c.group;
    descriptor.minInputSize = c.minInputSize;
    descriptor.maxInputSize = c.maxInputSize;
    descriptor.minOutputSize = c.minOutputSize;
    descriptor.maxOutputSize = c.maxOutputSize;
    descriptor.canChangeAppearance = c.canChangeAppearance;
    descriptor.hasColors = c.hasColors;
    descriptor.hasAudioBox = c.hasAudioBox;
    descriptor.hasTrigger = c.hasTrigger;
    descriptor.hasFrequency = c.hasFrequency;
    descriptor.hasDelay = c.hasDelay;
    descriptor.hasLabel = c.hasLabel;
    descriptor.hasTruthTable = c.hasTruthTable;
    descriptor.hasVolume = c.hasVolume;
    descriptor.rotatesGraphic = c.rotatesGraphic;
    return descriptor;
}

/**
 * \brief Self-registering element information trait.
 *
 * Specializations must define:
 * - static constexpr ElementConstraints constraints;
 * - static ElementTypeDescriptor describe();
 * - static inline const bool registered = [](){ ... }();
 *
 * The registered initializer adds the type's descriptor and its factory creator.
 *
 * Example:
 * \code
 * template<>
 * struct ElementInfo<And> {
 *     static constexpr ElementConstraints constraints{
 *         .type = ElementType::And,
 *         .group = ElementGroup::Gate,
 *         .minInputSize = 2,  .maxInputSize = 8,
 *         .minOutputSize = 1, .maxOutputSize = 1,
 *         .canChangeAppearance = true,
 *     };
 *     static_assert(validate(constraints));
 *
 *     static ElementTypeDescriptor describe() {
 *         auto descriptor = descriptorFromConstraints(constraints);
 *         descriptor.pixmapPath = []{ return QStringLiteral(":/path.svg"); };
 *         descriptor.titleText = QT_TRANSLATE_NOOP("And", "AND");
 *         descriptor.translatedName = QT_TRANSLATE_NOOP("And", "And");
 *         descriptor.trContext = "And";
 *         descriptor.defaultAppearances = {":/path.svg"};
 *         return descriptor;
 *     }
 *
 *     static inline const bool registered = []() {
 *         ElementTypeDescriptor::add(describe());
 *         ElementFactory::registerCreator(constraints.type, [] { return new And(); });
 *         return true;
 *     }();
 * };
 * \endcode
 */
template<typename T>
struct ElementInfo;  // Undefined — must be specialized in each element .cpp
