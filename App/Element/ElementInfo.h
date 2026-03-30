// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Self-registering element trait template and compile-time constraint validation.
 */

#pragma once

#include "App/Core/Enums.h"
#include "App/Element/ElementMetadata.h"

/**
 * \struct ElementConstraints
 * \brief Compile-time-validatable subset of ElementMetadata.
 *
 * \details These constexpr-friendly constraints enable static validation of
 * element metadata at compile time.  They are converted to ElementMetadata
 * via metadataFromConstraints().
 */
struct ElementConstraints {
    ElementType type = ElementType::Unknown;
    ElementGroup group = ElementGroup::Unknown;

    quint64 minInputSize = 0;
    quint64 maxInputSize = 0;
    quint64 minOutputSize = 0;
    quint64 maxOutputSize = 0;

    bool canChangeSkin = false;
    bool hasColors = false;
    bool hasAudio = false;
    bool hasAudioBox = false;
    bool hasTrigger = false;
    bool hasFrequency = false;
    bool hasDelay = false;
    bool hasLabel = false;
    bool hasTruthTable = false;
    bool hasVolume = false;
    bool rotatable = true;
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
 * \brief Converts ElementConstraints to an ElementMetadata with all constraint-derived fields set.
 *
 * \details Specializations of ElementInfo<T>::metadata() call this first, then fill in
 * the remaining display fields: \c pixmapPath, \c titleText, \c translatedName,
 * \c trContext, \c defaultSkins, and \c alternativeSkins.
 * \param c Source constraints.
 * \return Partially populated ElementMetadata.
 */
inline ElementMetadata metadataFromConstraints(const ElementConstraints &c)
{
    ElementMetadata meta;
    meta.type = c.type;
    meta.group = c.group;
    meta.minInputSize = c.minInputSize;
    meta.maxInputSize = c.maxInputSize;
    meta.minOutputSize = c.minOutputSize;
    meta.maxOutputSize = c.maxOutputSize;
    meta.canChangeSkin = c.canChangeSkin;
    meta.hasColors = c.hasColors;
    meta.hasAudio = c.hasAudio;
    meta.hasAudioBox = c.hasAudioBox;
    meta.hasTrigger = c.hasTrigger;
    meta.hasFrequency = c.hasFrequency;
    meta.hasDelay = c.hasDelay;
    meta.hasLabel = c.hasLabel;
    meta.hasTruthTable = c.hasTruthTable;
    meta.hasVolume = c.hasVolume;
    meta.rotatable = c.rotatable;
    return meta;
}

/**
 * \brief Self-registering element information trait.
 *
 * Specializations must define:
 * - static constexpr ElementConstraints constraints;
 * - static ElementMetadata metadata();
 * - static inline const bool registered = [](){ ... }();
 *
 * The registered initializer performs both metadata and factory registration.
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
 *         .canChangeSkin = true,
 *     };
 *     static_assert(validate(constraints));
 *
 *     static ElementMetadata metadata() {
 *         auto meta = metadataFromConstraints(constraints);
 *         meta.pixmapPath = []{ return QStringLiteral(":/path.svg"); };
 *         meta.titleText = QT_TRANSLATE_NOOP("And", "AND");
 *         meta.translatedName = QT_TRANSLATE_NOOP("And", "And");
 *         meta.trContext = "And";
 *         meta.defaultSkins = {":/path.svg"};
 *         return meta;
 *     }
 *
 *     static inline const bool registered = []() {
 *         ElementMetadataRegistry::registerMetadata(metadata());
 *         ElementFactory::registerCreator(constraints.type, [] { return new And(); });
 *         return true;
 *     }();
 * };
 * \endcode
 */
template<typename T>
struct ElementInfo;  // Undefined — must be specialized in each element .cpp

