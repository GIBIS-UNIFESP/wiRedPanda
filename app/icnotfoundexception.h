/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdexcept>

class IC;

class ICNotFoundException : public std::runtime_error
{
public:
    ICNotFoundException(const std::string &message, IC *ic);
    IC *getIC() const;

private:
    IC *m_ic;
};

