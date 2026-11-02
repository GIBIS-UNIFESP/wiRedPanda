// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementColour.h"

#include <algorithm>

namespace ElementColour {

const QStringList &names()
{
    static const QStringList names{"White", "Red", "Green", "Blue", "Purple"};
    return names;
}

int indexOf(const QString &name)
{
    return std::max(0, static_cast<int>(names().indexOf(name)));
}

QString next(const QString &name)
{
    // A name not in the cycle starts it again, at White
    const qsizetype index = names().indexOf(name);
    return (index < 0) ? names().constFirst() : names().at((index + 1) % names().size());
}

QString previous(const QString &name)
{
    const qsizetype index = names().indexOf(name);
    return (index < 0) ? names().constFirst() : names().at((index + names().size() - 1) % names().size());
}

} // namespace ElementColour
