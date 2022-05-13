// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "input.h"

int Input::outputSize() const
{
    return 1;
}

int Input::outputValue() const
{
    return (getOn());
}

bool Input::isLocked() const
{
    return locked;
}

void Input::setLocked(bool value)
{
    locked = value;
}
