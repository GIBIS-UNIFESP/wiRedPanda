// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>

#include <QMap>
#include <QString>
#include <QStringList>

#include "App/Core/Enums.h"

class GraphicElement;
class LogicElement;

/*!
 * @struct ElementMetadata
 * @brief Compile-time-registered properties for one element type.
 *
 * Each element .cpp registers one instance via ElementInfo<T> specialization.
 * All fields are populated at static-initialization time (before QApplication)
 * except pixmapPath and logicCreator, which are callables invoked lazily at
 * construction/simulation time.
 */
struct ElementMetadata {
    ElementType type = ElementType::Unknown;
    ElementGroup group = ElementGroup::Unknown;

    // Display properties
    std::function<QString()> pixmapPath;
    const char *titleText = nullptr;
    const char *translatedName = nullptr;
    const char *trContext = nullptr;

    // Port configuration
    quint64 minInputSize = 0;
    quint64 maxInputSize = 0;
    quint64 minOutputSize = 0;
    quint64 maxOutputSize = 0;

    // Skin configuration
    QStringList defaultSkins;
    QStringList alternativeSkins;

    // Feature flags
    bool canChangeSkin = false;
    bool hasColors = false;
    bool hasAudio = false;
    bool hasAudioBox = false;
    bool hasTrigger = false;
    bool hasFrequency = false;
    bool hasDelay = false;
    bool hasLabel = false;
    bool hasTruthTable = false;
    bool rotatable = true;

    // Logic element creator — null for IC only (which has no single LogicElement)
    std::function<std::shared_ptr<LogicElement>(GraphicElement *)> logicCreator;
};

/*!
 * @class ElementMetadataRegistry
 * @brief Registry of ElementMetadata, one entry per ElementType.
 */
class ElementMetadataRegistry
{
public:
    static const ElementMetadata &metadata(ElementType type);
    static void registerMetadata(const ElementMetadata &meta);
    static bool hasMetadata(ElementType type);

private:
    static QMap<ElementType, ElementMetadata> &registry();
};
