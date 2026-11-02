// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/InputLevel.h"

#include "App/Element/Element.h"

namespace InputLevel {

ParamSpec levelSpec(const char *key)
{
    return {.id = ParamId::On, .defaultValue = false, .key = key, .showsPicture = true, .drivesOutputs = true};
}

ParamSpec lockSpec()
{
    return {.id = ParamId::Locked, .defaultValue = false, .key = "locked"};
}

bool onLevel(const Element &element, const int port)
{
    Q_UNUSED(port)
    return element.param<bool>(ParamId::On);
}

void setOnLevel(Element &element, const bool level, const int port)
{
    Q_UNUSED(port)
    element.setParam(ParamId::On, level);
}

int levelSlot(const Element &element)
{
    return element.param<bool>(ParamId::On) ? 1 : 0;
}

} // namespace InputLevel
