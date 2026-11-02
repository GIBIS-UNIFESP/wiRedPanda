// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/WirelessNode.h"

#include "App/Element/Element.h"

namespace WirelessNode {

WirelessMode mode(const Element &element)
{
    return element.hasParam(ParamId::Wireless) ? static_cast<WirelessMode>(element.param<int>(ParamId::Wireless))
                                               : WirelessMode::None;
}

void setMode(Element &element, const WirelessMode mode)
{
    element.setParam(ParamId::Wireless, static_cast<int>(mode));
}

} // namespace WirelessNode
