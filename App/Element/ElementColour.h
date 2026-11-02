// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ElementColour: the colours an LED and a display offer (ParamId::Colour).
 */

#pragma once

#include <QString>
#include <QStringList>

/// The colours an LED and a display offer, by name, which their parameter (ParamId::Colour) holds.
namespace ElementColour {

/// The colour names, in their index order: White, Red, Green, Blue and Purple.
const QStringList &names();

/// Returns the index of colour \a name in names(), 0 to 4; any other name, White's (0).
int indexOf(const QString &name);

/// Returns the colour after \a name in the cycle White, Red, Green, Blue, Purple and back; White after any other name.
QString next(const QString &name);

/// Returns the colour before \a name in that cycle; White before any other name.
QString previous(const QString &name);

} // namespace ElementColour
