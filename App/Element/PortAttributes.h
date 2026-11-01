// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief PortAttributes: what an element type sets on one of its ports.
 */

#pragma once

#include <functional>
#include <optional>

#include <QPointF>
#include <QString>

#include "App/Core/Enums.h"

/// The attributes of one port that its element sets. An unset field is left as the port has it, so an element that
/// sets the requirement or the default elsewhere (a wireless node, say) keeps doing so.
struct PortAttributes {
    /// A name derived each time it is asked for, so a translated name follows the language, given with the same
    /// name in the source language. Built by DisplaySegments::input().
    struct DerivedName {
        std::function<QString()> name;
        QString source;
    };

    std::optional<QString> name = std::nullopt; ///< An explicit name, its own source.
    std::optional<DerivedName> derivedName = std::nullopt; ///< Takes the place of \c name.
    std::optional<bool> required = std::nullopt;
    std::optional<Status> defaultStatus = std::nullopt;
    std::optional<QPointF> position = std::nullopt; ///< Where the port sits; unset, the port is placed in its column.
};
